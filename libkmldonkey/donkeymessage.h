/* -*- c++ -*-
 *
 * donkeymessage.h
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
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

#ifndef __libkmldonkey_donkeymessage_h__
#define __libkmldonkey_donkeymessage_h__

#include <QByteArray>
#include <QVariant>

#include <kmldonkey_export.h>


class QHostAddress;
class QString;
class QTextCodec;


//! A core protocol message.
/*!
 * This class represents a message of the kind the core and client
 * use to communicate with.
 */

class KMLDONKEY_EXPORT DonkeyMessage
{
public:
    //! Construct an empty message with the given opcode.
    /*! \param opcode The message's opcode.
     *  \param len The message size. */
    DonkeyMessage(int opcode, int len = 0);
    /**
     * Construct message with given opcode and data
     * \param opcode The message's opcode.
     * \param data The message data payload.
     */
    DonkeyMessage(qint16 opcode, const QByteArray &data);
    //! Set the opcode of the message.
    /*! \param opcode The new opcode. */
    void setOpcode(int opcode);
    //! Get the opcode of the message.
    /*! \return The opcode. */
    int opcode() const;
    inline qint32 size() const { return m_data.size() + sizeof(m_opcode); }
    inline const QByteArray &data() const { return m_data; }

    //! Append a byte to the message.
    /*! \param v A byte. */
    void writeInt8(qint8 v);
    //! Append a 16-bit word to the message.
    /*! \param v A 16-bit word. */
    void writeInt16(qint16 v);
    //! Append a 32-bit word to the message.
    /*! \param v A 32-bit word. */
    void writeInt32(qint32 v);
    //! Append a 64-bit word to the message.
    /*! \param v A 64-bit word. */
    void writeInt64(qint64 v);
    //! Append a boolean value to the message.
    /*! \param v A boolean value. */
    void writeBool(bool v);
    //! Append a string to the message.
    /*! \param v A string. */
    void writeString(const QString& v);
    //! Append a string to the message.
    /*! \param v A null terminated string buffer. */
    void writeString(const char* v);
    //! Append a byte array to the message.
    /*! \param v A byte array. */
    void writeByteArray(const QByteArray& v);
    //! Append a floating point value to the message.
    /*! \param v A floating point value. */
    void writeFloat(double v);

    //! Return the position which represents the start of the message.
    int position();
    //! Reset the read position to the start of the message.
    void resetPosition();
    //! Set the read position to pos.
    void setPosition(int pos);

    //! Read a byte from the message.
    /*! \return A byte. */
    qint8 readInt8();
    //! Read a 16-bit word from the message.
    /*! \return A 16-bit word. */
    qint16 readInt16();
    //! Read a 32-bit word from the message.
    /*! \return A 32-bit word. */
    qint32 readInt32();
    //! Read a 64-bit word from the message.
    /*! \return A 64-bit word. */
    qint64 readInt64();
    //! Read a boolean value from the message.
    /*! \return A boolean value. */
    bool readBool();
    //! Read a string from the message.
    /*! \param ok If ok is not 0: if a "buffer exceeds size"-error occurs, *ok is
     *            set to FALSE; otherwise *ok is set to TRUE.
     *  \return A string. */
    QString readString(bool* ok = 0);
    /** Read a string list from the message.
     * \param ok If ok is not 0: if a "buffer exceeds size"-error occurs, *ok is
     *            set to FALSE; otherwise *ok is set to TRUE.
     * \return A string list.
     */
    QStringList readStringList(bool *ok = 0);
    //! Read a byte array from the message.
    /*! This is essentially the same as reading a string, but it's not processed
     *  by the charset decoder.
     *  \param ok If ok is not 0: if a "buffer exceeds size"-error occurs, *ok is
     *            set to FALSE; otherwise *ok is set to TRUE.
     *  \return A byte array. */
    QByteArray readByteArray(bool* ok = 0);
    /** Read a md4 hash from the message.
     * same as reading byte array, but no size field
     */
    QByteArray readMd4(bool *ok = 0);
    //! Read a time_t-value.
    /*! This is similar to readInt32() except that it does substract
     *  the actual time_t as defined with protocol >= 24 to get
     *  client-relative time-values. */
    int readDate();
    //! Read a floating point value from the message.
    /*! \return A floating point value. */
    double readFloat();
    //! Read an IP address from the message.
    /*! \return A string describing an IP address. */
    QString readIPAddress();
    /**
     * Writes an IP address to the message in little endian order
     */
    void writeIPAddress(const QHostAddress &addr);
    //! Read an encoded IP address from the message.
    /*! \return A string describing an IP address. */
    QString readAddress();
    //! Read an encoded tag and insert it into the given dictionary.
    /*! \param dict A reference to the dictionary to operate on.
     *  \return True on success, false if there was a syntax error.
     */
    bool readTag(QVariantMap& dict);

    //! Construct a string containing a hex dump of the message.
    QString dumpArray() const;

    //! Explicitly specify a QTextCodec to use in decoding strings from messages.
    /*! DonkeyMessage defaults to using an ISO-8859-1 codec when translating
     *  strings from the core into internal Unicode representations. If you are
     *  expecting a different charset, you can call this static method to specify
     *  the codec to be used.
     *  \param codec The QTextCodec object to be used in string decoding.
     */
    static void setStringCodec(QTextCodec* codec);


private:
    int m_pos;
    qint16 m_opcode;
    QByteArray m_data;

    void initCodec();

    static QTextCodec* codec;

    void init(qint16 opcode, const QByteArray &data);

    template <typename T> friend T help_readInt(DonkeyMessage *msg);
    template <typename T> friend void help_writeInt(DonkeyMessage *msg, T value);
    friend QByteArray help_readArray(DonkeyMessage *msg, int size, bool *ok);
};

#endif
