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

#ifndef __libkmldonkey_hostselectaction_h__
#define __libkmldonkey_hostselectaction_h__

#include <QString>
#include <QStringList>
#include <qmap.h>
#include <QObject>
#include <QAction>
#include <kactionmenu.h>
#include <QUrl>
#include <kmldonkey_export.h>
#include "hostiface.h"
class DonkeyHost;
class GiftHost;

class KDirWatch;
class QSignalMapper;

class HostManager;

//! QAction for selecting a host from the HostManager.
/**
 * This QAction provides a list of the hosts handled by the HostManager object tied to it.
 * It is meant to be operated by the user in two ways: either clicking on it in a toolbar
 * to perform a default connect (when the standard activated() signal is emitted), or
 * selecting a host from its submenu signifying the user wants to connect to the given
 * host (when hostSelected() is emitted).
 *
 * It automatically reconstructs its submenu when the attached HostManager signals the
 * host list has changed.
 */

class KMLDONKEY_EXPORT HostSelectAction : public KActionMenu
{
    Q_OBJECT

public:

    //! Construct a new HostSelectAction.
    HostSelectAction(const QString& text, const QString& icon, HostManager* hostManager = 0, QObject* parent = 0);

signals:

    //! Emitted when a host is selected, provides the name of the selected host.
    void hostSelected(const QString& hostName);
    //! Emitted when a host is selected, provides a reference to the selected HostInterface object.
    void hostSelected(HostInterface* host);

private slots:

    void populateMenu();
    void slotItemSelected(const QString&);

private:

    HostManager* m_hostManager;
    QSignalMapper* m_connectMapper;

};

#endif
