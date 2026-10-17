/* -*- c++ -*-
 *
 * donkeyhost.h
 *
 * Copyright (C) 2003, 2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003 Sebastian Sauer <mail@dipe.org>
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

#ifndef __libkmldonkey_donkeyhost_h__
#define __libkmldonkey_donkeyhost_h__

#include <QVariant>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QObject>

#include <hostiface.h>
#include <kmldonkey_export.h>

//! Representation of an MLDonkey host.
class KMLDONKEY_EXPORT DonkeyHost : public HostInterface
{

public:
    DonkeyHost(const QString& h_name, const QString& h_address, int h_port, int h_httpPort,
               const QString& h_username, const QString& h_password,
               HostInterface::HostType h_type = HostInterface::External,
               const QUrl& h_binaryPath = QUrl(), const QUrl& h_rootPath = QUrl(),
               HostInterface::StartupMode h_startupMode = AtKDEStart);
    DonkeyHost(DonkeyHost&);
    DonkeyHost();
    virtual ~DonkeyHost() {}

    //! The username to authenticate with.
    const QString& username() const;
    //! The password to authenticate with.
    const QString& password() const;
    //! The HTTP interface port.
    int httpPort() const;

    //! Set the username to authenticate with.
    void setUsername(const QString&);
    //! Set the password to authenticate with.
    void setPassword(const QString&);
    //! Set the HTTP interface port.
    void setHttpPort(int);

private:
    QString m_username,
            m_password;
    int m_httpPort;
};

Q_DECLARE_METATYPE(DonkeyHost*)

#endif
