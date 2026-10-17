/* -*- c++ -*-
 *
 * serverinfo.cpp
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

#include "serverinfo.h"
#include "donkeymessage.h"

ServerInfo::ServerInfo(int num, DonkeyMessage* msg, int proto)
{
    this->num = num;
    updateServerInfo(msg,proto);
}

void ServerInfo::updateServerInfo(DonkeyMessage *msg, int proto)
{
    network = msg->readInt32();
    address = msg->readAddress();
    port = msg->readInt16();
    score = msg->readInt32();

    //qDebug()<<"ServerInfo::ServerInfo num="<<num<<"network="<<network<<"address="<<address<<"port="<<port<<"score="<<score;

    tags.clear();
    for (int i = msg->readInt16(); i; i--)
        if(! msg->readTag(tags)) return;

    if (proto < 28) {
        nusers = msg->readInt32();
        nfiles = msg->readInt32();
    } else {
        nusers = msg->readInt64();
        nfiles = msg->readInt64();
    }
    updateServerState(msg, proto);
    name = msg->readString();
    description = msg->readString();
    if (proto > 28)
        preferred = msg->readBool();
    else
        preferred = false;
}

ServerInfo::~ServerInfo()
{
}

void ServerInfo::updateServerState(DonkeyMessage* msg, int proto)
{
    state = (ClientInfo::State)msg->readInt8();
    if ( (proto >= 12 && (state == ClientInfo::Connected2 || state == ClientInfo::NotConnected2))
	|| (proto >= 21 && state == ClientInfo::Downloading) )
	msg->readInt32();
}

const int& ServerInfo::serverNo() const
{
    return num;
}

const QString& ServerInfo::serverName() const
{
    return name;
}

const int& ServerInfo::serverNetwork() const
{
    return network;
}

const QString& ServerInfo::serverDescription() const
{
    return description;
}

const QString& ServerInfo::serverAddress() const
{
    return address;
}

const int& ServerInfo::serverPort() const
{
    return port;
}

const int& ServerInfo::serverScore() const
{
    return score;
}

const int64& ServerInfo::serverNUsers() const
{
    return nusers;
}

const int64& ServerInfo::serverNFiles() const
{
    return nfiles;
}

const ClientInfo::State& ServerInfo::serverState() const
{
    return state;
}

const QMap<QString,QVariant>& ServerInfo::serverTags() const
{
    return tags;
}

const bool& ServerInfo::serverPreferred() const
{
    return preferred;
}

bool ServerInfo::isConnected() const
{
    return this
        && (state == ClientInfo::Connected
            || state == ClientInfo::Connected2
            || state == ClientInfo::Connected3);
}
