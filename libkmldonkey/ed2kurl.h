/* -*- c++ -*-
 *
 * ed2kurl.h
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

#ifndef __libkmldonkey_ed2kurl_h__
#define __libkmldonkey_ed2kurl_h__

#include <QString>
#include <kmldonkey_export.h>

#include "donkeytypes.h"

class FileInfo;
class ShareInfo;
class ResultInfo;
class ServerInfo;

/**
 * Representation of an ED2K URL.
 */

class KMLDONKEY_EXPORT ED2KURL
{
public:

    /**
     * Construct an ED2K URL from a QUrl.
     */
    ED2KURL(const QUrl& u);
    explicit ED2KURL(const QString& text);
    /**
     * Construct an ED2K URL from a FileInfo object.
     */
    ED2KURL(FileInfo*);
    /**
     * Construct an ED2K URL from a ShareInfo object.
     */
    ED2KURL(ShareInfo*);
    /**
     * Construct an ED2K URL from a ResultInfo object.
     */
    ED2KURL(ResultInfo*);
    /**
     * Construct an ED2K URL from a ServerInfo object.
     */
    ED2KURL(ServerInfo*);

    /**
     * Return the type of URL, either @p "file", @p "server", or @p "invalid".
     */
    QString ed2kType() const;

    /**
     * If this is a server URL, return the server address.
     * @return The IP address of this server URL.
     */
    QString serverAddress() const;
    /**
     * If this is a server URL, return the server port.
     * @return The port number of this server URL.
     */
    int16 serverPort() const;

    /**
     * If this is a file URL, return the file name.
     * @return The name of this file URL.
     */
    QString fileName() const;
    /**
     * If this is a file URL, return the file size.
     * @return The size of this file URL.
     */
    int64 fileSize() const;
    /**
     * If this is a file URL, return the file's MD4 hash.
     * @return The MD4 hash of this file URL.
     */
    QByteArray fileHash() const;

    /**
     * Determine if two ED2KURL objects refer to the same file.
     * This matches the file size and hash, but not the name. Two files
     * can have different names but identical content.
     *
     * @param u The URL to match this URL with.
     * @return True if the two URLs refer to the same file.
     */
    bool isSameFile(const ED2KURL& u) const;

    /**
     * Return a string representation of this URL.
     * @return A string containing a representation of this URL.
     */
    QString toString() const;

    /**
     * Determine if this URL is invalid.
     * @return True if the URL is invalid.
     */
    bool isInvalid() const;

private:

    QString type, address, name;
    QByteArray hash;
    int64 size;
    int16 port;

};

#endif
