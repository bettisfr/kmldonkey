/* -*- c++ -*-
 *
 * downloadrenamer.h
 *
 * Copyright (C) 2009       Aleksey Markelov <markelovai@gmail.com>
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

#ifndef KMLDONKEY_DOWNLOADRENAMER_H
#define KMLDONKEY_DOWNLOADRENAMER_H

#include <QObject>

/**
    \brief E2DK file automatic download renamer

    This class provides a mechanism so the user cabn automaticaly rename a file which is started
    to being downloaded from the Donkey search results tab. As MLNet doesn't support this feature
    directly anymore, what the program does is to start downloding the file, then issue a "rename"
    command over the downloaded file.

    \author Patch was originally provided by Aleksey Markelov (MarkelovAI@gmail.com)
    \sa <a href="http://bugs.kde.org/show_bug.cgi?id=109704">KDE bug 109704</a>

  */

class DownloadRenamer : public QObject
{
    Q_OBJECT

public:
    DownloadRenamer( const QString &edk2Hash, const QString &newName, QObject *parent = 0 );
    virtual ~DownloadRenamer();

private slots:
    void downloadStarted( int fileNo );

private:
    const QString m_ed2kHash;
    const QString m_newName;
};

#endif // KMLDONKEY_DOWNLOADRENAMER_H
