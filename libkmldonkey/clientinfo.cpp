/* -*- c++ -*-
 *
 * clientinfo.cpp
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


#include "donkeymessage.h"
#include "clientinfo.h"
#include "fileinfo.h"

ClientInfo::ClientInfo(int num, DonkeyMessage* msg, int proto)
{
    this->num = num;
    updateClientInfo(msg, proto);
}

void ClientInfo::updateClientInfo(DonkeyMessage *msg, int proto)
{
    network = msg->readInt32();
    switch (msg->readInt8()) {
    case 0: {
        QString ip = msg->readIPAddress();
        int port = msg->readInt16();
        kind = ip + ":" + QString::number(port);
    } break;
    case 1: {
        QString name = msg->readString();
        QByteArray md4 = msg->readMd4();
        kind = "INDIRECT:" + FileInfo::md4ToString(md4) + ":" + name;
    } break;
    default:
        kind = "UNKNOWN";
        break;
    }
    setClientState(msg, proto);
    type = (Type)msg->readInt8();

    for (int i = msg->readInt16(); i; i--)
        if(! msg->readTag(tags)) return;

    name = msg->readString();
    rating = msg->readInt32();
    software = msg->readString();
    downloaded = msg->readInt64();
    uploaded = msg->readInt64();
    upload = msg->readString();
    chatPort = 0;
    connectTime = msg->readInt32(); // don't use (proto >= 24) ? msg->readDate() here cause the connectTime isn't relative!
    emulemod = msg->readString();
    release = (proto >= 33) ? msg->readString() : QString();
}

ClientInfo::~ClientInfo()
{
}

void ClientInfo::setClientState(ClientInfo::State newstate, int newqueue)
{
    state = newstate;
    if (state == Connected2)
        queue = newqueue;
    downloading = (state == Downloading) ? newqueue : 0;
}

void ClientInfo::setClientState(DonkeyMessage* msg, int /*proto*/)
{
    ClientInfo::State st = (State)msg->readInt8();
    if ( st == Connected2 || st == NotConnected2 || st == Downloading )
	setClientState(st, msg->readInt32());
    else
	setClientState(st);
}

void ClientInfo::setClientType(ClientInfo::Type newtype)
{
    type = newtype;
}

void ClientInfo::setClientType(DonkeyMessage* msg, int)
{
    type = (Type)msg->readInt8();
}

const int& ClientInfo::clientNo() const { return num; }
const int& ClientInfo::clientNetwork() const { return network; }
const QString& ClientInfo::clientKind() const { return kind; }
const ClientInfo::State& ClientInfo::clientState() const { return state; }
const int& ClientInfo::clientQueuePosition() const { return queue; }
const ClientInfo::Type& ClientInfo::clientType() const { return type; }
const QMap<QString,QVariant>& ClientInfo::clientTags() const { return tags; }
const QString& ClientInfo::clientName() const { return name; }
const int& ClientInfo::clientRating() const { return rating; }
const int& ClientInfo::clientChatPort() const { return chatPort; }
const QString& ClientInfo::clientSoftware() const { return software; }
const int64& ClientInfo::clientDownloaded() const { return downloaded; }
const int64& ClientInfo::clientUploaded() const { return uploaded; }
const QString& ClientInfo::clientUpload() const { return upload; }
const int& ClientInfo::clientConnectTime() const { return connectTime; }
const QString& ClientInfo::clientEmuleMod() const { return emulemod; }
const int& ClientInfo::clientDownloading() const { return downloading; }
const QString& ClientInfo::clientRelease() const { return release; }
