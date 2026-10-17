/* -*- c++ -*-
 *
 * donkeyprotocol.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006 Christian Muehlhaeuser <chris@chris.de>
 * Copyright (C) 2009 Gioacchino Mazzurco <gmazzurco89@gmail.com>
 * Copyright (C) 2009 Aleksey Markelov <markelovai@gmail.com>
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

#include "donkeyprotocol.h"

#include "donkeysocket.h"
#include "fileinfo.h"
#include "serverinfo.h"
#include "network.h"
#include "clientinfo.h"
#include "shareinfo.h"
#include "searchinfo.h"
#include "hostmanager.h"
#include "hostiface.h"
#include "donkeyhost.h"
#include "searchquery.h"
#include "torrenthost.h"
#include "options.h"

#include <QTimer>
#include <QRegExp>
#include <QFileInfo>
#include <QHostAddress>
#include <QNetworkProxy>
#include <QAuthenticator>
#include <QDebug>

template<class T> void ptrContainerClear(T &cont)
{
    qDeleteAll(cont);
    cont.clear();
}

DonkeyProtocol::DonkeyProtocol(bool poll, QObject *parent)
    : ProtocolInterface("mldonkey", parent)
    , m_socket(new DonkeySocket(this))
{
    uname = "admin";
    passwd = "";
    donkeyError = NoError;
    connectedservers = 0;
    proto = MIN_PROTOCOL_VERSION;
    wantpoll = poll;
    connect(m_socket, SIGNAL(readyMessage()), SLOT(processMessage()));
    connect(m_socket, SIGNAL(disconnected()), SLOT(socketDisconnected()));
    connect(m_socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), SLOT(socketError(QAbstractSocket::SocketError)));
    connect(m_socket, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)), SLOT(socketProxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)));
    connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
}

DonkeyProtocol::~DonkeyProtocol()
{
    flushState();
}

bool DonkeyProtocol::poll() const
{
    return wantpoll;
}

void DonkeyProtocol::setPassword(const QString& username, const QString& pwd)
{
    uname = username;
    passwd = pwd;
}

void DonkeyProtocol::setPassword(const QString& pwd)
{
    uname = "admin";
    passwd = pwd;
}

const QString& DonkeyProtocol::username()
{
    return uname;
}

const QString& DonkeyProtocol::password()
{
    return passwd;
}

bool DonkeyProtocol::isConnected()
{
    return (m_socket->state() == DonkeySocket::ConnectedState);
}

QAbstractSocket::SocketState DonkeyProtocol::connectionState() const
{
    return m_socket->state();
}

bool DonkeyProtocol::connectToCore()
{
    donkeyError = NoError;
    if (isConnected()) {
        qDebug() << "Is already connected. Trying to disconnect...";
        if (! disconnectFromCore()) {
            qDebug() << "Failed to disconnect.";
            return false;
        }
    }
    else
        flushState();

    DonkeyHost *host = (DonkeyHost*)getHost();
    if(host) {
        setPassword(host->username(), host->password());
        m_socket->connectDonkey(host->address(), host->port());
    }
    else {
        m_socket->connectDonkey();
    }

    return true;
}

bool DonkeyProtocol::disconnectFromCore()
{
    // The QSocket-documentation says that unwritten data will be written
    // on socket.close(). But that doesn't seem to be the case if
    // disconnectFromCore() is called on KMLDonkey::queryClose() cause
    // the stopSearch()'s called at SearchPage::stopAllSearches() got
    // lost. So, we've explicit to call flush() to be sure.
    if (m_socket->state() == QAbstractSocket::UnconnectedState)
        return false;
    m_socket->flush();
    m_socket->close();
    flushState();
    return true;
}

void DonkeyProtocol::flushState()
{
    qDebug() << "DonkeyProtocol::flushState";
    ptrContainerClear(download);
    ptrContainerClear(downloaded);
    upload.clear();
    ptrContainerClear(servers);
    ptrContainerClear(networks);
    ptrContainerClear(clients);
    ptrContainerClear(shares);
    ptrContainerClear(searches);
    ptrContainerClear(unmappedSearches);
    ptrContainerClear(unmappedResults);
    options.clear();
    friends.clear();
    ptrContainerClear(rooms);
    m_sectionOptions.clear();
    m_pluginOptions.clear();
    ptrContainerClear(consoleCallbacks);
    ptrContainerClear(defSearches);
    connectedservers = 0;
    downloadstarted = false;
}

void DonkeyProtocol::socketDisconnected()
{
    qDebug() << "DonkeyProtocol::socketDisconnected err=" << donkeyError;
    emit signalDisconnected(donkeyError);
    flushState();
}

void DonkeyProtocol::socketStateChanged(QAbstractSocket::SocketState socketState)
{
    QString s;
    switch(socketState) {
        case QAbstractSocket::UnconnectedState: s = "Unconnected"; break;
        case QAbstractSocket::HostLookupState: s = "HostLookup"; break;
        case QAbstractSocket::ConnectingState: s = "Connecting"; break;
        case QAbstractSocket::ConnectedState: s = "Connected"; break;
        case QAbstractSocket::BoundState: s = "Bound"; break;
        case QAbstractSocket::ClosingState: s = "Closing"; break;
        case QAbstractSocket::ListeningState: s = "Listening"; break;
    }
    qDebug() << "DonkeyProtocol::socketStateChanged socketState=" << s;
    emit stateChanged(socketState);
}

void DonkeyProtocol::socketError(QAbstractSocket::SocketError err)
{
    qDebug() << "DonkeyProtocol::socketError isConnected=" << isConnected() << "err=" << err << m_socket->errorString();
    donkeyError = (enumDisconnectError) err;
    if (isConnected())
        disconnectFromCore();
    else
        emit signalDisconnected(err);
}

void DonkeyProtocol::socketProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
    qDebug() << "DonkeyProtocol::socketProxyAuthenticationRequired proxy=" << QString("%1@%2:%3").arg(proxy.user()).arg(proxy.hostName()).arg(proxy.port());
    Q_UNUSED(authenticator);
    //TODO
}

void DonkeyProtocol::pruneClientRecord(int clientno, DonkeyProtocol::FromCore reason)
{
    foreach (FileInfo *fi, download) {
        if (fi->removeSource(clientno)) {
            emit fileUpdated(fi->fileNo(), reason);
            emit fileSourceRemoved(fi->fileNo(), clientno);
        }
    }
    if (friends.removeAll(clientno))
        emit friendRemoved(clientno);
}

const int& DonkeyProtocol::protocolVersion()
{
    return proto;
}

void DonkeyProtocol::processMessage()
{
    QMap<int, QList<ResultInfo*> > searchesUpdated;
    DonkeyMessage* msg;
    while ((msg = m_socket->popMessage())) {
        //qDebug()<<"DonkeyProtocol::processMessage opcode="<<msg->opcode();
        DonkeyMessage* out;
        QString baz;
        emit messageReceived(msg);
        switch (msg->opcode()) {
        case CoreProtocol:
            proto = msg->readInt32();
            // FIXME: We might read the protocol 26 max_to_gui and max_from_gui values here (both int32).
            // We don't, because we don't really need them for anything useful right now.
            qDebug()<<"CoreProtocol message, version="<<proto<<"MIN_PROTOCOL_VERSION="<<MIN_PROTOCOL_VERSION<<"MAX_PROTOCOL_VERSION="<<MAX_PROTOCOL_VERSION;
            if (proto < MIN_PROTOCOL_VERSION) {
                qDebug() << "Error: Obsolete protocol version!";
                donkeyError = IncompatibleProtocolError;
                disconnectFromCore();
                break;
            }
            out = new DonkeyMessage(GuiProtocol);
            out->writeInt32(MAX_PROTOCOL_VERSION);
            m_socket->sendMessage(*out);
            coreProto = proto;
            if (proto > MAX_PROTOCOL_VERSION)
                proto = MAX_PROTOCOL_VERSION;
            qDebug()<<"Using protocol="<<proto<<"wantpoll="<<wantpoll;
            delete out;
            if (wantpoll) {
                out = new DonkeyMessage(GuiExtensions);
                out->writeInt16(1);
                out->writeInt32(1);
                out->writeInt8(1);
                m_socket->sendMessage(*out);
                delete out;

                // MLNet does not send the DefineSearches if polling is enabling and there exists no
                // way to ask MLNet to send the defined searches so we need to provide them our own.
                QueryAnd *q = new QueryAnd();
                q->append(new QueryKeywords("Keywords", ""));
                QueryAnd *so = new QueryAnd();
                so->append(new QueryMinSize("Min size", ""));
                so->append(new QueryMaxSize("Max size", ""));
                so->append(new QueryMedia("Media", ""));
                so->append(new QueryFormat("Format", ""));
                q->append(new QueryModule("Simple Options", so));
                QueryAnd *mp3 = new QueryAnd();
                mp3->append(new QueryMp3Artist("Artist", ""));
                mp3->append(new QueryMp3Album("Album", ""));
                mp3->append(new QueryMp3Title("Title", ""));
                mp3->append(new QueryMp3Bitrate("Bitrate", ""));
                q->append(new QueryModule("Mp3 Options", mp3));
                q->setParent(this);
                defSearches.insert("Complex Search", q);
            }
            out = new DonkeyMessage(Password);
            out->writeString(passwd);
            out->writeString(uname);
            m_socket->sendMessage(*out);
            delete out;
            if (proto >= 31) requestVersion();
            emit signalConnected();
            if (wantpoll) emit definedSearchesUpdated();
            break;
        case Console: {
            baz = msg->readString();
            bool hadCallback = false;
            if (!consoleCallbacks.isEmpty()) {
                QRegExp rx("^\n[\\-]+\nEval[\\s]command:[\\s]([^\n]+)\n\n(.*)");
                if (rx.exactMatch(baz)) {
                    QString cmd = rx.cap(1).trimmed();
                    QString res = rx.cap(2).trimmed();
                    ConsoleCallbackInterface* cb = consoleCallbacks.take(cmd);
                    if (cb) {
                        // Deliver errors too, so callers can report unsupported commands.
                        cb->callback(cmd, res);
                        // check if this callback is referenced several times before deleting
                        if (consoleCallbacks.keys(cb).isEmpty())
                            delete cb;
                        hadCallback = true;
                    }
                }
            }
            if (!hadCallback) emit consoleMessage(baz);
        } break;
        case BadPassword:
            qDebug() << "Bad password!";
            donkeyError = AuthenticationError;
            disconnectFromCore();
            break;
        case Client_stats_v1:
        case Client_stats_v2:
        case Client_stats_v3:
            qDebug() << "Obsolete client stats message received";
            break;
        case Client_stats:
        {
            int64 ul = msg->readInt64();
            int64 dl = msg->readInt64();
            int64 sh = msg->readInt64();
            int32 nsh = msg->readInt32();
            int32 tul = msg->readInt32();
            int32 tdl = msg->readInt32();
            int32 uul = msg->readInt32();
            int32 udl = msg->readInt32();
            int32 ndl = msg->readInt32();
            int32 ncp = msg->readInt32();

            clientstatsmap.clear();
            int i,j = msg->readInt16();
            for (i=0; i<j; i++) {
                int nw = msg->readInt32();
                int s = msg->readInt32();
                clientstatsmap.insert(nw, s);
            }
            emit clientStats(ul,dl,sh,nsh,tul,tdl,uul,udl,ndl,ncp,&clientstatsmap);
        } break;
        case File_add_source:
        {
            int fn = msg->readInt32();
            int cl = msg->readInt32();
            FileInfo* file = findDownloadFileNo(fn);
            if (file) {
                file->addSource(cl);
                emit fileUpdated(fn, File_add_source);
                emit fileSourceUpdated(fn,cl);
            }
        } break;
        case File_remove_source:
        {
            int fn = msg->readInt32();
            int cl = msg->readInt32();
            FileInfo* file = findDownloadFileNo(fn);
            if (file) {
                file->removeSource(cl);
                emit fileUpdated(fn, File_remove_source);
                emit fileSourceRemoved(fn,cl);
            }
        } break;
        case File_update_availability:
        {
            int fn = msg->readInt32();
            int cl = msg->readInt32();
            QByteArray av = msg->readByteArray();
            FileInfo* file = findDownloadFileNo(fn);
            if (file) {
                file->updateAvailability(cl, av);
                emit fileUpdated(fn, File_update_availability);
                emit fileSourceUpdated(fn,cl);
            }
        } break;
        case Client_info:
        {
            int cn = msg->readInt32();
            ClientInfo *client = findClientNo(cn);
            if (client) {
                client->updateClientInfo(msg,proto);
            } else {
                client = new ClientInfo(cn, msg, proto);
                clients.insert(cn, client);
            }

            if (client->clientType() == ClientInfo::FriendClient) {
                if (!friends.contains(cn)) {
                    friends.append(cn);
                    emit friendUpdated(cn);
                }
            }
            else if (friends.removeAll(cn))
                emit friendRemoved(cn);

            if (client->clientState() == ClientInfo::Removed) {
                pruneClientRecord(cn, Client_info);
                delete clients.take(cn);
                emit clientRemoved(cn);

                if (upload.contains(cn)) {
                    upload.remove(cn);
                    emit uploadRemoved(cn);
                }
            } else {
                emit clientUpdated(cn);

                if (upload.contains(cn)) {
                    if (upload[cn])
                        emit uploadUpdated(cn);
                    else {
                        upload.remove(cn);
                        emit uploadRemoved(cn);
                    }
                }
            }

        } break;
        case Client_state:
        {
            int clno = msg->readInt32();
            ClientInfo* client = findClientNo(clno);
            if (!client) {
                refreshClientInfo(clno);
                break;
            }
            client->setClientState(msg, proto);
            switch (client->clientState()) {
            case ClientInfo::Removed:
                emit clientRemoved(clno);
                pruneClientRecord(clno, Client_state);
                delete clients.take(clno);
                break;
            default:
                emit clientUpdated(clno);
                break;
            }
        } break;
        case Client_friend:
        {
            int clno = msg->readInt32();
            ClientInfo* client = findClientNo(clno);
            if (!client) {
                refreshClientInfo(clno);
                break;
            }
            client->setClientType(msg, proto);
            switch (client->clientType()) {
            case ClientInfo::FriendClient:
                if (!friends.contains(clno)) {
                    friends.append(clno);
                    emit friendUpdated(clno);
                }
                break;
            default:
                if (friends.removeAll(clno))
                    emit friendRemoved(clno);
            }
            emit clientUpdated(clno);
        } break;
        /*
        case Client_file:
        {
            qDebug() << "Client_file message: " << msg->readInt32() << " \"" << msg->readString() << "\" " << msg->readInt32();
        } break;
        */
        case DownloadFiles_v1:
        case DownloadFiles_v2:
        case DownloadFiles_v3:
            qDebug() << "Obsolete download files message received";
            break;
        case DownloadFiles_v4:
        case DownloadFiles:
        {
            ptrContainerClear(download);
            int i, j = msg->readInt16();
            for (i=0; i<j; i++) {
                int fn = msg->readInt32();
                FileInfo *fi = findDownloadFileNo(fn);
                if (fi) {
                    fi->updateFileInfo(msg, proto);
                    qDebug() << "DownloadFiles UPDATE fileno=" << fn << "state=" << fi->fileState() << "download=" << (download.contains(fn)?download[fn]->fileName():"") << "downloaded=" << (downloaded.contains(fn)?downloaded[fn]->fileName():"");
                } else {
                    fi = new FileInfo(fn, msg, proto);
                    qDebug() << "DownloadFiles NEW fileno=" << fn << "state=" << fi->fileState() << "download=" << (download.contains(fn)?download[fn]->fileName():"") << "downloaded=" << (downloaded.contains(fn)?downloaded[fn]->fileName():"");
                    download.insert(fn, fi);
                }
            }
            emit updatedDownloadFiles();
        } break;
        case File_downloaded_v1:
        case File_downloaded: // recevied when a file progressed on downloading
        {
            int fn = msg->readInt32();
            FileInfo* fi = download.value(fn);
            // Progress packets contain only counters, not a complete FileInfo.
            // They may still arrive after cancellation or completion.
            if (!fi)
                break;
            fi->updateDownloadStatus(msg, proto);
            switch (fi->fileState()) {
            case FileInfo::Shared:
            case FileInfo::Cancelled:
            case FileInfo::Aborted:
                qDebug() << "File_downloaded DEL fileno=" << fn << "state=" << fi->fileState() << "download=" << (download.contains(fn)?download[fn]->fileName():"") << "downloaded=" << (downloaded.contains(fn)?downloaded[fn]->fileName():"");
                delete download.take(fn);
                emit fileRemoved(fn);
                break;
            default:
                emit fileUpdated(fn, File_downloaded);
                break;
            }
        } break;
        case DownloadedFiles_v1:
            qDebug() << "Obsolete downloaded files message received";
            break;
        case DownloadedFiles_v2:
        case DownloadedFiles:
        {
            ptrContainerClear(downloaded);
            int i, j = msg->readInt16();
            for (i=0; i<j; i++) {
                int fn = msg->readInt32();
                FileInfo *fi = downloaded.value(fn);
                qDebug() << "DownloadedFiles fileno=" << fn << "state=" << (fi?fi->fileState():-1) << "download=" << (download.contains(fn)?download[fn]->fileName():"") << "downloaded=" << (downloaded.contains(fn)?downloaded[fn]->fileName():"");
                if (fi) {
                    fi->updateFileInfo(msg, proto);
                } else {
                    fi = download.take(fn);
                    if (fi)
                        fi->updateFileInfo(msg, proto);
                    else
                        fi = new FileInfo(fn, msg, proto);
                    downloaded.insert(fn, fi);
                }
            }
            emit updatedDownloadedFiles();
        } break;
        case File_info_v1:
        case File_info_v2:
            qDebug() << "Obsolete file info message received";
            break;
        case File_info_v3:
        case File_info:
        {
            int fn = msg->readInt32();
            FileInfo* fi = findDownloadFileNo(fn);
            bool isDownload = fi != 0;
            bool isNew = false;
            if (fi) {
                Q_ASSERT(!findDownloadedFileNo(fn)); // the same file should not be in both lists
            } else {
                fi = findDownloadedFileNo(fn);
                if (!fi) {
                    isNew = true;
                    fi = new FileInfo(fn, msg, proto);
                    if (fi->fileState() == FileInfo::Complete)
                        downloaded.insert(fn, fi);
                    else
                        download.insert(fn, fi);
                }
            }
            if (!isNew) {
                fi->updateFileInfo(msg, proto);
            }
            if (isDownload && fi->fileState() == FileInfo::Complete) {
                qDebug() << "File_info movedFromDownloadToDownloaded fileno=" << fn << "state=" << fi->fileState() << "isNew=" << isNew << "download=" << (download.contains(fn)?download[fn]->fileName():"") << "downloaded=" << (downloaded.contains(fn)?downloaded[fn]->fileName():"");
                isDownload = false;
                downloaded.insert(fn, download.take(fn));
            }
            switch (fi->fileState()) {
            case FileInfo::Shared:
            case FileInfo::Cancelled:
            case FileInfo::Aborted:
                qDebug() << "File_info fileno=" << fn << "state=" << fi->fileState() << "isNew=" << isNew << "download=" << (download.contains(fn)?download[fn]->fileName():"") << "downloaded=" << (downloaded.contains(fn)?downloaded[fn]->fileName():"");
                delete downloaded.take(fn);
                delete download.take(fn);
                if (!isNew) emit fileRemoved(fn);
                break;
            default:
                if (isNew) {
                    qDebug() << "File_info fileno=" << fn << "state=" << fi->fileState() << "isNew=" << isNew << "download=" << (download.contains(fn)?download[fn]->fileName():"") << "downloaded=" << (downloaded.contains(fn)?downloaded[fn]->fileName():"");
                    emit fileAdded(fn, downloadstarted);
                } else {
                    //qDebug() << "File_info fileno=" << fn << "state=" << fi->fileState() << "isNew=" << isNew << "download=" << (download.contains(fn)?download[fn]->fileName():"") << "downloaded=" << (downloaded.contains(fn)?downloaded[fn]->fileName():"");
                    emit fileUpdated(fn, File_info);
                }
                break;
            }
        } break;
        case ConnectedServers:
        {
            ptrContainerClear(servers);
            connectedservers = 0;
            int i, j = msg->readInt16();
            for (i=0; i<j; i++) {
                int sn = msg->readInt32();
                ServerInfo *si = findServerNo(sn); //should always return null actually
                if (si) {
                    si->updateServerInfo(msg,proto);
                } else {
                    si = new ServerInfo(sn, msg, proto);
                    servers.insert(sn, si);
                    if (si->isConnected()) ++connectedservers;
                }
            }
            emit updatedConnectedServers();
        } break;
        case Server_info_v1:
        case Server_info:
        {
            int sn = msg->readInt32();
            ServerInfo *si = findServerNo(sn);
            bool oldstate = false;
            if (si) {
                oldstate = si->isConnected();
                si->updateServerInfo(msg,proto);
            } else {
                si = new ServerInfo(sn, msg, proto);
                servers.insert(sn, si);
            }
            bool newstate = si->isConnected();

            if (oldstate != newstate) connectedservers += newstate ? 1 : -1;

            switch (si->serverState()) {
            case ClientInfo::Removed:
                emit serverRemoved(sn);
                delete servers.take(sn);
                break;
            default:
                emit serverUpdated(sn);
                break;
            }
        } break;
        case Server_state:
        {
            int sn = msg->readInt32();
            ServerInfo* si = findServerNo(sn);
            if (!si) {
                qDebug() << "Core sent an invalid server number!";
                break;
            }
            bool oldstate = si->isConnected();
            si->updateServerState(msg,proto);
            bool newstate = si->isConnected();
            if (oldstate != newstate) connectedservers += newstate ? 1 : -1;
            switch (si->serverState()) {
            case ClientInfo::Removed:
                emit serverRemoved(sn);
                delete servers.take(sn);
                break;
            default:
                emit serverUpdated(sn);
                break;
            }
        } break;
        case Network_info:
        {
            int nn = msg->readInt32();
            Network *nw = findNetworkNo(nn);
            if (nw) {
                nw->updateNetwork(msg,proto);
            } else {
                nw = new Network(nn, msg, proto);
                networks.insert(nn, nw);
            }
            emit networkUpdated(nn);
        } break;
        case Shared_file_info_v1:
        case Shared_file_info:
        {
            int sn = msg->readInt32();
            ShareInfo *si = findShareNo(sn);
            if (si) {
                si->updateShareInfo(msg, proto);
            } else {
                si = new ShareInfo(sn, msg, proto);
                shares.insert(sn, si);
            }
            emit shareUpdated(sn);
        } break;
        case Shared_file_upload:
        {
            int sn = msg->readInt32();
            ShareInfo* si = findShareNo(sn);
            if (si) {
                si->updateShare(msg, proto);
                emit shareUpdated(sn);
            }
        } break;
        case Shared_file_unshared:
        {
            int shno = msg->readInt32();
            if (ShareInfo *si = shares.take(shno)) {
                emit shareRemoved(shno);
                delete si;
            }
        } break;
        case Options_info:
        {
            int i, j = msg->readInt16();
            bool ok = true;
            for (i=0; i<j; i++) {
                QString key, value;
                key = msg->readString(&ok);
                if (! ok) break;
                value = msg->readString(&ok);
                if (! ok) break;
                options.insert(key,value);
            }
            emit optionsUpdated();
        } break;

        case DefineSearches: {
            qDebug() << "DonkeyProtocol received DefineSearches";
            ptrContainerClear(defSearches);
            defSearch = QString();
            int i, j = msg->readInt16();
            for (i=0; i<j; i++) {
                QString name = msg->readString();
                SearchQuery *sq = SearchQuery::getQuery(msg);
                if (sq) {
                    if (defSearch.isEmpty()) defSearch = name; // first item is always the default one
                    sq->setParent(this);
                    defSearches.insert(name, sq);
                }
            }
            emit definedSearchesUpdated();
        } break;

        // Following both messages are used to transfer the Searchresults back.
        // First Result_info's are recieved. Later the Search_result's defines
        // to what Search the previous noted Result_info's belongs too.
        // MLDonkey does cache searches. So, if you start again a search for
        // something that was previously searched for, it returns only
        // Search_result's.
        case Result_info:
        {
            ResultInfo *si = new ResultInfo(msg, proto);
            unmappedResults.insert(si->resultNo(), si);
        } break;

        case Search_result:
        {
            int searchnum = msg->readInt32();
            int resultnum = msg->readInt32();
            SearchInfo *si = findSearchNo(searchnum);
            if (!si) {
                si = unmappedSearches.take(searchnum);
                if (!si) {
                    qDebug()<<"Got Search_result but have no SearchInfo for searchnum=" << searchnum;
                    si = new SearchInfo(searchnum);
                }
                searches.insert(searchnum, si);
            }
            si->setFetched(true);
            ResultInfo* ri = unmappedResults.value(resultnum);
            if (ri) {
                si->addResult(ri);
                //Q_ASSERT(!searchesUpdated[searchnum].contains(ri));
                searchesUpdated[searchnum].append(ri);
            } else {
                qDebug () << "No ResultInfo for resultnum=" << resultnum << "in searchnum=" << searchnum;
            }
        } break;

        case Pending:
        {
            //TODO
            int i, j = msg->readInt16();
            QString o("Pending: ");
            for (i=0; i<j; i++) {
                int v = msg->readInt32();
                o += QString::number(v) + " ";
            }
            qDebug()<<"PENDING" << o;
        } break;

        // We get those message as result of getSearches() requests.
        case Search:
        {
            int searchnum = msg->readInt32();
            SearchInfo *si = searches.value(searchnum);
            if (! si) {
                si = unmappedSearches.take(searchnum);
                if (!si) {
                    si = new SearchInfo(searchnum);
                }
                searches.insert(searchnum, si);
            }
            si->setQuery(msg);
            emit searchRequest(searchnum);
        } break;

        case Client_file:
        {
            int clno = msg->readInt32();
            QString dir = msg->readString();
            int result = msg->readInt32();
            emit clientFileListing(clno, dir, result);
        } break;

        case CleanTables:
        {
            ClientInfoHash newClients;
            int i, j = msg->readInt16();
            for (i=0; i<j; i++) {
                int cn = msg->readInt32();
                ClientInfo* cl = clients.take(cn);
                if (cl) newClients.insert(cn, cl);
            }
            ClientInfoHash::iterator it = clients.begin(), itend = clients.end();
            for (; it != itend; ++it) {
                emit clientRemoved(it.key());
                pruneClientRecord(it.key(), CleanTables);
            }
            ptrContainerClear(clients);
            clients = newClients;

            ServerInfoHash newServers;
            j = msg->readInt16();
            for (i=0; i<j; i++) {
                int sn = msg->readInt32();
                ServerInfo* si = servers.take(sn);
                if (si) newServers.insert(sn, si);
            }
            ServerInfoHash::iterator sit = servers.begin(), sitend = servers.end();
            for (; sit != sitend; ++sit) {
                emit serverRemoved(sit.key());
            }
            ptrContainerClear(servers);
            servers = newServers;
        } break;

        case MessageFromClient:
        {
            int clno = msg->readInt32();
            if (!findClientNo(clno))
                refreshClientInfo(clno);
            emit messageFromClient(clno, msg->readString());
        } break;

        case Add_section_option:
        {
            DonkeyOption opt(msg, proto);
            m_sectionOptions.append(opt);
            emit newSectionOption(opt);
        } break;

        case Add_plugin_option:
        {
            DonkeyOption opt(msg, proto);
            m_pluginOptions.append(opt);
            emit newPluginOption(opt);
        } break;

        case Room_info_v2:
        case Room_info:
        {
            int roomno = msg->readInt32();
            RoomInfo* ri = findRoomNo(roomno);
            if (ri) {
                ri->updateRoomInfo(msg, proto);
            } else {
                ri = new RoomInfo(roomno, msg, proto);
                rooms.insert(roomno, ri);
            }
            emit roomUpdated(roomno);
        } break;

        case Room_message:
        {
            int roomno = msg->readInt32();
            RoomInfo* ri = findRoomNo(roomno);
            if (ri) {
                RoomMessage* rm = new RoomMessage(msg, proto);
                ri->addMessage(rm);
                emit roomMessage(roomno, rm);
            }
            else qDebug() << QString("Room_message for not existing room %1").arg(roomno);
        } break;

        case Room_add_user:
        {
            int roomnum = msg->readInt32();
            int usernum = msg->readInt32();
            emit roomAddUser(roomnum, usernum);
            qDebug() << "Room_add_user roomnum=" << roomnum << " usernum=" << usernum;
        } break;

        case Room_remove_user:
        {
            int roomnum = msg->readInt32();
            int usernum = msg->readInt32();
            emit roomRemoveUser(roomnum, usernum);
            qDebug() << "Room_remove_user roomnum=" << roomnum << " usernum=" << usernum;
        } break;

        case UploadFiles:
        {
            for (QMap<int,bool>::Iterator it = upload.begin(); it != upload.end(); ++it)
                it.value() = false; // We use the bool-value to indicate if an uploaditem gots dirty (removed).
            int j = msg->readInt16();
            for (int i=0; i<j; i++) {
                int num = msg->readInt32();
                upload.insert(num, true);
                refreshClientInfo(num);
            }
            for (QMap<int,bool>::Iterator it = upload.begin(); it != upload.end(); ++it)
                if (! it.value()) refreshClientInfo(it.key());
        } break;

        case Version:
        {
            QString version = msg->readString();
            emit coreVersion(version);
            qDebug() << "Core reported version " << version;
        } break;

        default:
            emit unhandledMessage(msg);
            break;
        }
        delete msg;
    }

    for(QMap<int, QList<ResultInfo*> >::ConstIterator it = searchesUpdated.constBegin(); it != searchesUpdated.constEnd(); ++it) {
        emit searchUpdated(it.key(), it.value());
    }
}

