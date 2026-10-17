/* -*- c++ -*-
 *
 * shareinfo.cpp
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

#include "shareinfo.h"
#include "donkeymessage.h"
#include "fileinfo.h"

#include <QFile>
#include <QRegularExpression>
#include <QByteArray>

ShareInfo::ShareInfo(int num, DonkeyMessage* msg, int proto)
{
    this->num = num;
    updateShareInfo(msg, proto);
}

void ShareInfo::updateShareInfo(DonkeyMessage *msg, int proto)
{
    network = msg->readInt32();
    name = msg->readString();
    size = msg->readInt64();
    uploaded = msg->readInt64();
    requests = msg->readInt32();
    if (msg->opcode() >= 48) {
        uids = proto >= 31
            ? msg->readStringList()
            : QStringList( QString("urn:ed2k:") + FileInfo::md4ToString( msg->readMd4() ) );
    }
}

ShareInfo::~ShareInfo()
{
}

void ShareInfo::updateShare(DonkeyMessage* msg, int)
{
    uploaded = msg->readInt64();
    requests = msg->readInt32();
}

const int& ShareInfo::shareNo() const { return num; }
const int& ShareInfo::shareNetwork() const { return network; }
const QString& ShareInfo::shareName() const { return name; }
const int64& ShareInfo::shareSize() const { return size; }
const int64& ShareInfo::shareUploaded() const { return uploaded; }
const int& ShareInfo::shareRequests() const { return requests; }

const QStringList& ShareInfo::shareUids() const
{
    return uids;
}

QString ShareInfo::shareUid() const
{
    return uids.isEmpty() ? QString() : uids.first();
}

QString ShareInfo::shareUid(const QString& type) const
{
    QRegularExpression match(QString("^urn:") + type + ":");
    QStringList results = uids.filter(match);
    if (!results.count()) return QString();

    QString result = results.first();
    result.remove(match);
    return result;
}

