/* -*- c++ -*-
 *
 * hostmanager.h
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

#ifndef __libkmldonkey_hostmanager_h__
#define __libkmldonkey_hostmanager_h__

#include <QString>
#include <QStringList>
#include <QMap>
#include <QObject>
#include <QFileSystemWatcher>
#include <QSignalMapper>

#include "hostiface.h"
#include "kmldonkey_export.h"

//! Interface to the KMLDonkey host configuration.
class KMLDONKEY_EXPORT HostManager : public QObject
{
    Q_OBJECT

public:

    //! Construct a new HostManager.
    HostManager(QObject* parent = 0, const char* name = 0, bool disableNotification = false);
    virtual ~HostManager();

    //! Re-read the configuration from disk.
    void refreshHostList();
    //! Return a list of host IDs.
    QStringList hostList() const;
    //! Return a list of host IDs filtered on the host type.
    QStringList hostList(HostInterface::HostType filter) const;
    //! Verify that the given string is a valid host ID.
    bool validHostName(const QString& hostName) const;
    //! Get the host ID of the default host.
    const QString& defaultHostName() const;
    //! Set the host ID of the default host.
    void setDefaultHostName(const QString& hostname);
    //! Get a HostInterface object describing the default host.
    HostInterface* defaultHost() const;
    //! Get a HostInterface object describing the named host.
    HostInterface* hostProperties(const QString& hostName) const;
    //! Find the host type of the named host.
    HostInterface::HostType hostType(const QString& name) const;
    //! Check if the named host is a local type (Local or Managed).
    bool isLocalHost(const QString& name) const;

signals:

    //! Emitted when the host list has changed, usually because the configuration file has been updated.
    void hostListUpdated();

private slots:

    void fileChanged(const QString&);

private:
    QMap<QString,HostInterface*> m_hosts;
    QString m_default;

    QFileSystemWatcher *configWatcher;

};

#endif
