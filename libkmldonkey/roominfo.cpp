/* -*- c++ -*-
 *
 * roominfo.cpp
 *
 * Copyright (C) 2003,2004 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2003,2004 Petter Stokke <ummo@hellokitty.com>
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

#include "roominfo.h"

RoomMessage::RoomMessage(DonkeyMessage* msg, int proto)
{
    Q_UNUSED(proto);
    int t = msg->readInt8();
    switch (t) {
        case 0: {
            type = Server;
            user = -1; // no user cause it's a servermessage
            message = msg->readString();
        } break;
        case 1: {
            type = Public;
            user = msg->readInt32();
            message = msg->readString();
        } break;
        case 2: {
            type = Private;
            user = msg->readInt32();
            message = msg->readString();
        } break;
        default: {
            type = Unknown;
        } break;
    }
}

RoomMessage::~RoomMessage()
{
}

RoomMessage::MessageType RoomMessage::messageType() { return type; }
QString RoomMessage::messageBody() { return message; }
int RoomMessage::messageUser() { return user; }

RoomInfo::RoomInfo(int num, DonkeyMessage* msg, int proto)
{
    this->num = num;
    updateRoomInfo(msg, proto);
}


RoomInfo::~RoomInfo()
{
    qDeleteAll(messages);
}

void RoomInfo::updateRoomInfo(DonkeyMessage* msg, int /*proto*/)
{
    network = msg->readInt32();
    name = msg->readString();
    switch( msg->readInt8() ) {
      case 0: state = Open; break;
      case 1: state = Closed; break;
      case 2: state = Paused; break;
      default: state = Unknown; break;
    }
    users = msg->readInt32();
}

int RoomInfo::roomNum() { return num; }
QString RoomInfo::roomName() { return name; }
int RoomInfo::roomNetwork() { return network; }
RoomInfo::RoomState RoomInfo::roomState() { return state; }
int RoomInfo::roomUsers() { return users; }

QList<RoomMessage*> RoomInfo::getMessages() { return messages; }
void RoomInfo::addMessage(RoomMessage* msg) { messages.append(msg); }
