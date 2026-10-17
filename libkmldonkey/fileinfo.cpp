#include <QLocale>
/* -*- c++ -*-
 *
 * fileinfo.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2010 Gioacchino Mazzurco <gmazzurco89@gmail.com>
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


#include <KLocalizedString>
#include <QRegularExpression>

#ifndef LIBKMLDONKEY_NO_KDE
    #include <KSharedConfig>
#endif

#include "fileinfo.h"
#include "donkeymessage.h"


FileInfo::FileInfo(int num, DonkeyMessage* msg, int proto)
{
    this->num = num;
    updateFileInfo(msg, proto);

    firstdown = downloaded;
    firsttime = time(0);
}

FileInfo::~FileInfo()
{
}

void FileInfo::updateFileInfo(DonkeyMessage* msg, int proto)
{
    int i,j;
    network = msg->readInt32();
    names = msg->readStringList();
    QByteArray md4 = msg->readMd4();
    size = msg->readInt64();
    downloaded = msg->readInt64();
    nlocations = msg->readInt32();
    nclients = msg->readInt32();
    state = (State)msg->readInt8();
    if (state == 6)
        abortedmsg = msg->readString();

    chunks = msg->readByteArray();

    availability.clear();
    j = msg->readInt16();
    for (i=0; i<j; i++) {
        int av_net = msg->readInt32();
        availability.insert(av_net, msg->readByteArray());
    }

    speed = msg->readFloat();

    j = msg->readInt16();
    chunks_age.clear();
    for (i=0; i<j; i++) {
        time_t a = msg->readDate();
        if (a < 0) a = 0x7fffffff + a; // FIXME: Is this hack still relevant?
        chunks_age.append(a);
    }

    age = msg->readDate();
    if (age < 0) age = 0x7fffffff + age; // FIXME: Is this hack still relevant?

    format = msg->readInt8();
    switch (format) {
    case 1:
    {
        QString foo = msg->readString();
        QString bar = msg->readString();
        formatinfo = foo + " " + bar;
    } break;
    case 2:
    {
        QString foo = msg->readString();
        int i = msg->readInt32();
        int j = msg->readInt32();
        int k = msg->readInt32();
        int l = msg->readInt32();
        // xgettext: no-c-format
        formatinfo = ki18nc("AVI codec resolution framerate bitrate", "AVI %1 %2x%3 %4fps rate %5")
            .subs(foo).subs(i).subs(j).subs((double)k / 1000.0, 0, 'f', 2).subs(l).toString();
    } break;
    case 3:
    {
        QString foo = msg->readString();
        QString bar = msg->readString();
        QString baz = msg->readString();
        QString quux = msg->readString();
        msg->readString();
        int i = msg->readInt32();
        msg->readInt32();
        formatinfo = i18nc("MP3 title artist album year track", "MP3 Ti:%1 Ar:%2 Al:%3 Yr:%4 Tr:%5",
                           foo, bar, baz, quux, i);
    } break;
    case 4:
    {
        formatinfo = i18nc("Ogg format header", "Ogg");
        int i = msg->readInt16();
        for (; i; i--) {
            int streamNo = msg->readInt32();
            int foo = msg->readInt8();
            QString streamType;
            switch (foo) {
                case 0: streamType = i18nc("Ogg stream type: Video", "Video"); break;
                case 1: streamType = i18nc("Ogg stream type: Audio", "Audio"); break;
                case 2: streamType = i18nc("Ogg stream type: Text", "Text"); break;
                case 3: streamType = i18nc("Ogg stream type: Index", "Index"); break;
                case 4: streamType = i18nc("Ogg stream type: Vorbis", "Vorbis"); break;
                case 5: streamType = i18nc("Ogg stream type: Theora", "Theora"); break;
                default: streamType = i18nc("Ogg stream type: Unknown", "Unknown"); break;
            }
            int j = msg->readInt16();
            QStringList tags;
            for (; j; j--) {
                int type = msg->readInt8();
                switch (type) {
                    case 0: tags.append(i18nc("Ogg codec tag", "Codec:%1", msg->readString())); break;
                    case 1: tags.append(i18nc("Ogg bytes per sample tag", "BPS:%1", msg->readInt32())); break;
                    case 2: tags.append(i18nc("Ogg duration tag", "Dur:%1", msg->readInt32())); break;
                    case 3: tags.append(i18nc("Ogg subtitle tag", "Sub")); break;
                    case 4: tags.append(i18nc("Ogg index tag", "Index")); break;
                    case 5: tags.append(i18nc("Ogg audio channels tag", "AudCh:%1", msg->readInt32())); break;
                    case 6: tags.append(i18nc("Ogg audio sample rate tag", "SmpRt:%1", msg->readFloat())); break;
                    case 7: tags.append(i18nc("Ogg audio block align tag", "BlkAl:%1", msg->readInt32())); break;
                    case 8: tags.append(i18nc("Ogg audio average bytes per second tag", "AvgBPS:%1", msg->readFloat())); break;
                    case 9: tags.append(i18nc("Ogg Vorbis version tag", "VorbisVer:%1", msg->readFloat())); break;
                    case 10: tags.append(i18nc("Ogg Vorbis sample rate tag", "SmpRt:%1", msg->readFloat())); break;
                    case 11: {
                        QStringList bitRates;
                        int k = msg->readInt16();
                        for (; k; k--) {
                            int rateType = msg->readInt8();
                            switch (rateType) {
                                case 0: bitRates.append(i18nc("Ogg Vorbis maximum bitrate tag", "Max:%1", msg->readFloat())); break;
                                case 1: bitRates.append(i18nc("Ogg Vorbis nominal bitrate tag", "Nom:%1", msg->readFloat())); break;
                                case 2: bitRates.append(i18nc("Ogg Vorbis minimum bitrate tag", "Min:%1", msg->readFloat())); break;
                                default: break;
                            }
                        }
                        tags.append(i18nc("Ogg Vorbis bitrates tag", "BitRate:%1", bitRates.join(i18nc("Ogg Vorbis bitrate tag separator", ","))));
                    } break;
                    case 12: tags.append(i18nc("Ogg Vorbis block size 0 tag", "BlkSz0:%1", msg->readInt32())); break;
                    case 13: tags.append(i18nc("Ogg Vorbis block size 1 tag", "BlkSz1:%1", msg->readInt32())); break;
                    case 14: tags.append(i18nc("Ogg video width tag", "Wt:%1", msg->readFloat())); break;
                    case 15: tags.append(i18nc("Ogg video height tag", "Ht:%1", msg->readFloat())); break;
                    case 16: tags.append(i18nc("Ogg video sample rate tag", "SmpRt:%1", msg->readFloat())); break;
                    case 17: tags.append(i18nc("Ogg video aspect ratio tag", "Aspect:%1", msg->readFloat())); break;
                    case 18: {
                        QString cs;
                        switch (msg->readInt8()) {
                            case 1: cs = i18nc("Ogg Theora CS tag value: Rec470M", "Rec470M"); break;
                            case 2: cs = i18nc("Ogg Theora CS tag value: Rec470BG", "Rec470BG"); break;
                            default: cs = i18nc("Ogg Theora CS tag value: undefined", "Undef"); break;
                        }
                        tags.append(i18nc("Ogg Theora CS tag", "CS:%1", cs));
                    } break;
                    case 19: tags.append(i18nc("Ogg Theora quality tag", "Qual:%1", msg->readInt32())); break;
                    case 20: tags.append(i18nc("Ogg Theora average bytes per second tag", "AvgBPS:%1", msg->readInt32())); break;
                    default: break;
                }
            }
            formatinfo += i18nc("Ogg stream block", " %1:%2 [%3]", streamNo, streamType,
                                tags.join(i18nc("Ogg stream tag separator", " ")));
        }
    } break;
    default:
        formatinfo = i18n("Unknown format");
        break;
    }

    name = msg->readString();
    lastseen = msg->readInt32(); // FIXME: Is this relative?
    priority = msg->readInt32();
    if (priority > 0x40000000) priority -= 0x80000000;
    comment = msg->readString();
    uids = proto >= 31
        ? uids = msg->readStringList()
        : QStringList(QString("urn:ed2k:") + md4ToString(md4));
}

const int& FileInfo::fileNo() const
{
    return num;
}

const int& FileInfo::fileNetwork() const
{
    return network;
}

const QString& FileInfo::fileName() const
{
    return name;
}

const QStringList& FileInfo::fileNames() const
{
    return names;
}

const QStringList& FileInfo::fileUids() const
{
     return uids;
}

QString FileInfo::fileUid() const
{
    return uids.isEmpty() ? QString("NOT AVAILABLE") : uids.first();
}

QString FileInfo::fileUid(const QString& type) const
{
    QRegularExpression match(QString("^urn:") + type + ":");
    QStringList results = uids.filter(match);
    if (!results.count()) return QString();
    QString result(results.first());
    result.replace(match, "");
    return result;
}

const int64& FileInfo::fileSize() const
{
    return size;
}

const int64& FileInfo::fileDownloaded() const
{
    return downloaded;
}

const int64& FileInfo::fileFirstDownloaded() const
{
    return firstdown;
}

const int& FileInfo::fileNLocations() const
{
    return nlocations;
}

const int& FileInfo::fileNClients() const
{
    return nclients;
}

const FileInfo::State& FileInfo::fileState() const
{
    return state;
}

const QString& FileInfo::fileAbortedMsg() const
{
    return abortedmsg;
}

const QByteArray& FileInfo::fileChunks() const
{
    return chunks;
}

const QHash<int,QByteArray>& FileInfo::fileAvailability() const
{
    return availability;
}

const double& FileInfo::fileSpeed() const
{
    return speed;
}

const QList<time_t>& FileInfo::fileChunksAge() const
{
    return chunks_age;
}

const time_t& FileInfo::fileAge() const
{
    return age;
}

const time_t& FileInfo::fileFirstTime() const
{
    return firsttime;
}

const int& FileInfo::fileFormat() const
{
    return format;
}

const QString& FileInfo::fileFormatInfo() const
{
    return formatinfo;
}

const int& FileInfo::fileLastSeen() const
{
    return lastseen;
}

const int& FileInfo::filePriority() const
{
    return priority;
}

const QHash<int,QByteArray>& FileInfo::fileSources() const
{
    return sources;
}

const QString& FileInfo::fileComment() const
{
    return comment;
}


void FileInfo::setFileName(const QString& newname)
{
    name = newname;
}

void FileInfo::addSource(int source)
{
    if (!sources.contains(source))
        sources.insert(source, QByteArray());
}

bool FileInfo::removeSource(int source)
{
    return sources.remove(source) != 0;
}

void FileInfo::updateAvailability(int source, const QByteArray &avail)
{
    sources.insert(source, avail);
}

void FileInfo::updateDownloadStatus(DonkeyMessage* msg, int)
{
    downloaded = msg->readInt64();
    speed = msg->readFloat();
    if (msg->opcode() >= 46)
        lastseen = msg->readInt32();
}


QString FileInfo::md4ToString(const QByteArray& hash)
{
    return QString( hash.toHex().toUpper() );
}

QByteArray FileInfo::stringToMd4(const QString& hash)
{
    if (hash.length() != 32) return QByteArray(16,0);
    return QByteArray::fromHex( hash.toLatin1() );
}

// Utility functions
QString FileInfo::humanReadableSize( int64 rsz )
{
    QString foo;
    double sz = (double)rsz;

    if ( sz >= (100.0 * 1024.0 * 1024.0 * 1024.0) )
    {
        sz = sz / (1024.0 * 1024.0 * 1024.0);
        // xgettext: no-c-format
        foo = ki18nc( "gigabyte suffix", "%1G" ).subs( QLocale().toString(double(sz), 'f', 1) ).toString();
    } else if ( sz >= (10.0 * 1024.0 * 1024.0 * 1024.0) )
    {
        sz = sz / (1024.0 * 1024.0 * 1024.0);
        // xgettext: no-c-format
        foo = ki18nc( "gigabyte suffix", "%1G" ).subs( QLocale().toString(double(sz), 'f', 2) ).toString();
    } else if ( sz >= (1024.0 * 1024.0 * 1024.0) )
    {
        sz = sz / (1024.0 * 1024.0 * 1024.0);
        // xgettext: no-c-format
        foo = ki18nc( "gigabyte suffix", "%1G" ).subs( QLocale().toString(double(sz), 'f', 3) ).toString();
    } else if ( sz >= (1024.0 * 1024.0) )
    {
        sz = sz / (1024.0 * 1024.0);
        // xgettext: no-c-format
        foo = ki18nc( "megabyte suffix", "%1M" ).subs( QLocale().toString(double(sz), 'f', 1) ).toString();
    } else if ( sz >= 1024.0)
    {
        sz = sz / 1024.0;
        // xgettext: no-c-format
        foo = ki18nc( "kilobyte suffix", "%1K" ).subs( QLocale().toString(double(sz), 'f', 1) ).toString();
    } else
        foo = QLocale().toString(double(sz), 'f', 0);

    return foo;
}


QString FileInfo::humanReadableSpeed( double sp )
{
    if (!sp)
        return i18nc( "signifies absence of data in list columns", "-" );
    else
        return QLocale().toString(double(sp / 1024.0), 'f', 1);
}


QString FileInfo::humanReadableTime( time_t t, bool shortFormat )
{
    if ( !t )
        return i18nc( "zero seconds", "0s" );
    if ( t < 0 )
        return i18nc( "signifies absence of data in list columns", "-" );

    QString foo;
    int f = 0;

    if ( t >= 24*3600*365) {
        return QChar(0x221e);//infinity sign
    }
    if ( t > 86400 )
    {
        // xgettext: no-c-format
        foo += i18nc( "number of days", "%1d ", QLocale().toString(double(t/86400), 'f', 0) );
        t %= 86400;
        f = 1;
        if ( shortFormat )
            return foo.simplified();
    }
    if ( t > 3600 )
    {
        // xgettext: no-c-format
        foo += i18nc( "number of hours", "%1h ", QLocale().toString(double(t/3600), 'f', 0) );
        t %= 3600;
        if ( shortFormat )
            return foo.simplified();
    }
    if ( t > 60 )
    {
        // xgettext: no-c-format
        foo += i18nc( "number of minutes", "%1m ", QLocale().toString(double(t/60), 'f', 0) );
        t %= 60;
        if ( shortFormat )
            return foo.simplified();
    }

    if ( t && !f )
        // xgettext: no-c-format
        foo += i18nc( "number of seconds", "%1s", QLocale().toString(double(t), 'f', 0) );

    return foo.simplified();
}

void help_dldata(const FileInfo *fi, qint64 &left, qint64 &loaded, time_t &timeDelta, double &spd)
{
    left = fi->fileSize() - fi->fileDownloaded();
    loaded = fi->fileDownloaded() - fi->fileFirstDownloaded();
    timeDelta = time(0) - fi->fileFirstTime();
    spd = timeDelta ? (double)loaded / timeDelta : 0.0;
}

double FileInfo::calculateETANumeric( const FileInfo* fi )
{
    qint64 left;
    qint64 loaded;
    time_t delta;
    double spd;
    help_dldata(fi, left, loaded, delta, spd);
    if (left <= 0)
        return 0;
    if ( !loaded || !delta )
        return 3600*24*365;
    return left/spd;
}


QString FileInfo::calculateETA( const FileInfo* fi )
{
    qint64 left;
    qint64 loaded;
    time_t delta;
    double spd;
    help_dldata(fi, left, loaded, delta, spd);
    if (left < 0)
        return i18nc( "file should have completed already", "Overdue" );
    if (left == 0)
        return i18nc( "file is just about to complete", "Imminent" );
    if ( !loaded || !delta)
        return i18nc( "signifies absence of data in list columns", "-" );
    return humanReadableTime( (time_t)(left/spd), false );
}


QString FileInfo::humanReadablePriority( int pri )
{
    if ( pri > 0 )
        return pri > 10 ? i18nc( "very high priority", "Very high" ) : i18nc( "high priority", "High" );
    if ( pri < 0 )
        return pri < -10 ? i18nc( "very low priority", "Very low" ) : i18nc( "low priority", "Low" );
    return i18nc( "normal priority", "Normal" );
}
