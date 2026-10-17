/* -*- c++ -*-
 *
 * torrenthost.cpp
 *
 * Copyright (C) 2009 Gioacchino Mazzurco <gmazzurco89@gmail.com>
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

#include "torrenthost.h"

#include <QRegExp>
#include <QStringList>
#include <QHostAddress>
#include <QIODevice>
#include <QtDebug>

TorrentHost::TorrentHost(const QString &filePath, const QHostAddress &local, const QHostAddress &donkey) 
    : QTcpServer()
    , m_donkey(donkey) 
{
  qDebug() << "TorrentHost::TorrentHost(QString filePath) called \n";
  torrentFile.setFileName(filePath);
  connect(this, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
  listen(local);
  qDebug() << "TorrentHost listening on port " << serverPort();
}

TorrentHost::~TorrentHost(){}


void TorrentHost::acceptConnection(){
  qDebug() << "TorrentHost::acceptConnection() called";

  httpConnection = nextPendingConnection();
    if (!m_donkey.isNull() && httpConnection->peerAddress() != m_donkey) {
        qDebug() << "Incorrect host connected: expecting" << m_donkey << "got: " << httpConnection->peerAddress();
        httpConnection->deleteLater();
        return;
    }
  connect(httpConnection, SIGNAL(readyRead()), this, SLOT(dataAvailable()));
  
}

void TorrentHost::dataAvailable()
{
    qDebug() << "got data";
    while (httpConnection->canReadLine()) {
        qDebug() << "can read line";
        const QByteArray rline = httpConnection->readLine();
        qDebug() << "line : "<< rline;
        if (rline.trimmed().isEmpty()) {
            if (!m_currentRequest.isEmpty())
                processRequest();
        } else {
            m_currentRequest << rline;
        }
    }
}

void TorrentHost::processRequest()
{
    qDebug() << "processing request" << m_currentRequest;
    bool head = m_currentRequest.at(0).startsWith("HEAD");
    bool get = !head && m_currentRequest.at(0).startsWith("GET");
    Q_ASSERT(head || get);
    QList<QByteArray> response;
    response << "HTTP/1.0 200 Ok"
        << "Content-Type: application/x-bittorrent"
        << "Server: libkmldonkey/svn";
    if (get) 
        response << QByteArray("Content-length: ") + QByteArray::number(torrentFile.size());
    response << "";
    if (get) {
        if (!torrentFile.open(QIODevice::ReadOnly)) 
            Q_ASSERT(false);//form 404 response
        QByteArray torrent = torrentFile.readAll();
        response << torrent;
    }
    foreach (const QByteArray &line, response) {
        qDebug() << "response : " << line;
        httpConnection->write(line);
        httpConnection->write("\r\n");
    }
    if (get) {
        httpConnection->flush();
        this->deleteLater();
    }
    m_currentRequest.clear();
}
