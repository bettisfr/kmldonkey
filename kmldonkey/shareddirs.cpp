/* -*- c++ -*-
 *
 * shareddirs.cpp
 *
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
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

#include <QLayout>
#include <QLabel>
#include <QToolTip>
#include <QHBoxLayout>

#include <KSharedConfig>
#include <kconfig.h>
#include <KLocalizedString>
#include <QDebug>
#include <QPushButton>
#include <QTreeWidget>
#include <klineedit.h>
#include <QSpinBox>
#include <kmessagebox.h>
#include <QFileDialog>

#include "infolist.h"
#include "prefs/prefs.h"
#include "kmldonkey.h"

#include "hostmanager.h"

#include "shareddirs.h"

void applySharesConsoleCallback::callback(const QString& cmd, const QString& res)
{
    count--;
    list << QString("%1: %2").arg(cmd).arg(res);
    if (count == 0)
        KMessageBox::informationList(KMLDonkey::App, i18n("Messages from core about applied changes."), list, i18n("Shared Folders"));
}


void sharesConsoleCallback::callback(const QString& /*cmd*/, const QString& res)
{
    QStringList list = res.split("\n");
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString s = (*it).trimmed();
        int pos = s.indexOf(" ");
        if (pos < 0) continue;

        bool ok;
        int prio = s.left(pos).toInt(&ok);
        if (! ok) continue;
        QString dir = s.right(s.length() - pos - 1).trimmed();
        if(dlg->shareddirs.contains(dir)) continue;

        if (dlg->addItem(dir, prio))
            dlg->shareddirs[dir] = prio;
    }
    dlg->setEnabled();
}

SharedDirsDialog::SharedDirsDialog(QWidget* parent)
    : PortedDialog(parent)
{
    setCaption(i18n("Shared Folders"));
    setButtons(PortedDialog::Ok | PortedDialog::Cancel);
    setObjectName("SharedDirsDialog");
    setModal(true);

    applyCallback = 0;

    QWidget *grpBox = mainWidget();
    QVBoxLayout* grpLayout = new QVBoxLayout(grpBox);
    grpLayout->setContentsMargins(0, 0, 0, 0);
    grpLayout->setSpacing(12);
    grpBox->setLayout(grpLayout);

    QWidget *vbox = new QWidget(grpBox);
    QHBoxLayout* vboxLayout = new QHBoxLayout(vbox);
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    vboxLayout->setSpacing(12);
    vbox->setLayout(vboxLayout);
    grpLayout->addWidget(vbox, 2);

    // Listview
    view = new QTreeWidget(vbox);
    vboxLayout->addWidget(view, 2);
    view->setAllColumnsShowFocus(true);
	view->setColumnCount( 2 );
	QTreeWidgetItem * hi = view->headerItem();
	if( hi )
	{
		hi->setText( 0, i18n("Folder") );
		hi->setText( 1, i18n("Priority") );
	}
    connect(view, SIGNAL(itemSelectionChanged()), this, SLOT( slotSelectionChanged() ));

    // Buttons
    QWidget *btnBox = new QWidget(vbox);
    QVBoxLayout* btnboxLayout = new QVBoxLayout(btnBox);
    btnboxLayout->setContentsMargins(0, 0, 0, 0);
    btnBox->setLayout(btnboxLayout);
    vboxLayout->addWidget(btnBox);

    btnAdd = new QPushButton(QIcon::fromTheme("list-add"), i18n("&Add"), btnBox);
    btnboxLayout->addWidget(btnAdd);
    btnAdd->setToolTip(i18n("Add shared folder"));
    btnAdd->setEnabled(false);
    connect(btnAdd, SIGNAL( clicked() ), this, SLOT( slotAdd() ));

    btnRemove = new QPushButton(QIcon::fromTheme("list-remove"), i18n("&Remove"), btnBox);
    btnboxLayout->addWidget(btnRemove);
    btnRemove->setToolTip(i18n("Remove shared folder"));
    btnRemove->setEnabled(false);
    connect(btnRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ));

    btnboxLayout->addStretch(1);

    // Edit
    editBox = new QFrame(grpBox);
    grpLayout->addWidget(editBox);
    QVBoxLayout* editboxLayout = new QVBoxLayout(editBox);
    editboxLayout->setContentsMargins(0, 0, 0, 0);
    editBox->setLayout(editboxLayout);
    editBox->setEnabled(false);

    QWidget *hb1 = new QWidget(editBox);
    editboxLayout->addWidget(hb1);
    QHBoxLayout* hb1Layout = new QHBoxLayout(hb1);
    hb1Layout->setContentsMargins(0, 0, 0, 0);
    hb1->setLayout(hb1Layout);
    hb1Layout->addWidget( new QLabel(i18n("Folder:"), hb1) );
    editDir = new KLineEdit(hb1);
    editDir->setClearButtonEnabled(true);
    editDir->setPlaceholderText(i18n("Folder on the MLDonkey server"));
    hb1Layout->addWidget( editDir, 1 );
    connect(editDir, SIGNAL( textChanged(const QString &) ), this, SLOT( slotEditChanged() ));
    if (KMLDonkey::App->hostManager->hostType(KMLDonkey::App->lastHost) != HostInterface::External) {
        QPushButton *editBtn = new QPushButton(QIcon::fromTheme("document-open"), QString(), hb1);
        editBtn->setToolTip(i18n("Browse folders"));
        connect(editBtn, SIGNAL( clicked() ), this, SLOT( slotDirPath() ));
    }

    QWidget *hb2 = new QWidget(editBox);
    editboxLayout->addWidget(hb2);
    QHBoxLayout* hb2Layout = new QHBoxLayout(hb2);
    hb2Layout->setContentsMargins(0, 0, 0, 0);
    hb2->setLayout(hb2Layout);
    hb2Layout->addWidget( new QLabel(i18n("Priority:"), hb2) );
    editPrio = new QSpinBox(hb2);
    hb2Layout->addWidget( editPrio, 1 );
    connect(editPrio, SIGNAL( valueChanged(int) ), this, SLOT( slotPrioChanged() ));

    editboxLayout->addStretch(1);
    resize(QSize(520, 380).expandedTo( minimumSizeHint() ));
    enableButton(Ok, false);
    connect(this, SIGNAL( okClicked() ), this, SLOT( applyChanges() ));
    connect(this, SIGNAL(finished(int)), this, SLOT( closeDialog() ));

    enabled = false;
    if (KMLDonkey::App->donkey->isConnected()) {
        sharesConsoleCallback *sharesCallback = new sharesConsoleCallback;
        sharesCallback->dlg = this;
        KMLDonkey::App->donkey->sendConsoleMessage("shares", sharesCallback);
    }
}

