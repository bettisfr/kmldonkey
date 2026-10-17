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

#include "protocoliface.h"

#include "hostiface.h"
#include "infoiface.h"

#include <QMetaObject>
#include <QMetaEnum>

ProtocolInterface::ProtocolInterface(const QString& corename, QObject* parent)
    : QObject(parent)
{
    this->corename = corename;
    host = 0;
}

ProtocolInterface::~ProtocolInterface() {}

QString ProtocolInterface::getCoreName() { return corename; }

HostInterface* ProtocolInterface::getHost() { return host; }
void ProtocolInterface::setHost(HostInterface* h) { host = h; }

bool ProtocolInterface::isConnected() { return false; }
bool ProtocolInterface::connectToCore() { return false; }
bool ProtocolInterface::disconnectFromCore() { return false; }

ProtocolInterface::enumDisconnectError ProtocolInterface::errorValue(const QByteArray &name) const
{
    QMetaEnum metaEnum = metaObject()->enumerator(metaObject()->indexOfEnumerator("enumDisconnectError"));
    Q_ASSERT(metaEnum.isValid());
    return ProtocolInterface::enumDisconnectError(metaEnum.keyToValue(name.constData()));
}

QByteArray ProtocolInterface::errorName(ProtocolInterface::enumDisconnectError err) const
{
    QMetaEnum metaEnum = metaObject()->enumerator(metaObject()->indexOfEnumerator("enumDisconnectError"));
    Q_ASSERT(metaEnum.isValid());
    return QByteArray(metaEnum.valueToKey(err));
}

QString ProtocolInterface::errorTextShort(ProtocolInterface::enumDisconnectError err)
{
    switch (err) {
        case UnknownSocketError:
            return tr("Unknown error");
        case ConnectionRefusedError:
            return tr("Connection refused");
        case RemoteHostClosedError:
            return tr("Host closed connection");
        case HostNotFoundError:
            return tr("Host address not found");
        case SocketAccessError:
            return tr("Access error");
        case SocketResourceError:
            return tr("Resource error");
        case SocketTimeoutError:
            return tr("Timed out error");
        case DatagramTooLargeError:
            return tr("Datagram too large");
        case NetworkError:
            return tr("Network error");
        case AddressInUseError:
            return tr("Address in used");
        case SocketAddressNotAvailableError:
            return tr("Address not available");
        case UnsupportedSocketOperationError:
            return tr("Unsupported operation");
        case ProxyAuthenticationRequiredError:
            return tr("Proxy-authentication required");
        case SslHandshakeFailedError:
            return tr("SSL handshake failed");
        case UnfinishedSocketOperationError:
            return tr("Unfinished operation");
        case ProxyConnectionRefusedError:
            return tr("Proxy-connection refused");
        case ProxyConnectionClosedError:
            return tr("Proxy-connection closed");
        case ProxyConnectionTimeoutError:
            return tr("Proxy-connection timeout");
        case ProxyNotFoundError:
            return tr("Proxy not found");
        case ProxyProtocolError:
            return tr("Proxy protocol error");
        case IncompatibleProtocolError:
            return tr("Incompatible protocol error");
        case AuthenticationError:
            return tr("Host authentication error");
        case NoError:
            return tr("Disconnected");
    }
    return QString();
}

QString ProtocolInterface::errorTextLong(ProtocolInterface::enumDisconnectError err)
{
    HostInterface *host = getHost();
    QString hostAddress = host ? host->address() : tr("Unknown");
    QString hostAddressPort = host ? QString("%1:%2").arg(host->address()).arg(host->port()) : tr("Unknown");

    switch (err) {
        case UnknownSocketError:
            return tr("An unidentified error occurred.");
        case ConnectionRefusedError:
            return tr("The connection was refused by the peer (or timed out). Are you sure there is a MLNet-server running at %1 that allows our IP-address to connect?").arg(hostAddressPort);
        case RemoteHostClosedError:
            return tr("The remote host closed the connection.");
        case HostNotFoundError:
            return tr("The host address %1 was not found. Are you sure the address is correct?").arg(hostAddress);
        case SocketAccessError:
            return tr("The socket operation failed because the application lacked the required privileges.");
        case SocketResourceError:
            return tr("The local system ran out of resources (e.g., too many sockets).");
        case SocketTimeoutError:
            return tr("The socket operation timed out.");
        case DatagramTooLargeError:
            return tr("The datagram was larger than the operating system's limit.");
        case NetworkError:
            return tr("An error occurred with the network (e.g. the network cable was accidentally plugged out).");
        case AddressInUseError:
            return tr("The address specified to bind the socket is already in use and was set to be exclusive.");
        case SocketAddressNotAvailableError:
            return tr("The address specified to bind the socket does not belong to the host.");
        case UnsupportedSocketOperationError:
            return tr("The requested socket operation is not supported by the local operating system (e.g. lack of IPv6 support).");
        case ProxyAuthenticationRequiredError:
            return tr("The socket is using a proxy, and the proxy requires authentication.");
        case SslHandshakeFailedError:
            return tr("The SSL/TLS handshake failed, so the connection was closed.");
        case UnfinishedSocketOperationError:
            return tr("The last operation attempted has not finished yet (still in progress in the background).");
        case ProxyConnectionRefusedError:
            return tr("Could not contact the proxy server because the connection to that server was denied.");
        case ProxyConnectionClosedError:
            return tr("The connection to the proxy server was closed unexpectedly (before the connection to the final peer was established).");
        case ProxyConnectionTimeoutError:
            return tr("The connection to the proxy server timed out or the proxy server stopped responding in the authentication phase.");
        case ProxyNotFoundError:
            return tr("The defined proxy address was not found.");
        case ProxyProtocolError:
            return tr("The connection negotiation with the proxy server because the response from the proxy server could not be understood.");
        case IncompatibleProtocolError:
            return tr("The MLNet-server uses a to KMLDonkey incompatible GUI-protocol.");
        case AuthenticationError:
            return tr("The MLNet-server denied access to the define username and password. Are you sure they are correct?");
        case NoError:
            return QString();
    }
    return QString();
}