void DonkeyProtocol::updateDownloadFiles()
{
    m_socket->sendMessage(DonkeyMessage(GetDownloadFiles));
}

void DonkeyProtocol::updateDownloadedFiles()
{
    m_socket->sendMessage(DonkeyMessage(GetDownloadedFiles));
}

void DonkeyProtocol::updateConnectedServers()
{
    m_socket->sendMessage(DonkeyMessage(GetConnectedServers));
}

const FileInfoHash& DonkeyProtocol::downloadFiles()
{
    return download;
}

const FileInfoHash& DonkeyProtocol::downloadedFiles()
{
    return downloaded;
}

const ServerInfoHash& DonkeyProtocol::connectedServers()
{
    return servers;
}

const NetworkHash& DonkeyProtocol::availableNetworks()
{
    return networks;
}

const ClientInfoHash& DonkeyProtocol::clientList()
{
    return clients;
}

const ShareInfoHash& DonkeyProtocol::sharedFiles()
{
    return shares;
}

const QList<int>& DonkeyProtocol::friendList()
{
    return friends;
}

FileInfo* DonkeyProtocol::findDownloadFileNo(int fileno)
{
    return download.value(fileno);
}

FileInfo* DonkeyProtocol::findDownloadedFileNo(int fileno)
{
    return downloaded.value(fileno);
}

