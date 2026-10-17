/* -*- c++ -*-
 *
 * clientinfo.h
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

#ifndef __libkmldonkey_clientinfo_h__
#define __libkmldonkey_clientinfo_h__

#include <kmldonkey_export.h>

#include <donkeytypes.h>

#include <QString>
#include <qmap.h>
#include <QVariant>

class DonkeyMessage;

/** Representation of a client.
 */

class KMLDONKEY_EXPORT ClientInfo
{

    public:

    /** Client state.
     */
    enum State {
        NotConnected = 0,
        Connecting,
        Initiating,
        Downloading,
        Connected,
        Connected2,
        NewHost,
        Removed,
        Blacklisted,
        NotConnected2,
	Connected3
    };

    /** Client type.
     */
    enum Type {
        NormalClient = 0,
        FriendClient,
        ContactClient
    };

    ClientInfo(int num, DonkeyMessage* msg, int proto);
    ~ClientInfo();

    void updateClientInfo(DonkeyMessage *msg, int proto);

    //! The numerical ID of the client.
    const int& clientNo() const;
    //! The network the client is on.
    const int& clientNetwork() const;
    //! String describing the kind of client, either DIRECT or INDIRECT.
    const QString& clientKind() const;
    //! The state of the client.
    const State& clientState() const;
    //! If the client is in state Connected, this is the position we are in the client's queue.
    const int& clientQueuePosition() const;
    //! The client type.
    const Type& clientType() const;
    //! Client tags.
    const QMap<QString,QVariant>& clientTags() const;
    //! The name of the client.
    const QString& clientName() const;
    //! The client's rating.
    const int& clientRating() const;
    //! The client's chat port.
    const int& clientChatPort() const;
    //! The client software.
    const QString& clientSoftware() const;
    //! The number of bytes downloaded from the client.
    const int64& clientDownloaded() const;
    //! The number of bytes uploaded to the client.
    const int64& clientUploaded() const;
    //! The file being uploaded to this client, or an empty string if nothing.
    const QString& clientUpload() const;
    //! The client's connect time.
    const int& clientConnectTime() const;
    //! The client's eMule mod.
    const QString& clientEmuleMod() const;
    //! The ID of the file currently being downloaded from this client.
    const int& clientDownloading() const;
    //! The client's software release.
    const QString& clientRelease() const;

    void setClientState(State newstate, int newqueue = 0);
    void setClientState(DonkeyMessage* msg, int proto);
    void setClientType(Type newtype);
    void setClientType(DonkeyMessage* msg, int proto);

private:

    int num, network;
    QString kind;
    State state;
    int queue;
    Type type;
    QMap<QString,QVariant> tags;
    QString name;
    int rating;
    int chatPort;
    QString software;
    int64 downloaded, uploaded;
    QString upload;
    int connectTime;
    QString emulemod;
    int downloading;
    QString release;

    Q_DISABLE_COPY(ClientInfo)
};

#endif
