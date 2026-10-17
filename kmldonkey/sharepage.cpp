#include <QMimeData>
#include <KConfigGroup>
/* -*- c++ -*-
 *
 * sharepage.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2009 Aleksey Markelov <markelovai@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <QDebug>
#include <KLocalizedString>
#include <kconfig.h>
#include <QAction>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <QSpinBox>
#include <QMimeDatabase>
#include <kxmlguifactory.h>

#include <QClipboard>
#include <QMenu>
#include <QRegExp>
#include <QTimer>
#include <QCheckBox>
#include <QShowEvent>

#include <QUrl>

#include "prefs/prefs.h"
#include "kmldonkey.h"
#include "shareddirs.h"

#include "network.h"
#include "shareinfo.h"
#include "clientinfo.h"

#include "sharepage.h"

#include "sharedfilemodel.h"
#include "uploadersmodel.h"
#include "kmldview.h"

SharePage::SharePage(QWidget* parent)
    : QWidget(parent)
    , KMLDonkeyPage()
{
    setObjectName("sharePage");
    refreshTimer = new QTimer(this);
    refreshTimer->setObjectName("refreshTimer");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(8, 8, 8, 8);
    setLayout(layout);

    uploadSplitter = new QSplitter(this);
    uploadSplitter->setObjectName("uploadSplitter");
    uploadSplitter->setOrientation(Qt::Vertical);

    QAbstractItemModel *smodel = new Q4::SharedFileModel(this);
    shareView = KMLDonkey::App->viewCreator->createViewSorted(smodel, "shareView", uploadSplitter);
    shareView->setContextMenuPolicy(Qt::CustomContextMenu);
    shareView->setDragEnabled(true);
    shareView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QAbstractItemModel *umodel = new Q4::UploadersModel(this);
    uploadersView = KMLDonkey::App->viewCreator->createViewSorted(umodel, "uploadersView", uploadSplitter);
    uploadersView->setContextMenuPolicy(Qt::CustomContextMenu);

    layout->addWidget(uploadSplitter, 1);
    uploadSplitter->setStretchFactor(0,2);
    uploadSplitter->setStretchFactor(1,1);

    connect(shareView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            SLOT(clearUploadersSelection()));
    connect(uploadersView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            SLOT(clearUploadSelection()));

    connect(shareView, SIGNAL(customContextMenuRequested(const QPoint&)), 
            SLOT(uploadContextMenu(const QPoint&)));
    connect(uploadersView, SIGNAL(customContextMenuRequested(const QPoint&)), 
            SLOT(uploadersContextMenu(const QPoint&)));

    connect(KMLDonkey::App->donkey, SIGNAL(signalConnected()), this, SLOT(actionRefreshUploaders()));

    connect(refreshTimer, SIGNAL(timeout()), SLOT(refreshLists()));
}

void SharePage::handleGenericAction(const QString& action)
{
    if (shareView->hasFocus()) {
        QMimeData *data =
            shareView->model()->mimeData(shareView->selectionModel()->selectedIndexes());
        if (!data) return;
        QString text;
        if (action == "copy_url") {
            QStringList urls;
            for(const auto &url:data->urls()) urls.append(url.toString());
            text = urls.join("\n");
        } else if (action == "copy_html") {
            text = data->html();
        } else if (action == "copy_hash") {
            text = data->text();
        }
        delete data;
        QApplication::clipboard()->setText(text);
    }
}

void SharePage::showEvent(QShowEvent*)
{
    if (KMLDonkey::App->donkey->isConnected() && ! autoUpdate) {
        actionRefreshShared();
        actionRefreshUploaders();
    }
}

void SharePage::deactivatePageActions()
{
    //uploadActions.setEnabled(false);
    //uploadersActions.setEnabled(false);
}

QStringList SharePage::supportedGenericActions()
{
    QStringList actions;

    bool share = shareView->selectionModel()->hasSelection();
    //uploadActions.setEnabled(share);
    if (share) {
        actions << "copy_url" << "copy_html" << "copy_hash";
    }

    //bool uploaders = uploadersView->selectionModel()->hasSelection();
    //uploadersActions.setEnabled(uploaders);

    return actions;
}

void SharePage::plugGenericActions(QObject* object, const char* slot)
{
    connect(this, SIGNAL(genericActionsChanged(KMLDonkeyPage*)), object, slot);
}

void SharePage::clearUploadSelection()
{
    shareView->blockSignals(true);
    shareView->clearSelection();
    shareView->blockSignals(false);
    emit genericActionsChanged(static_cast<KMLDonkeyPage*>(this));
}

void SharePage::clearUploadersSelection()
{
    uploadersView->blockSignals(true);
    uploadersView->clearSelection();
    uploadersView->blockSignals(false);
    emit genericActionsChanged(static_cast<KMLDonkeyPage*>(this));
}

void SharePage::setupActions(KActionCollection* actionCollection)
{
    refreshShared = new QAction(QIcon::fromTheme("view-refresh"), i18n("&Refresh Shared File List"), this);
    actionCollection->addAction("upload_refresh", refreshShared);
    connect(refreshShared, SIGNAL(triggered()), this, SLOT(actionRefreshShared()));

    refreshUploaders = new QAction(QIcon::fromTheme("view-refresh"), i18n("Refresh U&ploaders List"), this);
    actionCollection->addAction("uploaders_refresh", refreshUploaders);
    connect(refreshUploaders, SIGNAL(triggered()), this, SLOT(actionRefreshUploaders()));

    QAction* action = new QAction(QIcon::fromTheme("document-export"), i18n("&Shared Folders..."), this);
    actionCollection->addAction("shared_dirs", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionSharedDirs()));
    KMLDonkey::App->addCoreAction( action );

    action = new QAction(QIcon::fromTheme("arrow-up"), i18n("Activate Uploads Page"), this);
    action->setIconText(i18n("Uploads"));
    actionCollection->addAction("activate_page_uploads", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionActivatePage()));

    deactivatePageActions();
}

void SharePage::clear()
{
}

void SharePage::configurePrefsDialog(KMLDonkeyPreferences* prefs)
{
    prefs->listsPage->autoUpdateCheckbox->setChecked(autoUpdate);
    prefs->listsPage->updateFrequencyEntry->setValue(updateFrequency);
}

void SharePage::applyPreferences(KMLDonkeyPreferences* prefs)
{
    if (prefs) {
        autoUpdate = prefs->listsPage->autoUpdateCheckbox->isChecked();
        updateFrequency = prefs->listsPage->updateFrequencyEntry->value();
    }

    updateTimer();
    //TODO: move this to KmldView
    shareView->setFont(KMLDonkey::App->listFont);
    if (KMLDonkey::App->listFont !=  uploadersView->font())
        uploadersView->setFont(KMLDonkey::App->listFont);
}

void SharePage::restoreState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "SharePage" );
    autoUpdate = group.readEntry("AutoUpdateShareLists", false);
    updateFrequency = group.readEntry("ShareListUpdateFrequency", 10);

    QList<int> upsizes = group.readEntry( "UploadSplitter", QList<int>() );
    if( upsizes.count() > 0 )
        uploadSplitter->setSizes(upsizes);

    applyPreferences();
}

void SharePage::saveState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "SharePage" );
    group.writeEntry("AutoUpdateShareLists", autoUpdate);
    group.writeEntry("ShareListUpdateFrequency", updateFrequency);
    group.writeEntry("UploadSplitter", uploadSplitter->sizes());
}

void SharePage::uploadContextMenu(const QPoint& pt)
{
    QMenu *pop = qobject_cast<QMenu*>(KMLDonkey::App->factory()->
            container("upload_actions", KMLDonkey::App));
    if (!pop)
        KMLDonkey::App->showBadInstallDialog();
    else
        pop->popup(shareView->mapToGlobal(pt));
}

void SharePage::uploadersContextMenu(const QPoint& pt)
{
    QMenu *pop = qobject_cast<QMenu*>(KMLDonkey::App->factory()->
            container("uploaders_actions", KMLDonkey::App));
    if (!pop)
        KMLDonkey::App->showBadInstallDialog();
    else
        pop->popup(uploadersView->mapToGlobal(pt));
}

void SharePage::actionRefreshShared()
{
    KMLDonkey::App->donkey->refreshShared();
}

void SharePage::actionSharedDirs()
{
    if (! KMLDonkey::App->donkey->isConnected()) return;
    SharedDirsDialog *dlg = new SharedDirsDialog(this);
    if (dlg) dlg->show();
}



void SharePage::actionActivatePage()
{
    KMLDonkey::App->activatePage(this);
}

void SharePage::actionRefreshUploaders()
{
    KMLDonkey::App->donkey->updateUploaders();
}

void SharePage::refreshLists()
{
    actionRefreshShared();
    actionRefreshUploaders();
}

void SharePage::updateTimer()
{
    refreshShared->setEnabled(!autoUpdate);
    refreshUploaders->setEnabled(!autoUpdate);
    if (!autoUpdate) {
        if (refreshTimer->isActive()) refreshTimer->stop();
        return;
    }
    if (refreshTimer->isActive()) {
        if (updateFrequency * 1000 != activeFrequency) {
            activeFrequency = updateFrequency * 1000;
            refreshTimer->start(activeFrequency);
        }
    } else {
        activeFrequency = updateFrequency * 1000;
        refreshTimer->start(activeFrequency);
    }
}
