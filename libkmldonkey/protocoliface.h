/* -*- c++ -*-
 *
 * protocoliface.h
 *
 * Copyright (C) 2003 Sebastian Sauer <mail@dipe.org>
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

#ifndef __libkmldonkey_protocoliface_h__
#define __libkmldonkey_protocoliface_h__

#include <QObject>
#include <QMetaType>
#include <QAbstractSocket>

#include "hostiface.h"
#include "infoiface.h"
#include "kmldonkey_export.h"

/**
 * Interface-class for all the different gui-protocols. Each protocol
 * have to inherit this class and overwrite the functions as needed.
 */
class KMLDONKEY_EXPORT ProtocolInterface : public QObject
{
    Q_OBJECT
    Q_ENUMS(enumDisconnectError)
public:
    /**
     * Constructor
     */
    ProtocolInterface(const QString& corename, QObject* parent = 0);
    /**
     * Destructor
     */
    ~ProtocolInterface();

    /**
     * Each core does have an unique name. For the mlnet the name is "mldonkey"
     */
    QString getCoreName();

    /**
     * Each protocol does should have one HostInterface based class
     * for host-based options.
     */
    HostInterface* getHost();
    void setHost(HostInterface* h);

    /**
     * Is the protocol connected with core?
     * \return results true if a connection to core is established and false if there
     *         isn't a connection right now.
     */
    virtual bool isConnected();
    /**
     * Connect with core.
     * \return Protocol trues to connect with core (true) or wasn't able to (false).
     *         If the result is true wait for signalConnected() to wait till the
     *         connection is established.
     */
    virtual bool connectToCore();
    /**
     * Disconnect from core.
     * \return protocol does try to disconnect now (true) or isn't able to (false).
     *         If the result is true you should evaluate signalDisconnected() to
     *         know if the disconnect was successfully.
     */
    virtual bool disconnectFromCore();

    /**
     * Used to indicate why a connection to the core was closed.
     * Connect with signalDisconnected(int status) to handle this
     * cases.
     * This basically equals the QAbstractSocket::SocketError enum
     * but extends if with some own items.
     */
    enum enumDisconnectError {
        UnknownSocketError = QAbstractSocket::UnknownSocketError, // -1
        ConnectionRefusedError = QAbstractSocket::ConnectionRefusedError, // 0
        RemoteHostClosedError = QAbstractSocket::RemoteHostClosedError, // 1
        HostNotFoundError = QAbstractSocket::HostNotFoundError, // 2
        SocketAccessError = QAbstractSocket::SocketAccessError, // 3
        SocketResourceError = QAbstractSocket::SocketResourceError, // 4
        SocketTimeoutError = QAbstractSocket::SocketTimeoutError, // 5
        DatagramTooLargeError = QAbstractSocket::DatagramTooLargeError, // 6
        NetworkError = QAbstractSocket::NetworkError, // 7
        AddressInUseError = QAbstractSocket::AddressInUseError, // 8
        SocketAddressNotAvailableError = QAbstractSocket::SocketAddressNotAvailableError, // 9
        UnsupportedSocketOperationError = QAbstractSocket::UnsupportedSocketOperationError, // 10
        ProxyAuthenticationRequiredError = QAbstractSocket::ProxyAuthenticationRequiredError, // 12
        SslHandshakeFailedError = QAbstractSocket::SslHandshakeFailedError, // 13
        UnfinishedSocketOperationError = QAbstractSocket::UnfinishedSocketOperationError, // 11
        ProxyConnectionRefusedError = QAbstractSocket::ProxyConnectionRefusedError, // 14
        ProxyConnectionClosedError = QAbstractSocket::ProxyConnectionClosedError, // 15
        ProxyConnectionTimeoutError = QAbstractSocket::ProxyConnectionTimeoutError, // 16
        ProxyNotFoundError = QAbstractSocket::ProxyNotFoundError, // 17
        ProxyProtocolError = QAbstractSocket::ProxyProtocolError, // 18
        IncompatibleProtocolError, // Protocol isn't compatible.
        AuthenticationError, // Incorrect authentification (name/password).
        NoError // No error, disconnect was intentional.
    };

    enumDisconnectError errorValue(const QByteArray &name) const;
    QByteArray errorName(enumDisconnectError err) const;
    QString errorTextShort(enumDisconnectError err);
    QString errorTextLong(enumDisconnectError err);

#if 0
    /**
     * Checks if a InfoDict with that key exists.
     * \param key Unique name of the InfoDict
     * \return true if such a InfoDict exists else false
     */
    bool hasInfo(const QString& key) {
        return infomap.contains(key);
    }
    /**
     * Returns the with key and id defined InfoInterface-instance.
     * \param key Unique name of the InfoDict
     * \param id Unique number of the InfoInterface-instance inside the InfoDict.
     * \return The InfoInterface-instance or 0L if there exists no such instance.
     */
    InfoInterface* getInfo(const QString& key, int id) {
        return infomap.contains(key) ? (InfoInterface*)( infomap[key].find(id) ) : 0L;
    }
    /**
     * Returns the with key defined QIntDict.
     * \param key Unique name of the InfoDict
     * \return The InfoDict or 0L if there exists no such InfoDict.
     */
    InfoDict getInfos(const QString& key) {
        return infomap[key];
    }
#endif //deprecated?
signals:
    /**
     * This signal is emitted if the connection to the core was established
     * successfully.
     */
    void signalConnected();

    /**
     * This signal is emitted if connection to core failed.
     * \param err The enumDisconnectError value that contains the reason for the disconnect.
     */
    void signalDisconnected(int err);

private:
    QString corename;
    HostInterface* host;

protected:
    //InfoMap infomap; // each protocol have to add there supported InfoInterface-classes into this map.
};

#endif
