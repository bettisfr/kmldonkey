/* -*- c++ -*-
 *
 * serverpage.h
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
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

#ifndef __kmldonkey_serverpage_h__
#define __kmldonkey_serverpage_h__

#include <QObject>
#include <QHash>
#include <QList>
#include "autobox.h"

#include "clipboard.h"
#include "page.h"

class KActionCollection;

class KMLDonkeyPrefs;
class InfoList;

class ServerPage : public VBoxWidget, public KMLDonkeyPage, public ClipboardHelper
{
    Q_OBJECT
public:
    ServerPage(QWidget* parent = 0);
    virtual ~ServerPage();

    virtual void setupActions(KActionCollection*);
    virtual QStringList supportedGenericActions();
    virtual void handleGenericAction(const QString&);
    virtual void plugGenericActions(QObject*, const char*);
    virtual void deactivatePageActions();

    virtual void configurePrefsDialog(KMLDonkeyPreferences* prefs);
    virtual void applyPreferences(KMLDonkeyPreferences* prefs = 0);
    virtual void saveState(KSharedConfigPtr);
    virtual void restoreState(KSharedConfigPtr);

    virtual void clear();

    KToggleAction *hideDisconnectedServersAction;
    KToggleAction *hideBlacklistedServersAction;

signals:
    void genericActionsChanged(KMLDonkeyPage*);

public slots:
    void serverUpdated(int server);
    void serverRemoved(int server);

protected slots:
    void contextServer(QTreeWidgetItem*,const QPoint&);

    void actionConnectServer();
    void actionConnectMoreServers();
    void actionDisconnectServer();
    void actionAddServer();
    void actionRemoveServer();
    void actionRemoveOldServers();
    void actionBlacklistServer();
    void actionServerCopyURL();
    void actionServerCopyHTML();
    void actionHideDisconnectedServers(bool show);
    void actionHideBlacklistedServers(bool show);
    void actionRenameServer();
    void actionTogglePreferred();

    void actionActivatePage();

    void pleaseUpdateActions();

    void networkUpdated(int);

protected:
    void copyServerToClipboard(QList<ServerInfo *>& servers, ClipFormat format);

private:
    InfoList *serverView;
    QHash<int, ServerInfoItem *> serverItemList;
    QList<QAction *> serverActions;

    QAction* m_addServerAction;
};

#endif
