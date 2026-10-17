/* -*- c++ -*-
 *
 * downloadrenamer.cpp
 *
 * Copyright (C) 2009       Aleksey Markelov <markelovai@gmail.com>
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

#include "downloadrenamer.h"

#include <QTimer>

#include "kmldonkey.h"
#include "donkeyprotocol.h"
#include "fileinfo.h"

DownloadRenamer::DownloadRenamer(const QString &ed2kHash, const QString &newName, QObject *parent) :
     QObject(parent), 
     m_ed2kHash(ed2kHash), 
     m_newName(newName)
{
    connect(KMLDonkey::App->donkey, SIGNAL(fileAdded(int,bool)), this, SLOT(downloadStarted(int)));
    QTimer::singleShot(10000, this, SLOT(deleteLater()));    
}


void DownloadRenamer::downloadStarted(int fileNo)
{
    FileInfo *fi = KMLDonkey::App->donkey->findDownloadFileNo(fileNo);
    if (!fi || fi->fileUid("ed2k") != m_ed2kHash )
        return;
    qDebug("Using download renamer");
    KMLDonkey::App->donkey->renameFile(fileNo, m_newName);
    deleteLater();
}

DownloadRenamer::~DownloadRenamer()
{
    qDebug("Removing download renamer");

}

