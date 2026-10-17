/* -*- c++ -*-
 *
 * donkeysocket.h
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

#ifndef __libkmldonkey_donkeysocket_h__
#define __libkmldonkey_donkeysocket_h__

#include <QTcpSocket>
#include <QQueue>


class DonkeyMessage;

//! A socket object with a DonkeyMessage queue.
/*!
 * This object is the low-level communications layer that deals
 * directly with the core protocol. It works like a QSocket, except
 * it sends and receives DonkeyMessage objects. It has an internal
 * queue of received messages, and emits the readyMessage() signal
 * when there are messages in the queue.
 */

class DonkeySocket : public QTcpSocket
{
    Q_OBJECT

public:
    //! Constructor.
    /*! \param parent The parent object.
     *  \param name The object name. */
    DonkeySocket( QObject *parent = 0 );
    //! Destructor.
    ~DonkeySocket();

    //! Send a message to the core.
    /*! \param msg The DonkeyMessage object containing the message to be sent.
     *  \return True if the message was successfully dispatched.
     */
    bool sendMessage(const DonkeyMessage& msg);
    //! Return the size of the message queue.
    /*! \return The number of messages waiting in the queue. */
    uint queueSize();
    //! Pop a message off the top of the queue.
    /*! \return Pointer to a message, or NULL. Once you have popped a message
     *    off the queue, it is your responsibility. You should delete it when
     *    you are done with it. */
    DonkeyMessage* popMessage();

public slots:
    //! Reconnect to the core.
    /*!
     * Initiate a connection to the core, using the address previously set.
     * \sa connectDonkey(const QString&, quint16)
     */
    void connectDonkey();
    //! Connect to the core.
    /*!
     * Initiate a connection to the core at the provided address.
     * \param host The host address.
     * \param port The number of the core's GUI port.
     */
    void connectDonkey(const QString& host, quint16 port);

protected slots:
    //! Construct messages from the socket and put them on the queue.
    void readMessage();

signals:
    //! Emitted when messages are ready on the queue.
    /*! When this signal is received, you should process all messages on
     * the queue (call popMessage() until it returns NULL). It is not
     * necessarily emitted for every message that arrives.
     * \sa popMessage()
     */
    void readyMessage();

private:
    QString mlHost;
    quint16 mlPort;
    qint16 m_opcode;
    qint32 m_size;
    QQueue<DonkeyMessage*> fifo;
    DonkeyMessage *cur;
};

#endif
