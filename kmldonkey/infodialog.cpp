#include <KConfigGroup>
#include <QLocale>
/* -*- c++ -*-
 *
 * infodialog.cpp
 *
 * Copyright (C) 2003, 2005 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
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

#include "infodialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QListWidget>

#include <KSharedConfig>
#include <kconfig.h>
#include <KLocalizedString>
#include <QDebug>
#include <QMenu>
#include <QIcon>
#include <KLineEdit>

#include <network.h>
#include <donkeyhost.h>
#include <hostmanager.h>
#include <fileinfo.h>

#include "availability.h"
#include "clientlist.h"
#include "kmldonkey.h"
#include "webservices.h"


static QLabel* makeLabel(const QString& label, QWidget* parent, bool alignTop = false)
{
    QLabel* l = new QLabel(label, parent);
    if (alignTop) l->setAlignment(Qt::AlignTop | Qt::AlignRight);
    else l->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    l->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    return l;
}

FileInfoDialog::FileInfoDialog(int file)
    : PortedPageDialog()
{
    setCaption(i18n("File Information"));
    setFaceType(PortedPageDialog::Tabbed);
    setButtons(PortedDialog::Close);

    fileno = file;
    FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo(fileno);
    if (!fi) return;

    QFrame *frame = new QFrame(this);
    addPage(frame, i18n("File"));
    QGridLayout* layout = new QGridLayout(frame);

    layout->addWidget(makeLabel(i18n("Name:"), frame), 0, 0);
    layout->addWidget(makeLabel(i18n("Complete:"), frame), 1, 0);
    layout->addWidget(makeLabel(i18n("Downloaded:"), frame), 2, 0);
    layout->addWidget(makeLabel(i18n("Speed:"), frame), 3, 0);
    layout->addWidget(makeLabel(i18n("Hash:"), frame), 4, 0);
    layout->addWidget(makeLabel(i18n("Format:"), frame), 5, 0);
    layout->addWidget(makeLabel(i18n("Comment:"), frame), 6, 0);
    layout->addWidget(makeLabel(i18n("Alt. names:"), frame, true), 7, 0);

    nameView = new KLineEdit(fi->fileName(), frame);
    nameView->setObjectName("nameView");
    nameView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameView->setReadOnly(true);
    layout->addWidget(nameView, 0, 1);

    availView = new AvailabilityWidget(fileno, frame, "availView");
    layout->addWidget(availView, 1, 1);

    QHBoxLayout* l2 = new QHBoxLayout();

    sizeView = new KLineEdit(FileInfo::humanReadableSize(fi->fileDownloaded()), frame);
    sizeView->setObjectName("sizeView");
    sizeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizeView->setReadOnly(true);
    l2->addWidget(sizeView);

    l2->addWidget(makeLabel(i18n("Size:"), frame));
    totalSizeView = new KLineEdit(FileInfo::humanReadableSize(fi->fileSize()), frame);
    totalSizeView->setObjectName("totalSizeView");
    totalSizeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    totalSizeView->setReadOnly(true);
    l2->addWidget(totalSizeView);

    l2->addWidget(makeLabel(i18n("Network:"), frame));
    networkView = new KLineEdit(KMLDonkey::App->donkey->findNetworkNo(fi->fileNetwork())->networkName(), frame);
    networkView->setObjectName("networkView");
    networkView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    networkView->setReadOnly(true);
    l2->addWidget(networkView);

    layout->addLayout(l2, 2, 1);

    QHBoxLayout* l3 = new QHBoxLayout();

    speedView = new KLineEdit(FileInfo::humanReadableSize((int64)fi->fileSpeed()) + "b/s", frame);
    speedView->setObjectName("speedView");
    speedView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    speedView->setReadOnly(true);
    l3->addWidget(speedView);

    l3->addWidget(makeLabel(i18n("ETA:"), frame));
    etaView = new KLineEdit(FileInfo::calculateETA(fi), frame);
    etaView->setObjectName("etaView");
    etaView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    etaView->setReadOnly(true);
    l3->addWidget(etaView);

    layout->addLayout(l3, 3, 1);

    hashView = new KLineEdit(fi->fileUid(), frame);
    hashView->setObjectName("hashView");
    hashView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hashView->setReadOnly(true);
    layout->addWidget(hashView, 4, 1);

    formatView = new KLineEdit(fi->fileFormatInfo(), frame);
    formatView->setObjectName("formatView");
    formatView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formatView->setReadOnly(true);
    layout->addWidget(formatView, 5, 1);

    commentView = new KLineEdit(fi->fileComment(), frame);
    commentView->setObjectName("commentView");
    commentView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    commentView->setReadOnly(true);
    layout->addWidget(commentView, 6, 1);

    namesView = new QListWidget(frame);
    namesView->setObjectName("namesView");
    //namesView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    namesView->setContextMenuPolicy(Qt::CustomContextMenu);
    namesView->addItems(fi->fileNames());
    layout->addWidget(namesView, 7, 1);

    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 100);

    QFrame *srcframe = new QFrame(this);
    addPage(srcframe,i18n("Sources"));
    QVBoxLayout* srclayout = new QVBoxLayout(srcframe);
    clientList = new ClientList(fileno, srcframe);
    srclayout->addWidget(clientList);
	
	// TODO
    // KConfigGroup group = KSharedConfig::openConfig()->group("DownloadSourceList");
    // clientList->restoreLayout(group);

#if 0 //sebsauer
    DonkeyHost* host = dynamic_cast<DonkeyHost*>(KMLDonkey::App->hostManager->hostProperties(KMLDonkey::App->lastHost));
    if (host) {
        QUrl path;
        path.setProtocol("http");
        path.setHost("localhost");
        path.setPort(37435);
        path.setPath("/");
        path.addPath(host->name());
        path.addPath(host->username());
        path.addPath(host->password());
        path.addPath(QString::number(fi->fileNo()));
        qDebug() << "Preview URL: " << path.toDisplayString();
        previewframe = new QFrame(this);
        addPage(previewframe, i18n("Preview"));
        (new QVBoxLayout(previewframe))->addWidget(preview = new PreviewWidget(path, fi->fileName(), previewframe));
    } else {
        preview = 0;
        previewframe = 0;
    }
#else
    preview = 0;
    previewframe = 0;
    // The historical multimedia preview service is not available in Qt6.
#endif

    if (KMLDonkey::App->donkey->findNetworkNo(fi->fileNetwork())->networkName() == "Donkey") {
        QFrame* emugleframe = new QFrame(this);
        addPage(emugleframe, i18n("eMugle"));
        (new QVBoxLayout(emugleframe))->addWidget(new EMugleFileInfo(fileno, emugleframe));
    }

    updateCaption(fi);

    restoreDialogSize(KSharedConfig::openConfig()->group("FileInfoDialog"));

    connect(this, SIGNAL(finished(int)), this, SLOT(saveSize()));
    connect(KMLDonkey::App->donkey, SIGNAL(updatedDownloadFiles()), this, SLOT(updateFile()));
    connect(KMLDonkey::App->donkey, SIGNAL(fileUpdated(int,DonkeyProtocol::FromCore)), this, SLOT(updateFile(int)));
    connect(namesView, SIGNAL(customContextMenuRequested(const QPoint&)),
            SLOT(namesViewContextMenu(const QPoint&)));
}

void FileInfoDialog::updateCaption(FileInfo* fi)
{
    QString caption;
    caption = i18nc("Dialog window caption: (percent complete) filename","%1",
        QString("(%1%) %2").arg(QLocale().toString(double((fi->fileDownloaded() * 100.0) / fi->fileSize()), 'f', 2)).arg(fi->fileName()) );
    setCaption(caption);
}

void FileInfoDialog::saveSize()
{
    KConfigGroup sizeg = KSharedConfig::openConfig()->group("FileInfoDialog");
    saveDialogSize(sizeg);
	
	// TODO
    // KConfigGroup group = KSharedConfig::openConfig()->group("DownloadSourceList");
    // clientList->saveLayout(group);
    
	deleteLater();
}

void FileInfoDialog::updateFile(int file)
{
    if (file >= 0 && file != fileno) return;
    FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo(fileno);
    if (!fi) {
        deleteLater();
        return;
    }
    nameView->setText(fi->fileName());
    availView->update();
    sizeView->setText(FileInfo::humanReadableSize(fi->fileDownloaded()));
    totalSizeView->setText(FileInfo::humanReadableSize(fi->fileSize()));
    speedView->setText(i18nc("download speed in bytes per second", "%1b/s", FileInfo::humanReadableSize((int64)fi->fileSpeed())));
    etaView->setText(FileInfo::calculateETA(fi));
    formatView->setText(fi->fileFormatInfo());
    commentView->setText(fi->fileComment());
    updateCaption(fi);
}

void FileInfoDialog::namesViewContextMenu(const QPoint& pt)
{
    if (!namesView->currentItem()) return;
    QMenu* pop = new QMenu(this);
    pop->setTitle(i18n("Alternative Names"));
    pop->addAction(QIcon::fromTheme("edit-rename"), i18n("Use This &Name"), this, SLOT(namesViewRenameAction()));
    pop->popup(namesView->mapToGlobal(pt));
}

void FileInfoDialog::namesViewRenameAction()
{
    QListWidgetItem *item = namesView->currentItem();
    if (!item) return;
    KMLDonkey::App->donkey->renameFile(fileno, item->text());
}

