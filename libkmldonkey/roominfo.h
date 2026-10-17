/* -*- c++ -*-
 *
 * roominfo.h
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

#ifndef __libkmldonkey_roominfo_h__
#define __libkmldonkey_roominfo_h__

#include <QString>
#include <QList>
#include <kmldonkey_export.h>

#include "donkeymessage.h"

/**
 * Presents a room-message in a mlnet-room.
 */
class KMLDONKEY_EXPORT RoomMessage
{
public:
    /**
     * Types of messages.
     */
    enum MessageType {
        Server = 0,
        Public,
        Private,
        Unknown
    };

    /**
     * Constructor.
     */
    RoomMessage(DonkeyMessage* msg, int proto);

    ///Destructor.
    ~RoomMessage();

    /**
     * Returns the messagetype.
     */
    MessageType messageType();

    /**
     * Returns the messagestring.
     */
    QString messageBody();

    /**
     * Returns the messageuser.
     */
    int messageUser();

private:
    MessageType type;
    QString message;
    int user;
};

/**
 * Presents a mlnet-room.
 */
class KMLDONKEY_EXPORT RoomInfo
{
public:
    /**
     * Possible states a room could have.
     */
    enum RoomState {
        Open = 0,
        Closed,
        Paused,
        Unknown
    };

    /**
     * Constructor.
     */
    RoomInfo(int num, DonkeyMessage* msg, int proto);
    ///Destructor
    ~RoomInfo();

    /**
     * Updates the RoomInfo-content.
     */
    void updateRoomInfo(DonkeyMessage* msg, int proto);

    /**
     * Each room does have a unique number.
     */
    int roomNum();

    /**
     * The name of this room.
     */
    QString roomName();

    /**
     * The network-number this room belongs to.
     */
    int roomNetwork();

    /**
     * State of this room (open, closed, paused, ...).
     */
    RoomState roomState();

    /**
     * Number of users in this room.
     */
    int roomUsers();

    /**
     * Return the room-messages.
     */
    QList<RoomMessage *> getMessages();

    /**
     * Add a room-message.
     */
    void addMessage(RoomMessage *);

private:
    int num;
    int network;
    QString name;
    RoomState state;
    int users;
    QList<RoomMessage *> messages;
};

#endif
