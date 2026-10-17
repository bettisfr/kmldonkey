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

#include "hostselectaction.h"
#include "hostmanager.h"
#include "donkeyhost.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <QStandardPaths>
#include <kdirwatch.h>
#include <QIcon>
#include <QSignalMapper>
#include <QToolButton>
#include <QMenu>

HostSelectAction::HostSelectAction(const QString& text, const QString& icon,
                                   HostManager* hostManager, QObject* parent)
    : KActionMenu(QIcon::fromTheme(icon), text, parent)
{
    setObjectName("HostSelectAction");
    if (hostManager)
        m_hostManager = hostManager;
    else
        m_hostManager = new HostManager(this);
    setPopupMode(QToolButton::DelayedPopup);
    m_connectMapper = new QSignalMapper(this);
    connect(m_connectMapper, SIGNAL(mappedString(const QString&)), SLOT(slotItemSelected(const QString&)));
    populateMenu();
    connect(m_hostManager, SIGNAL(hostListUpdated()), SLOT(populateMenu()));
}

void HostSelectAction::populateMenu()
{
    qDeleteAll(menu()->actions());
    foreach (const QString &host, m_hostManager->hostList()) {
        QAction *action = new QAction(host, this);
        connect(action, SIGNAL(triggered()), m_connectMapper, SLOT(map()));
        m_connectMapper->setMapping(action, host);
        addAction(action);
    }
}

void HostSelectAction::slotItemSelected(const QString& item)
{
    if (m_hostManager->validHostName(item)) {
        emit hostSelected(item);
        emit hostSelected(m_hostManager->hostProperties(item));
    }
}
