/* -*- c++ -*-
 *
 * hostmanager.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
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

#include "hostmanager.h"
#include "donkeyhost.h"

#ifndef LIBKMLDONKEY_NO_KDE
#include <kconfig.h>
#include <kconfiggroup.h>
#include <QStandardPaths>
#endif

HostManager::HostManager(QObject* parent, const char* name, bool disableNotification)
    : QObject(parent)
{
    setObjectName(name ? name : "HostManager");
    refreshHostList();

    if (!disableNotification) {
        configWatcher = new QFileSystemWatcher(this);
        connect(configWatcher, SIGNAL(fileChanged(const QString&)), SLOT(fileChanged(const QString&)));

#ifndef LIBKMLDONKEY_NO_KDE
        configWatcher->addPath( QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/mldonkeyrc" );
#endif
    }
}

HostManager::~HostManager()
{
    qDeleteAll(m_hosts);
}

void HostManager::refreshHostList()
{
    qDeleteAll(m_hosts);
    m_hosts.clear();
    m_default = QString();

#ifndef LIBKMLDONKEY_NO_KDE
    KConfig *config = new KConfig("mldonkeyrc");

    QStringList list = config->groupList();
    QStringList::iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        KConfigGroup group = config->group(*it);
        if(group.hasKey("DonkeyHost")) {
            DonkeyHost* entry = new DonkeyHost(*it,
                group.readEntry("DonkeyHost", "localhost"),
                group.readEntry("DonkeyGuiPort", 4001),
                group.readEntry("DonkeyHTTPPort", 4080),
                group.readEntry("DonkeyUsername", "admin"),
                group.readEntry("DonkeyPassword"),
                (HostInterface::HostType)group.readEntry("HostMode", 0),
                QUrl( group.readEntry<QString>("BinaryPath", QString()) ),
                QUrl( group.readEntry<QString>("RootPath", QString()) ),
                (HostInterface::StartupMode)group.readEntry("StartupMode", 0) );

            if (group.readEntry("Default", false) && m_default.isNull())
                m_default = entry->name();
            m_hosts.insert(entry->name(), entry);
        }
    }
    if (m_hosts.empty()) { // create default
        DonkeyHost* entry = new DonkeyHost("MLDonkey", "localhost", 4001, 4080, "admin", "", HostInterface::External);
        m_default = entry->name();
        m_hosts.insert(entry->name(), entry);
    }
    if (m_default.isNull()) {
        QMap<QString,HostInterface*>::Iterator it = m_hosts.begin();
        m_default = it.key();
    }

    delete config;
#else
    //TODO
#endif
}

void HostManager::fileChanged(const QString&)
{
    refreshHostList();
    emit hostListUpdated();
}

QStringList HostManager::hostList() const
{
    return m_hosts.keys();
}

QStringList HostManager::hostList(HostInterface::HostType filter) const
{
    QMap<QString,HostInterface*>::ConstIterator it;
    QStringList l;
    for (it = m_hosts.begin(); it != m_hosts.end(); ++it) {
        if (it.value()->type() == filter)
            l.append(it.key());
    }
    return l;
}

const QString& HostManager::defaultHostName() const
{
    return m_default;
}

void HostManager::setDefaultHostName(const QString& hostname)
{
    m_default = hostname;
}

HostInterface* HostManager::defaultHost() const
{
    return m_hosts[m_default];
}

HostInterface* HostManager::hostProperties(const QString& hostName) const
{
    return m_hosts[hostName];
}

bool HostManager::validHostName(const QString& hostName) const
{
    return m_hosts.contains(hostName);
}

HostInterface::HostType HostManager::hostType(const QString& hostName) const
{
    if (!validHostName(hostName)) return HostInterface::Unknown;
    return m_hosts[hostName]->type();
}
