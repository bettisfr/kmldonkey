/* -*- c++ -*-
 *
 * hostiface.h
 *
 * Copyright (C) 2003 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2003, 2004 Petter Stokke <ummo@hellokitty.com>
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

#ifndef __libkmldonkey_hostiface_h__
#define __libkmldonkey_hostiface_h__

#include <QString>
#include <QUrl>
#include <QMetaType>

/** Base class for all hosts. DonkeyHost and GiftHost are inherited from
 * this class as example.
 */
class HostInterface
{
public:
    enum HostType {
        Unknown = -1,
        External = 0,
        // Local (1) is obsolete
        Managed = 2
    };

    enum StartupMode {
        AtKDEStart = 0,
        AtKMLDonkeyStart
    };

    HostInterface(const QString& h_name = QString(),
                  const QString& h_address = QString(),
                  int h_port = 0,
                  HostType h_type = Unknown,
                  const QUrl& h_binaryPath = QUrl(),
                  const QUrl& h_rootPath = QUrl(),
                  StartupMode h_startupMode = AtKDEStart)
    {
        setName(h_name);
        setAddress(h_address);
        setPort(h_port);
        setType(h_type);
        setBinaryPath(h_binaryPath);
        setRootDirectory(h_rootPath);
        setStartupMode(h_startupMode);
    }

    HostInterface(const HostInterface& c) {
        setName(c.name());
        setAddress(c.address());
        setPort(c.port());
        setType(c.type());
        setBinaryPath(c.binaryPath());
        setRootDirectory(c.rootDirectory());
        setStartupMode(c.startupMode());
    }

    virtual ~HostInterface() {}

    //! The host ID (descriptive name) of the host.
    virtual const QString& name() const { return m_name; }
    virtual void setName(const QString& h_name) { m_name = h_name; }

    //! The network address of the host.
    virtual const QString& address() const { return m_address; }
    virtual void setAddress(const QString& h_address) { m_address = h_address.trimmed(); }

    //! The (GUI-) port used to communicate with the core.
    virtual const int& port() const { return m_port; }
    virtual void setPort(const int h_port) { m_port = h_port; }

    //! The type of this host.
    const HostType& type() const { return m_type; }
    void setType(HostType h_type) { m_type = h_type; }

    //! The path to this host's core executable. Only meaningful for managed hosts.
    virtual const QUrl& binaryPath() const { return m_binary; }
    virtual void setBinaryPath(const QUrl& path) { m_binary = path; }
    //! The path to this host's root directory. Only meaningful for managed hosts.
    virtual const QUrl& rootDirectory() const { return m_root; }
    virtual void setRootDirectory(const QUrl& path) { m_root = path; }
    //! The host's startup mode. Only meaningful for managed hosts.
    const StartupMode& startupMode() const { return m_startupMode; }
    void setStartupMode(StartupMode mode) { m_startupMode = mode; }

private:
    QString m_name,
            m_address;
    QUrl m_binary, m_root;
    int m_port;
    HostType m_type;
    StartupMode m_startupMode;
};

Q_DECLARE_METATYPE(HostInterface*)

#endif
