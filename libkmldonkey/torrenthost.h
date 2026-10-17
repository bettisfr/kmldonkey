/* -*- c++ -*-
 *
 * torrenthost.h
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


#ifndef __libkmldonkey_torrenthost_h__
#define __libkmldonkey_torrenthost_h__

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QFile>

#include "kmldonkey_export.h"

class KMLDONKEY_EXPORT TorrentHost : public QTcpServer{

  Q_OBJECT

  public:
    TorrentHost(const QString &filePath, const QHostAddress &local, const QHostAddress &donkey);
    ~TorrentHost();

  private:
    QFile torrentFile;
    QTcpSocket * httpConnection;
    QHostAddress m_donkey;
    QList<QByteArray> m_currentRequest;
  
    void processRequest();

  private slots:
    void acceptConnection();
    void dataAvailable();

};

#endif /* __libkmldonkey_torrenthost_h__ */
