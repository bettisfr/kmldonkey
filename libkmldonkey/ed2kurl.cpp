/* -*- c++ -*-
 *
 * ed2kurl.cpp
 *
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

#include <QRegExp>
#include <QUrl>

#include "ed2kurl.h"
#include "fileinfo.h"
#include "searchinfo.h"
#include "serverinfo.h"
#include "shareinfo.h"

ED2KURL::ED2KURL(const QUrl& u)
    : ED2KURL(u.scheme() + "://" + u.path(QUrl::FullyDecoded))
{
}

ED2KURL::ED2KURL(const QString& text)
{
    QString url = text;
    if (url.startsWith("ed2k:|")) url.insert(5, "//");
    QRegExp fre("^ed2k://\\|file\\|([^|]+)\\|(\\d+)\\|([0-9a-fA-F]+)\\|/?$");
    QRegExp sre("^ed2k://\\|server\\|(\\d+\\.\\d+\\.\\d+\\.\\d+)\\|(\\d+)\\|/?$");

    if (fre.indexIn(url) >= 0) {
        type = "file";
        name = fre.cap(1).replace('+', ' ');
        size = fre.cap(2).toULongLong();
        hash = FileInfo::stringToMd4(fre.cap(3));
    }
    else if (sre.indexIn(url) >= 0) {
        type = "server";
        address = sre.cap(1);
        port = sre.cap(2).toUShort();
    }
    else {
        type = "invalid";
    }
}

ED2KURL::ED2KURL(FileInfo* fi)
{
    type = "file";
    name = fi->fileName();
    size = fi->fileSize();
    hash = FileInfo::stringToMd4(fi->fileUid("ed2k"));
}

ED2KURL::ED2KURL(ShareInfo* fi)
{
    type = "file";
    name = fi->shareName();
    size = fi->shareSize();
    hash = FileInfo::stringToMd4(fi->shareUid("ed2k"));
}

ED2KURL::ED2KURL(ResultInfo* fi)
{
    type = "file";
    name = fi->resultName();
    size = fi->resultSize();
    hash = FileInfo::stringToMd4(fi->resultUid("ed2k"));
}

ED2KURL::ED2KURL(ServerInfo* si)
{
    type = "server";
    address = si->serverAddress();
    port = si->serverPort();
}

QString ED2KURL::ed2kType() const { return type; }
QString ED2KURL::serverAddress() const { return address; }
int16 ED2KURL::serverPort() const { return port; }
QString ED2KURL::fileName() const { return name; }
int64 ED2KURL::fileSize() const { return size; }
QByteArray ED2KURL::fileHash() const { return hash; }
bool ED2KURL::isInvalid() const { return type == "invalid"; }

bool ED2KURL::isSameFile(const ED2KURL& u) const
{
    if (type != "file" || u.ed2kType() != "file") return false;
    if (size != u.fileSize()) return false;
    if (hash != u.fileHash()) return false;
    return true;
}

QString ED2KURL::toString() const
{
    if (type == "file")
	return QString("ed2k://|file|") + name + "|" + QString::number(size) + "|" + FileInfo::md4ToString(hash) + "|/";
    else if (type == "server")
	return QString("ed2k://|server|") + address + "|" + QString::number(port) + "|/";
    else
	return QString();
}