SharedDirsDialog::~SharedDirsDialog()
{
    // FIXME: We can't delete applyCallback here cause
    // it's callback-function will be called later, but
    // we need to free it at some point.
    //delete applyCallback; applyCallback = 0;
}

void SharedDirsDialog::setEnabled()
{
    if (! enabled) {
        enabled = true;
        btnAdd->setEnabled(true);
        enableButton(Ok, true);
    }
}

QTreeWidgetItem* SharedDirsDialog::addItem(QString dir, int prio)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(view);
    item->setText(0, dir);
    item->setText(1, QString::number(prio));
    return item;
}

void SharedDirsDialog::slotSelectionChanged()
{
    bool b = enabled && view->currentItem();
    btnRemove->setEnabled(b);
    editBox->setEnabled(b);
    editDir->setText( b ? view->currentItem()->text(0) : QString("") );
    editPrio->setValue( b ? view->currentItem()->text(1).toInt() : 0 );
}

void SharedDirsDialog::slotAdd()
{
    view->setCurrentItem( addItem("", 0) );
    editDir->setFocus();
}

void SharedDirsDialog::slotRemove()
{
    QTreeWidgetItem *item = view->currentItem();
    if (item) 
		delete item;
}

void SharedDirsDialog::slotEditChanged()
{
    QTreeWidgetItem *item = view->currentItem();
    if (item) 
		item->setText(0, editDir->text());
}

void SharedDirsDialog::slotPrioChanged()
{
    QTreeWidgetItem *item = view->currentItem();
    if (item) 
		item->setText(1, QString::number(editPrio->value()));
}

void SharedDirsDialog::slotDirPath()
{
    QUrl dir = QUrl::fromLocalFile(QFileDialog::getExistingDirectory(this, i18n("Select Folder"), editDir->text()));
    if (! dir.isEmpty()) 
		editDir->setText( dir.path() );
}

void SharedDirsDialog::applyChanges()
{
    if (! applyCallback)
        applyCallback = new applySharesConsoleCallback();

    applyCallback->count = 0;
    applyCallback->list.clear();

    QString s;
    bool b;
    QTreeWidgetItemIterator vit( view );
    for ( ; *vit; ++vit) {
		QTreeWidgetItem * item = *vit;
        QString s = item->text(0);
        int i = item->text(1).toInt(&b);
        if ( (! shareddirs.contains(s) || shareddirs[s] != i) && ! s.isEmpty() && b) {
            applyCallback->count++;
            KMLDonkey::App->donkey->sendConsoleMessage("share " + QString::number(i) + " \"" + s + "\"", applyCallback);
        }
        if (shareddirs.contains(s))
            shareddirs.remove(s);
    }

    for (QMap<QString,int>::iterator sit = shareddirs.begin(); sit != shareddirs.end(); ++sit) {
        applyCallback->count++;
        KMLDonkey::App->donkey->sendConsoleMessage("unshare \"" + sit.key() + "\"", applyCallback);
    }
}

void SharedDirsDialog::closeDialog()
{
    deleteLater();
}

