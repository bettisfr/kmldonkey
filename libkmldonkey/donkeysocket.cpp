/* -*- c++ -*-
 *
 * donkeysocket.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
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

#include "donkeysocket.h"

#include <QtEndian>

#include "donkeymessage.h"

static const qint16 NullOpCode = -1;

DonkeySocket::DonkeySocket(QObject *parent)
    : QTcpSocket(parent)
    , mlHost(QString())
    , mlPort(4001)
    , m_opcode(NullOpCode)
    , m_size(0)
{
    connect(this, SIGNAL(readyRead()), SLOT(readMessage()));
}

DonkeySocket::~DonkeySocket()
{
    qDeleteAll(fifo);
}

void DonkeySocket::connectDonkey()
{
    qDebug() << "Connecting to " << mlHost << ":" << mlPort;
    connectToHost(mlHost, mlPort);
}

void DonkeySocket::connectDonkey(const QString& host, quint16 port)
{
    mlHost = host;
    mlPort = port;
    connectDonkey();
}

bool DonkeySocket::sendMessage(const DonkeyMessage& msg)
{
    if (state() != ConnectedState) return false;
    unsigned char op[4];
    qToLittleEndian<qint32>(msg.size() , op);
    qint64 r = write((char *)op, 4);
    if (r != 4) {
        qDebug()<<"DonkeySocket::sendMessage (r!=4) opcode="<<msg.opcode()<<"r="<<r;
        return false;
    }
    qToLittleEndian<qint16>(msg.opcode(), op);
    r = write((char *)op, 2);
    if (r != 2) {
        qDebug()<<"DonkeySocket::sendMessage (r!=2) opcode="<<msg.opcode()<<"r="<<r;
        return false;
    }
    r = write(msg.data());
    bool result = (r == msg.data().size());
    //qDebug()<<"DonkeySocket::sendMessage opcode="<<msg.opcode()<<"r="<<r<<"msg.size="<<msg.size()<<"result="<<result;
    return result;
}

void DonkeySocket::readMessage()
{
    char header[6];
    while (true) {
        if (m_opcode == NullOpCode) {//try to read header
            if (bytesAvailable() < (qint64)sizeof(header)) {
                break;//not enough to form new header
            }
            qint64 r = read(header,sizeof(header));
            if (r != sizeof(header)) {
                qDebug() << "Error reading header";
                abort();
                break;
            }
            m_size = qFromLittleEndian<qint32>((uchar *)header) - sizeof(m_opcode);
            m_opcode = qFromLittleEndian<qint16>((uchar *)( header + sizeof(m_size) ));
            if (m_opcode == NullOpCode || m_size < 0) {
                qDebug() << "Malformed packet. size: " << m_size << "opcode: " << m_opcode;
                abort();
                break;
            }
        }
        if (bytesAvailable() < m_size) {
            break; //not enough to read message body
        }
        QByteArray data = read(m_size);
        if (data.size() != m_size) return;//err
        fifo.enqueue(new DonkeyMessage(m_opcode, data));
        m_opcode = NullOpCode;
    }

    if (!fifo.isEmpty())
        emit readyMessage();
}

uint DonkeySocket::queueSize()
{
    return fifo.count();
}

DonkeyMessage* DonkeySocket::popMessage()
{
    return fifo.isEmpty() ? 0 : fifo.dequeue();
}
