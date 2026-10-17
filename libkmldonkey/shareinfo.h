/* -*- c++ -*-
 *
 * shareinfo.h
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

#ifndef __libkmldonkey_shareinfo_h__
#define __libkmldonkey_shareinfo_h__

#include <QString>
#include <QStringList>
#include <kmldonkey_export.h>

#include "donkeytypes.h"

class DonkeyMessage;

//! Representation of a shared file.

class KMLDONKEY_EXPORT ShareInfo
{

    public:

    ShareInfo(int num, DonkeyMessage* msg, int proto);
    ~ShareInfo();

    void updateShareInfo(DonkeyMessage* msg, int proto);

    void updateShare(DonkeyMessage* msg, int proto);

    //! The numerical ID of the share.
    const int& shareNo() const;
    //! The network the file is being shared on.
    const int& shareNetwork() const;
    //! The file name of the share.
    const QString& shareName() const;
    //! The size of the shared file, in bytes.
    const int64& shareSize() const;
    //! The number of bytes uploaded to clients.
    const int64& shareUploaded() const;
    //! The number of requests received for this share.
    const int& shareRequests() const;
    //! Return the list of uids relevant to this share.
    const QStringList& shareUids() const;
    //! Return the first uid of the file, including the identifier string.
    QString shareUid() const;
    //! Return the uid for the given URN type, without the identifier string.
    QString shareUid(const QString& type) const;

private:

    int num, network;
    QString name;
    int64 size, uploaded;
    int requests;
    QStringList uids;

    Q_DISABLE_COPY(ShareInfo)
};

#endif
