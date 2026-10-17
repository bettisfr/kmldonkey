/* -*- c++ -*-
 *
 * serverinfo.h
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

#ifndef __libkmldonkey_serverinfo_h__
#define __libkmldonkey_serverinfo_h__

#include <clientinfo.h>

#include <QString>
#include <QStringList>
#include <QVariant>

class DonkeyMessage;

//! Representation of a server.

class KMLDONKEY_EXPORT ServerInfo
{

public:

    ServerInfo(int num, DonkeyMessage* msg, int proto);
    ~ServerInfo();

    void updateServerInfo(DonkeyMessage *msg, int proto);
    void updateServerState(DonkeyMessage* msg, int proto);

    //! The numerical ID of this server.
    const int& serverNo() const;
    //! The name of the server.
    const QString& serverName() const;
    //! The network the server is on.
    const int& serverNetwork() const;
    //! The server's description.
    const QString& serverDescription() const;
    //! The network address of the server.
    const QString& serverAddress() const;
    //! The port the server listens on.
    const int& serverPort() const;
    //! The server's score.
    const int& serverScore() const;
    //! The number of users connected to the server.
    const int64& serverNUsers() const;
    //! The number of files the server is indexing.
    const int64& serverNFiles() const;
    //! The state of the server.
    const ClientInfo::State& serverState() const;
    //! The server tags.
    const QMap<QString,QVariant>& serverTags() const;
    //! The server's preferred flag.
    const bool& serverPreferred() const;
    /// Returns true if this != NULL and state is one of Connected,..2,..3
    bool isConnected() const;
private:

    int num, network;
    QString name, description, address;
    int port;
    int score;
    int64 nusers, nfiles;
    ClientInfo::State state;
    QMap<QString,QVariant> tags;
    bool preferred;

    Q_DISABLE_COPY(ServerInfo)
};

#endif