ServerInfo* DonkeyProtocol::findServerNo(int serverno)
{
    return servers.value(serverno);
}

Network* DonkeyProtocol::findNetworkNo(int nwno)
{
    return networks.value(nwno);
}


Network *DonkeyProtocol::findNetworkName(const QString &name)
{
    foreach (Network *net, networks) {
        if (net->networkName() == name) return net;
    }
    return 0;
}

ClientInfo* DonkeyProtocol::findClientNo(int clno)
{
    return clients.value(clno);
}

ShareInfo* DonkeyProtocol::findShareNo(int shno)
{
    return shares.value(shno);
}

void DonkeyProtocol::sendConsoleMessage(const QString& _msg, ConsoleCallbackInterface* callback)
{
    QString msg = _msg.trimmed();
    if (callback) {
        delete consoleCallbacks.take(msg);
        consoleCallbacks.insert(msg, callback);
    }
    DonkeyMessage out(Command);
    out.writeString(msg);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::saveFile(int fileno, const QString& name)
{
    DonkeyMessage out(SaveFile);
    out.writeInt32(fileno);
    out.writeString(name);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::pauseFile(int fileno, bool pause)
{
    DonkeyMessage out(SwitchDownload);
    out.writeInt32(fileno);
    out.writeInt8((int8)!pause);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::cancelFile(int fileno)
{
    DonkeyMessage out(RemoveDownload_query);
    out.writeInt32(fileno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::setFilePriority(int fileno, int pri)
{
    DonkeyMessage out(SetFilePriority);
    out.writeInt32(fileno);
    out.writeInt32(pri);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::verifyFileChunks(int fileno)
{
    DonkeyMessage out(VerifyAllChunks);
    out.writeInt32(fileno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::getFileFormat(int fileno)
{
    DonkeyMessage out(Query_Format);
    out.writeInt32(fileno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::previewFile(int fileno)
{
    DonkeyMessage out(Preview);
    out.writeInt32(fileno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::retryFile(int fileno)
{
    DonkeyMessage out(ConnectAll);
    out.writeInt32(fileno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::submitUrl(const QString& url)
{
    downloadstarted = true;
    QRegExp rx("^(ftp|http)://.+");
    if (rx.indexIn(url) >= 0 && ! url.toLower().endsWith(".torrent")) {
        sendConsoleMessage("http \"" + url + "\"");
    }
    else {
        DonkeyMessage out(Url);
        out.writeString(url);
        m_socket->sendMessage(out);
    }
}

void DonkeyProtocol::submitUrl(const QUrl &url)
{
    downloadstarted = true;//?
    const QString scheme = url.scheme().toLower();
    const QString fileName = url.path();
    const bool isTorrent = fileName.endsWith(".torrent",Qt::CaseInsensitive);
    if ( (scheme.isEmpty() || scheme == "file") && isTorrent ) {
        //local torrent
        const QHostAddress localAddress = m_socket->localAddress();
        const QHostAddress donkeyAddress = QHostAddress(getHost()->address());
        //XXX: may be improved by using one instance of TorrentHost 
        //and make distinction inside instead of using instance per file
        TorrentHost * torrentSender = new TorrentHost(fileName, localAddress, donkeyAddress);
        qDebug() << "submitting file: " << fileName
            << "local address: " << localAddress
            << "donkey host: " << donkeyAddress;

        QUrl httpUrl;
        httpUrl.setScheme("http");
        httpUrl.setHost(localAddress.toString());
        httpUrl.setPort(torrentSender->serverPort());
        httpUrl.setPath(QFileInfo(fileName).fileName());
        //resubmit
        submitUrl(httpUrl);
    } else if ( (scheme == "http" || scheme == "https" || scheme == "ftp") && !isTorrent) {
        //web file
        sendConsoleMessage("http \"" + url.toString() + "\"");
    }
    else {
        //other
        DonkeyMessage out(Url);
        out.writeString(url.toString());
        m_socket->sendMessage(out);
    }
}


void DonkeyProtocol::sendMessage(const DonkeyMessage& msg)
{
    m_socket->sendMessage(msg);
}

void DonkeyProtocol::connectMoreServers()
{
    m_socket->sendMessage(DonkeyMessage(ConnectMore_query));
}

void DonkeyProtocol::cleanOldServers()
{
    m_socket->sendMessage(DonkeyMessage(CleanOldServers));
}

void DonkeyProtocol::addServer(int network, const QString& ip, int16 port)
{
    DonkeyMessage out(AddServer_query);
    out.writeInt32(network);
    out.writeIPAddress(QHostAddress(ip));
    out.writeInt16(port);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::removeServer(int serverno)
{
    DonkeyMessage out(RemoveServer_query);
    out.writeInt32(serverno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::blacklistServer(int serverno)
{
    ServerInfo* si = findServerNo(serverno);
    if (si) sendConsoleMessage("bs " + si->serverAddress());
}

void DonkeyProtocol::getServerInfo(int serverno)
{
    DonkeyMessage out(GetServer_info);
    out.writeInt32(serverno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::getServerUsers(int serverno)
{
    DonkeyMessage out(GetServer_users);
    out.writeInt32(serverno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::connectServer(int serverno)
{
    DonkeyMessage out(ConnectServer);
    out.writeInt32(serverno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::disconnectServer(int serverno)
{
    DonkeyMessage out(DisconnectServer);
    out.writeInt32(serverno);
    m_socket->sendMessage(out);
}

SearchInfo* DonkeyProtocol::findSearchNo(int num)
{
    return searches.value(num);
}

SearchInfo* DonkeyProtocol::findUnmappedSearchNo(int num)
{
    return unmappedSearches.value(num);
}

const SearchInfoHash& DonkeyProtocol::activeSearches() const
{
    return searches;
}

const QString& DonkeyProtocol::definedSearch() const
{
    return defSearch;
}

const QMap<QString, SearchQuery*>& DonkeyProtocol::definedSearches() const
{
    return defSearches;
}

void DonkeyProtocol::startSearch(int searchNum,
                                 SearchQuery* query,
                                 int maxHits,
                                 SearchType searchType,
                                 int network)
{
    delete unmappedSearches.take(searchNum);
    SearchQuery *sq = query->cloneSearchQuery();
    SearchInfo *si = new SearchInfo(searchNum, sq, maxHits, searchType, network);
    sq->setParent(si);
    unmappedSearches.insert(searchNum, si);

    DonkeyMessage out(Search_query);
    out.writeInt32((int32)searchNum); // search_num to identify searches
    query->writeQuery(out); // write the query into the message
    out.writeInt32((int32)maxHits); // search_max_hits
    out.writeInt8((int8)searchType); // search_type
    out.writeInt32((int32)network); // network to search
    m_socket->sendMessage(out);
}

class StopSearchCallback : public ConsoleCallbackInterface
{
public:
    StopSearchCallback(DonkeyProtocol *protocol, int searchNum) : ConsoleCallbackInterface(), m_protocol(protocol), m_searchNum(searchNum) {}
    virtual void callback(const QString &cmd, const QString &res) {
        qDebug() << "StopSearchCallback cmd=" << cmd << "res=" << res;
        if (res.isEmpty()) // we assume that not empty means an error-message...
            m_protocol->removeSearch(m_searchNum);
    }
private:
    DonkeyProtocol *m_protocol;
    int m_searchNum;
};

void DonkeyProtocol::stopSearch(int searchNum)
{
    /* Seems MLnet is buggy and does not proper close/forget the search using the GUI-protocol :-/
    DonkeyMessage out(CloseSearch);
    //DonkeyMessage out(ForgetSearch);
    out.writeInt32(searchNum);
    out.writeInt8(true); // close and forget
    //out.writeBool(true);
    m_socket->sendMessage(out);
    */

    QString cmd = QString("forget %1").arg(searchNum);
    sendConsoleMessage(cmd, new StopSearchCallback(this, searchNum));
}

void DonkeyProtocol::removeSearch(int searchNum)
{
    SearchInfo *si = searches.take(searchNum);
    if (si) {
        delete unmappedSearches.take(searchNum);
        unmappedSearches[searchNum] = si;
        emit searchRequest(searchNum);
    }
}

void DonkeyProtocol::getPending()
{
    DonkeyMessage out(GetPending);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::getSearches()
{
    DonkeyMessage out(GetSearches);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::getSearch(int search)
{
    DonkeyMessage out(GetSearch);
    out.writeInt32(search);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::extendedSearch()
{
    DonkeyMessage out(ExtendedSearch);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::startDownload(const QStringList &names, int num, bool force)
{
    downloadstarted = true;

    DonkeyMessage out(Download_query);

    out.writeInt16(names.count());
    for (int i = 0; i < (int)names.count(); i++) out.writeString(names[i]);

    out.writeInt32(num);
    out.writeInt8(force);

    m_socket->sendMessage(out);
}

void DonkeyProtocol::refreshShared()
{
    DonkeyMessage out(RefreshUploadStats);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::refreshFileInfo(int fileno)
{
    DonkeyMessage out(GetFile_info);
    out.writeInt32(fileno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::refreshClientInfo(int clientno)
{
    DonkeyMessage out(GetClient_info);
    out.writeInt32(clientno);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::getFileLocations(int fileno)
{
    DonkeyMessage out(GetFile_locations);
    out.writeInt32(fileno);
    m_socket->sendMessage(out);
}

uint DonkeyProtocol::connectedServerCount()
{
    return connectedservers;
}

uint DonkeyProtocol::totalServerCount()
{
    return servers.count();
}

void DonkeyProtocol::searchForFriend(const QString& name)
{
    DonkeyMessage out(FindFriend);
    out.writeString(name);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::addClientFriend(int client)
{
    DonkeyMessage out(AddClientFriend);
    out.writeInt32(client);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::removeFriend(int client)
{
    DonkeyMessage out(RemoveFriend);
    out.writeInt32(client);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::removeAllFriends()
{
    m_socket->sendMessage(DonkeyMessage(RemoveAllFriends));
}

void DonkeyProtocol::connectFriend(int client)
{
    DonkeyMessage out(ConnectFriend);
    out.writeInt32(client);
    m_socket->sendMessage(out);
}

const ResultInfo* DonkeyProtocol::findClientFile(int fileno)
{
    return unmappedResults.value(fileno);
}

const QMap<QString, QString>& DonkeyProtocol::optionsList()
{
    return options;
}

QString DonkeyProtocol::getOption(const QString& option)
{
    return options.contains(option) ? options[option] : QString();
}

void DonkeyProtocol::setOption(const QString& option, const QString& value)
{
    DonkeyMessage out(SetOption);
    out.writeString(option);
    out.writeString(value);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::setOptions(const QMap<QString, QString>& opts)
{
    if (opts.count() < 1) return;
    DonkeyMessage out(SaveOptions_query);
    out.writeInt16( (int16) opts.count() );
    for (QMap<QString, QString>::ConstIterator it = opts.begin(); it != opts.end(); ++it) {
        out.writeString( it.key() );
        out.writeString( it.value() );
    }
    m_socket->sendMessage(out);
}

void DonkeyProtocol::enableNetwork(int nwno, bool enable)
{
    Network* nw = findNetworkNo(nwno);
    if (! nw) return;
    DonkeyMessage out(EnableNetwork);
    out.writeInt32( (int32) nw->networkNo() );
    out.writeInt8( (int8) (enable ? 1 : 0) );
    m_socket->sendMessage(out);
}

const QList<DonkeyOption>& DonkeyProtocol::sectionOptions()
{
    return m_sectionOptions;
}

const QList<DonkeyOption>& DonkeyProtocol::pluginOptions()
{
    return m_pluginOptions;
}

void DonkeyProtocol::killCore()
{
    m_socket->sendMessage(DonkeyMessage(KillServer));
}

void DonkeyProtocol::sendPrivateMessage(int client, const QString& message)
{
    DonkeyMessage out(MessageToClient);
    out.writeInt32(client);
    out.writeString(message);
    m_socket->sendMessage(out);
}

const int& DonkeyProtocol::coreProtocolVersion()
{
    return coreProto;
}

RoomInfo* DonkeyProtocol::findRoomNo(int roomno)
{
    return rooms.value(roomno);
}

void DonkeyProtocol::setRoomState(int roomno, RoomInfo::RoomState state)
{
    qDebug() << "DonkeyProtocol::setRoomState() roomno=" << roomno << " state=" << state;
    DonkeyMessage out(SetRoomState);
    out.writeInt32(roomno);
    out.writeInt32(state);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::renameFile(int fileno, const QString& name)
{
    DonkeyMessage out(RenameFile);
    out.writeInt32(fileno);
    out.writeString(name);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::updateUploaders()
{
    DonkeyMessage out(GetUploaders);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::connectClient(int client)
{
    DonkeyMessage out(ConnectClient);
    out.writeInt32(client);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::disconnectClient(int client)
{
    DonkeyMessage out(DisconnectClient);
    out.writeInt32(client);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::sendNetworkMessage(int network, const QString& msg)
{
    DonkeyMessage out(NetworkMessage);
    out.writeInt32(network);
    out.writeString(msg);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::interestedInSources(bool interested)
{
    DonkeyMessage out(InterestedInSources);
    out.writeBool(interested);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::requestVersion()
{
    DonkeyMessage out(GetVersion);
    m_socket->sendMessage(out);
}

QAbstractSocket* DonkeyProtocol::socket() const
{
    return m_socket;
}

void DonkeyProtocol::renameServer(int id, const QString& name)
{
    DonkeyMessage out(ServerRename);
    out.writeInt32(id);
    out.writeString(name);
    m_socket->sendMessage(out);
}

void DonkeyProtocol::setServerPreferred(int id, bool preferred)
{
    DonkeyMessage out(ServerSetPreferred);
    out.writeInt32(id);
    out.writeBool(preferred);
    m_socket->sendMessage(out);
}
