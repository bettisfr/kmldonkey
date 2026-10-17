/* -*- c++ -*-
 *
 * donkeymessage.cpp
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

#include "donkeymessage.h"

#include <QHostAddress>
#include <QTextCodec>
#include <QtEndian>
#include <QStringList>

#include <time.h>

QTextCodec* DonkeyMessage::codec = 0;

template <typename T>
T help_readInt(DonkeyMessage *msg)
{
    Q_ASSERT(msg->m_pos + sizeof(T) <= (uint)msg->m_data.size());
    T result = qFromLittleEndian<T>((const uchar*)( msg->m_data.constData() + msg->m_pos ));
    msg->m_pos += sizeof(T);
    return result;
}

template <typename T>
void help_writeInt(DonkeyMessage *msg, T value)
{
    QByteArray &data = msg->m_data;
    int pos = data.size();
    data.resize(pos + sizeof(T));
    qToLittleEndian(value, (uchar*)(data.data() + pos));
    msg->m_pos = data.size();
}

void DonkeyMessage::initCodec()
{
    if (!codec) {
        codec = QTextCodec::codecForName("ISO-8859-1");
        if (!codec) {
            codec = QTextCodec::codecForLocale();
        }
        qDebug()<<"Using codec:"<<(codec ? codec->name() : "NULL");
    }
}

void DonkeyMessage::setStringCodec(QTextCodec* newCodec)
{
    codec = newCodec;
    qDebug()<<"Using codec:"<<(codec ? codec->name() : "NULL");
}

void DonkeyMessage::init(qint16 opcode, const QByteArray &data)
{
    m_data = data;
    m_opcode = opcode;
    m_pos = 0;
    initCodec();
}

DonkeyMessage::DonkeyMessage(int opcode, int len)
{
    QByteArray d = QByteArray();
    d.resize(len);
    init(opcode, d);
}

DonkeyMessage::DonkeyMessage(qint16 opcode, const QByteArray &data)
{
    init(opcode, data);
}

int DonkeyMessage::opcode() const
{
    return m_opcode;
}

void DonkeyMessage::setOpcode(int opcode)
{
    m_opcode = opcode;
}

void DonkeyMessage::writeInt8(qint8 v)
{
    m_data.append(v);
    ++m_pos;
}

void DonkeyMessage::writeInt16(qint16 v)
{
    help_writeInt(this, v);
}

void DonkeyMessage::writeInt32(qint32 v)
{
    help_writeInt(this, v);
}

void DonkeyMessage::writeInt64(qint64 v)
{
    help_writeInt(this, v);
}

void DonkeyMessage::writeBool(bool v)
{
    writeInt8(v ? 1 : 0);
}

void DonkeyMessage::writeString(const QString& v)
{
    QByteArray s = codec->fromUnicode(v);
    if (s.isNull()) {
        qDebug() << "Unable to convert string into charset " << codec->name() << ".";
        writeString("");
    } else
        writeString(s.constData());
}

void DonkeyMessage::writeString(const char* v)
{
    int i,l = qstrlen(v);
    m_pos = m_data.size();
    if (l >= 0xffff) {
        writeInt16(0xffff);
        writeInt32(l);
    } else writeInt16(l);
    m_data.resize(m_pos + l);
    for (i=0; i<l; i++)
        m_data[m_pos++] = v[i];
}

void DonkeyMessage::writeByteArray(const QByteArray& v)
{
    int i,l = (int)v.size();
    if (l >= 0xffff) {
        writeInt16(0xffff);
        writeInt32(l);
    } else writeInt16(l);
    m_data.resize(m_pos + l);
    for (i=0; i<l; i++)
        m_data[m_pos++] = v[i];
}

void DonkeyMessage::writeFloat(double v)
{
    QString foo = QString();
    foo = QString::asprintf("%.4f", v);
    writeString(foo);
}

int DonkeyMessage::position()
{
    return m_pos;
}

void DonkeyMessage::resetPosition()
{
    m_pos = 0;
}

void DonkeyMessage::setPosition(int pos)
{
    m_pos = pos;
}

qint8 DonkeyMessage::readInt8()
{
    return m_data.at(m_pos++);
}

qint16 DonkeyMessage::readInt16()
{
    return help_readInt<qint16>(this);
}

qint32 DonkeyMessage::readInt32()
{
    return help_readInt<qint32>(this);
}

qint64 DonkeyMessage::readInt64()
{
    return help_readInt<qint64>(this);
}

bool DonkeyMessage::readBool()
{
    return (bool)readInt8();
}

QString DonkeyMessage::readString(bool* ok)
{
    return codec->toUnicode( readByteArray(ok) );
}


QStringList DonkeyMessage::readStringList(bool *ok)
{
    int size = readInt16();
    QStringList result;
    for (int i = 0; i < size; ++i) {
        result << readString(ok);
    }
    return result;
}

QByteArray help_readArray(DonkeyMessage *msg, int size, bool *ok)
{

    if (msg->m_data.size() < msg->m_pos + size) {
        qDebug() << "Position " << msg->m_pos + size
            << "exceeds buffer size " << msg->m_data.size()
            << "\nMessage: " << msg->dumpArray();
        if (ok) {
            *ok = false;
        } else {
            qDebug() << "Invalid DonkeyMessage index access.";
        }
        return QByteArray();
    }

    QByteArray result = msg->m_data.mid(msg->m_pos,size);
    msg->m_pos += size;
    return result;
}

QByteArray DonkeyMessage::readByteArray(bool* ok)
{
    int sz = (int)readInt16();
    if (sz == 0xffff) sz = (int)readInt32();

    return help_readArray(this, sz, ok);
}

QByteArray DonkeyMessage::readMd4(bool *ok)
{
    return help_readArray(this, 16, ok);
}

int DonkeyMessage::readDate()
{
    return time(0) - readInt32();
}

double DonkeyMessage::readFloat()
{
    return readByteArray().toDouble();
}

QString DonkeyMessage::readIPAddress()
{
    return QHostAddress(qToBigEndian<qint32>(readInt32())).toString();
}


void DonkeyMessage::writeIPAddress(const QHostAddress &addr)
{
    qint32 ipv4 = qFromBigEndian(addr.toIPv4Address());
    writeInt32(ipv4);
}

QString DonkeyMessage::readAddress()
{
    if (readInt8())
        return readString();
    else
        return readIPAddress();
}

bool DonkeyMessage::readTag(QVariantMap &dict)
{
    bool ok = true;
    QString name = readString(&ok);
    if (! ok)
        return false;
    QVariant value;
    switch (readInt8()) {
        case 0:
        case 1:
            value = QVariant(readInt32());
            break;
        case 2: {
            bool is_ok = true;
            value = QVariant(readString(&is_ok));
            if(! is_ok) return false;
        } break;
        case 3:
            value = QVariant(readIPAddress());
            break;
        case 4:
            value = QVariant(readInt16());
            break;
        case 5:
            value = QVariant(readInt8());
            break;
        default:
            qDebug() << "DonkeyMessage::readTag() returned unknown value!";
            return false;
    }
    dict.insert(name, value);
    return true;
}

QString DonkeyMessage::dumpArray() const
{
    QString out = QString::fromLatin1("Opcode %1, size %2\n").arg(m_opcode).arg(m_data.size());

    int i;
    QString hex, asc, tmp;
    for (i=0; i < m_data.size(); i++) {
        uchar ch = (uchar)m_data.at(i);
        asc += QLatin1Char((ch >= 32 && ch <= 127) ? ch : '.');
        tmp = QString::asprintf("%02x", ch);
        hex += tmp + QLatin1Char(' ');
        if (i % 16 == 15) {
            tmp = QString::asprintf("%8d: ", i - 15);
            out += tmp + hex + QLatin1String("  ") + asc + QLatin1Char('\n');
            hex.clear();
            asc.clear();
        }
    }
    tmp = QString::asprintf("%8d: ", i - (i % 16));
    for (i %= 16; i < 16; i++)
        hex += QLatin1String("   ");
    out += tmp + hex + QLatin1String("  ") + asc + QLatin1Char('\n');
    return out;
}
