/* -*- c++ -*-
 *
 * fileinfo.h
 *
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

#ifndef __libkmldonkey_fileinfo_h__
#define __libkmldonkey_fileinfo_h__

#include <QString>
#include <QStringList>
#include <QHash>

#include <sys/types.h>
#include <time.h>
#include <kmldonkey_export.h>
#include "donkeytypes.h"

class DonkeyMessage;

//! Representation of a file.

class KMLDONKEY_EXPORT FileInfo
{
public:

    //! File state.
    enum State {
        Downloading = 0,
        Paused,
        Complete,
        Shared,
        Cancelled,
        New,
        Aborted,
        Queued
    };

    FileInfo(int num, DonkeyMessage* msg, int proto);
    ~FileInfo();

    void updateFileInfo(DonkeyMessage* msg, int proto);

    //! The numerical ID of the file.
    const int& fileNo() const;
    //! The network the file is on.
    const int& fileNetwork() const;
    //! The file name.
    const QString& fileName() const;
    //! Alternative file names found on the network.
    const QStringList& fileNames() const;
    //! A list of all the file's uids.
    const QStringList& fileUids() const;
    //! The file's primary uid.
    QString fileUid() const;
    //! Return the uid for the given URN type, without the identifier string.
    QString fileUid(const QString& type) const;
    //! The file size.
    const int64& fileSize() const;
    //! Bytes downloaded so far.
    const int64& fileDownloaded() const;
    //! Bytes downloaded at first time seen
    const int64& fileFirstDownloaded() const;
    const int& fileNLocations() const;
    const int& fileNClients() const;
    //! The file state.
    const State& fileState() const;
    //! If state is Aborted, this is the reason why.
    const QString& fileAbortedMsg() const;
    //! String describing file chunks downloaded, one char for each chunk.
    const QByteArray& fileChunks() const;
    //! A map of networks, with each value describing chunk availability, one char for each chunk.
    const QHash<int,QByteArray>& fileAvailability() const;
    //! Current download speed.
    const double& fileSpeed() const;
    //! The age of individual chunks.
    const QList<time_t>& fileChunksAge() const;
    //! The time the download was started (seconds since Epoch).
    const time_t& fileAge() const;
    //! The time the download was first seen (seconds since Epoch).
    const time_t& fileFirstTime() const;
    //! The file format.
    const int& fileFormat() const;
    //! A string describing the file format in more detail.
    const QString& fileFormatInfo() const;
    //! Seconds since the file was last seen on the network.
    const int& fileLastSeen() const;
    //! The file priority.
    const int& filePriority() const;
    //! File availability described by a map of client IDs to strings similar to fileChunks().
    const QHash<int,QByteArray>& fileSources() const;
    //! The file's comment, if any.
    const QString& fileComment() const;

    void setFileName(const QString& newname);

    void addSource(int source);
    bool removeSource(int source);
    void updateAvailability(int source, const QByteArray &avail);
    void updateDownloadStatus(DonkeyMessage* msg, int proto);

    //! Converts a QByteArray containing an MD4 hash to a readable string.
    static QString md4ToString(const QByteArray& hash);
    //! Converts a string describing an MD4 hash into a QByteArray.
    static QByteArray stringToMd4(const QString& hash);

    static QString humanReadableSize( int64 rsz );
    static QString humanReadableSpeed( double sp );
    static QString humanReadableTime( time_t t, bool shortFormat );
    static double calculateETANumeric( const FileInfo* fi );
    static QString calculateETA( const FileInfo* fi );
    static QString humanReadablePriority( int pri );

private:

    int num, network;
    QString name;
    QStringList names, uids;
    int64 size, downloaded, firstdown;
    int nlocations, nclients;
    State state;
    QString abortedmsg;
    QByteArray chunks;
    QHash<int,QByteArray> availability;
    double speed;
    QList<time_t> chunks_age;
    time_t age, firsttime;
    int format;
    QString formatinfo;
    int lastseen, priority;
    QString comment;

    QHash<int,QByteArray> sources;

    Q_DISABLE_COPY(FileInfo)
};

#endif
