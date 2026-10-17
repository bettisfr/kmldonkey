/* -*- c++ -*-
 *
 * network.cpp
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


#include "network.h"
#include "donkeymessage.h"



Network::Network(int num, DonkeyMessage* msg, int proto)
{
    this->num = num;
    updateNetwork(msg, proto);
}

Network::~Network()
{
}

void Network::updateNetwork(DonkeyMessage *msg, int proto)
{
    Q_UNUSED(proto);
    name = msg->readString();
    enabled = msg->readInt8();
    config_file = msg->readString();
    uploaded = msg->readInt64();
    downloaded = msg->readInt64();
    connected = msg->readInt32();
    int j = msg->readInt16();
    int flag = 0;
    for (int i=0; i<j; i++) {
        flag |= 1 << msg->readInt16();
    }
    flags = (NetworkFlags)flag;
}


const int& Network::networkNo() const { return num; }
const QString& Network::networkName() const { return name; }
const bool& Network::networkEnabled() const { return enabled; }
const QString& Network::networkConfigFile() const { return config_file; }
const int64& Network::networkUploaded() const { return uploaded; }
const int64& Network::networkDownloaded() const { return downloaded; }
const int& Network::networkConnected() const { return connected; }
const Network::NetworkFlags& Network::networkFlags() const { return flags; }

