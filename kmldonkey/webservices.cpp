/* -*- c++ -*-
 *
 * webservices.cpp
 *
 * Copyright (C) 2003, 2005 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2007 Sebastian Sauer <mail@dipe.org>
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

#include <QShowEvent>
#include <QDebug>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <fileinfo.h>

#include <donkeyprotocol.h>

#include "kmldonkey.h"
#include "webservices.h"

FileInfoWebBrowser::FileInfoWebBrowser(int fileno, QWidget* parent)
    : WebBrowser(parent)
{
    fn = fileno;
    loaded = false;
}

void FileInfoWebBrowser::handleED2K(const ED2KURL& url)
{
    FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo(fn);

    if (url.isSameFile(fi)) {
        QString body = i18n("Do you want to rename \"%1\" to \"%2\"?",fi->fileName(),url.fileName());
        if (KMessageBox::questionTwoActions(this, body, i18n("Rename download"), KGuiItem(i18n("Rename")), KGuiItem(i18n("Cancel")), QString(), KMessageBox::PlainText) == KMessageBox::PrimaryAction) {
            QString foo = "rename " + QString::number(fn) + " \"" + url.fileName() + "\"";
            KMLDonkey::App->sendConsoleMessage(foo);
            KMLDonkey::App->donkey->refreshFileInfo(fn);
        }
    }
    else
        WebBrowser::handleED2K(url);
}

void FileInfoWebBrowser::showEvent(QShowEvent* event)
{
    WebBrowser::showEvent(event);
    if (loaded) return;
    loaded = true;
    loadWebPage();
}

EMugleFileInfo::EMugleFileInfo(int fileno, QWidget* parent)
    : FileInfoWebBrowser(fileno, parent)
{
}

void EMugleFileInfo::loadWebPage()
{
    FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo(fn);
    if (!fi) {
        openURL(QUrl("http://www.emugle.com/"));
        return;
    }

    QUrl url(QString("http://www.emugle.com/details.php?f=") + fi->fileUid("ed2k").toUpper());
    openURL(url);
}

