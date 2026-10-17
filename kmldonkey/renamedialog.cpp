/* -*- c++ -*-
 *
 * renamedialog.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
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

#include "renamedialog.h"
#include "infolist.h"

#include <QLabel>
#include <QGridLayout>

#include <KSharedConfig>
#include <kconfig.h>
#include <KLocalizedString>
#include <QDebug>
#include <klineedit.h>
#include <kcombobox.h>

#include <fileinfo.h>

RenameDialog::RenameDialog(DonkeyProtocol* protocol, int file)
    : PortedDialog()
{
    setCaption(i18n("Select File Name"));
    setButtons(PortedDialog::Help|PortedDialog::Default|PortedDialog::Try|PortedDialog::Ok|PortedDialog::Cancel);

	connect( this, SIGNAL(okClicked()), this, SLOT(slotOk()) );
	connect( this, SIGNAL(defaultClicked()), this, SLOT(slotDefault()) );
	connect( this, SIGNAL(tryClicked()), this, SLOT(slotTry()) );
	
    donkey = protocol;
    fileno = file;
    FileInfo* fi = donkey->findDownloadFileNo(fileno);
    if (!fi) 
		return;

    setButtonText(Try, i18n("&Preview"));
    //setButtonTip(Try, i18n("Ask MLDonkey to preview the file."));

    QWidget *frame = mainWidget();
    QGridLayout* layout = new QGridLayout(frame);

    layout->addWidget(new QLabel(i18n("Name:"), frame), 0, 0);
    layout->addWidget(new QLabel(i18n("Size:"), frame), 1, 0);
    layout->addWidget(new QLabel(i18n("Hash:"), frame), 2, 0);
    layout->addWidget(new QLabel(i18n("Format:"), frame), 3, 0);
    layout->addWidget(new QLabel(i18n("New name:"), frame), 4, 0);

    nameView = new KLineEdit(fi->fileName(), frame);
    nameView->setObjectName("nameView");
    nameView->setReadOnly(true);
    layout->addWidget(nameView, 0, 1);

    sizeView = new KLineEdit(FileInfo::humanReadableSize(fi->fileSize()), frame);
    sizeView->setObjectName("sizeView");
    sizeView->setReadOnly(true);
    layout->addWidget(sizeView, 1, 1);

    hashView = new KLineEdit(fi->fileUid(), frame);
    hashView->setObjectName("hashView");
    hashView->setReadOnly(true);
    layout->addWidget(hashView, 2, 1);

    formatView = new KLineEdit(fi->fileFormatInfo(), frame);
    formatView->setObjectName("formatView");
    formatView->setReadOnly(true);
    layout->addWidget(formatView, 3, 1);

    nameEdit = new KComboBox(frame);
    nameEdit->setObjectName("nameEdit");
    nameEdit->setEditable(true);
    nameEdit->insertItems( 0, fi->fileNames());
    nameEdit->setCurrentIndex( nameEdit->findText( fi->fileName() ) );
    nameEdit->setEditText( fi->fileName() );
    layout->addWidget(nameEdit, 4, 1);
}

void RenameDialog::slotDefault()
{
    FileInfo* fi = donkey->findDownloadFileNo(fileno);
    if (!fi) 
		return;
	
    nameEdit->setCurrentIndex( nameEdit->findText( fi->fileName() ) );
    nameEdit->setEditText( fi->fileName() );
}

void RenameDialog::slotTry()
{
    donkey->previewFile(fileno);
}

void RenameDialog::slotOk()
{
    QString foo = nameEdit->currentText();
    emit renameFile(fileno, foo);
    PortedDialog::accept();
}

