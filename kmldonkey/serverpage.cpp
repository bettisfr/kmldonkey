#include <KConfigGroup>
/* -*- c++ -*-
 *
 * serverpage.cpp
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

#include <KLocalizedString>
#include <QDebug>
#include <kconfig.h>
#include <QAction>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <QApplication>
#include <QSpinBox>
#include <kxmlguifactory.h>
#include <QCheckBox>
#include <QClipboard>
#include <QList>
#include <QMenu>

#include "infolist.h"
#include "prefs/prefs.h"
#include "kmldonkey.h"
#include "addserver.h"

#include "serverinfo.h"
#include "clientinfo.h"
#include "network.h"

#include "serverpage.h"

ServerPage::ServerPage(QWidget* parent)
    : VBoxWidget(parent)
    , KMLDonkeyPage()
    , ClipboardHelper()
{
    setObjectName("serverPage");
    setMargin(8);
    setSpacing(8);
    m_addServerAction = 0;

    serverView = new InfoList(this, "serverView", true);
    serverView->addColumn( i18n( "Server name" ), 200 );
    serverView->addColumn( i18n( "Network" ) );
    serverView->addColumn( i18n( "Status" ) );
    serverView->addColumn( i18n( "Users" ) );
    serverView->addColumn( i18n( "Files" ) );
    serverView->addColumn( i18n( "Description" ), 200 );
    serverView->addColumn( i18n( "Address" ) );
    serverView->addColumn( i18n( "Port" ) );
    serverView->addColumn( i18n( "Score" ), 60 );
    serverView->addColumn( i18n( "ID" ) );
    serverView->addColumn( i18n( "Preferred" ) );

    connect(serverView, SIGNAL(contextMenu(QTreeWidgetItem*, const QPoint&)), this, SLOT(contextServer(QTreeWidgetItem*, const QPoint&)));
    connect(serverView, SIGNAL(itemSelectionChanged()), SLOT(pleaseUpdateActions()));
    connect(serverView, SIGNAL(gotFocus()), SLOT(pleaseUpdateActions()));

    connect(KMLDonkey::App->donkey, SIGNAL(serverUpdated(int)), this, SLOT(serverUpdated(int)));
    connect(KMLDonkey::App->donkey, SIGNAL(serverRemoved(int)), this, SLOT(serverRemoved(int)));
    connect(KMLDonkey::App->donkey, SIGNAL(networkUpdated(int)), this, SLOT(networkUpdated(int)));
}

ServerPage::~ServerPage()
{
    qDeleteAll( serverItemList );
    serverItemList.clear();
}

void ServerPage::handleGenericAction(const QString& action)
{
    if (action == "copy_url") actionServerCopyURL();
    else if (action == "copy_html") actionServerCopyHTML();
}

void ServerPage::deactivatePageActions()
{
    enableActionList(serverActions, false);
}

QStringList ServerPage::supportedGenericActions()
{
    QStringList actions;
    QList<QTreeWidgetItem *> sl = serverView->selectedItems();

    if (!sl.isEmpty()) {
        enableActionList(serverActions, true);
        actions.append("copy_url");
        actions.append("copy_html");
    }
    else enableActionList(serverActions, false);

    return actions;
}

void ServerPage::plugGenericActions(QObject* object, const char* slot)
{
    connect(this, SIGNAL(genericActionsChanged(KMLDonkeyPage*)), object, slot);
}

void ServerPage::pleaseUpdateActions()
{
    emit genericActionsChanged(static_cast<KMLDonkeyPage*>(this));
}

void ServerPage::setupActions(KActionCollection* actionCollection)
{
    QAction* action = new QAction(QIcon::fromTheme("network-connect"), i18n("&Connect to Server"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionConnectServer()));
    actionCollection->addAction("connect_server", action);
    serverActions.append(action);
    KMLDonkey::App->addCoreAction(action);

    action = new QAction(QIcon::fromTheme("network-disconnect"), i18n("&Disconnect From Server"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionDisconnectServer()));
    actionCollection->addAction("disconnect_server", action);
    serverActions.append(action);
    KMLDonkey::App->addCoreAction(action);

    m_addServerAction = new QAction(QIcon::fromTheme("list-add"), i18n("&Add Server..."), this);
    actionCollection->addAction("add_server", m_addServerAction);
    connect(m_addServerAction, SIGNAL(triggered()), this, SLOT(actionAddServer()));
    KMLDonkey::App->addCoreAction(m_addServerAction);

    networkUpdated(0);

    action = new QAction(QIcon::fromTheme("list-remove"), i18n("&Remove Server"), this);
    actionCollection->addAction("remove_server", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionRemoveServer()));
    serverActions.append(action);
    KMLDonkey::App->addCoreAction(action);

    action = new QAction(QIcon::fromTheme("edit-clear"), i18n("Remove &Old Servers"), this);
    actionCollection->addAction("remove_old_servers", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionRemoveOldServers()));
    KMLDonkey::App->addCoreAction(action);

    action = new QAction(QIcon::fromTheme("process-stop"), i18n("&Blacklist Server"), this);
    actionCollection->addAction("blacklist_server", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionBlacklistServer()));
    serverActions.append(action);

    action = new QAction(QIcon::fromTheme("edit-rename"), i18n("Re&name Server"), this);
    actionCollection->addAction("rename_server", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionRenameServer()));
    serverActions.append(action);

    action = new QAction(QIcon::fromTheme("bookmarks"), i18n("Toggle &Preferred"), this);
    actionCollection->addAction("toggle_server_preferred", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionTogglePreferred()));
    serverActions.append(action);

    hideDisconnectedServersAction = new KToggleAction(i18n("&Hide Disconnected Servers"), this);
    actionCollection->addAction("hide_disconnected_servers", hideDisconnectedServersAction);
    connect(hideDisconnectedServersAction, SIGNAL(toggled(bool)), this, SLOT(actionHideDisconnectedServers(bool)));

    hideBlacklistedServersAction = new KToggleAction(i18n("Hide B&lacklisted Servers"), this);
    actionCollection->addAction("hide_blacklisted_servers", hideBlacklistedServersAction);
    connect(hideBlacklistedServersAction, SIGNAL(toggled(bool)), this, SLOT(actionHideBlacklistedServers(bool)));

    action = new QAction(QIcon::fromTheme("network-server"), i18n("Activate Servers Page"), this);
    action->setIconText(i18n("Servers"));
    actionCollection->addAction("activate_page_servers", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionActivatePage()));

    enableActionList(serverActions, false);
}

void ServerPage::clear()
{
    qDeleteAll( serverItemList );
    serverItemList.clear();

    serverView->clear();
}

void ServerPage::configurePrefsDialog(KMLDonkeyPreferences* prefs)
{
    prefs->listsPage->hideDisconnectedServersCheckbox->setChecked(hideDisconnectedServersAction->isChecked());
    prefs->listsPage->hideBlacklistedServersCheckbox->setChecked(hideBlacklistedServersAction->isChecked());

    if (serverView->sortInterval() > 0) {
        prefs->listsPage->autoServerResortCheckbox->setChecked(true);
        prefs->listsPage->serverResortEntry->setValue( serverView->sortInterval() );
    }
    else
        prefs->listsPage->autoServerResortCheckbox->setChecked(false);
}

void ServerPage::applyPreferences(KMLDonkeyPreferences* prefs)
{
    if (prefs) {
        if (prefs->listsPage->hideDisconnectedServersCheckbox->isChecked() != hideDisconnectedServersAction->isChecked())
            actionHideDisconnectedServers( prefs->listsPage->hideDisconnectedServersCheckbox->isChecked() );
        if (prefs->listsPage->hideBlacklistedServersCheckbox->isChecked() != hideBlacklistedServersAction->isChecked())
            actionHideBlacklistedServers( prefs->listsPage->hideBlacklistedServersCheckbox->isChecked() );

        if (prefs->listsPage->autoServerResortCheckbox->isChecked() != (serverView->sortInterval() > 0) ||
            (prefs->listsPage->autoServerResortCheckbox->isChecked() && (serverView->sortInterval() != prefs->listsPage->serverResortEntry->value()) ) )
                serverView->setSortInterval( prefs->listsPage->autoServerResortCheckbox->isChecked() ? prefs->listsPage->serverResortEntry->value() : 0 );
    }

    if (KMLDonkey::App->listFont !=  serverView->font())
        serverView->setFont(KMLDonkey::App->listFont);
}

void ServerPage::restoreState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "ServerPage" );
    hideDisconnectedServersAction->setChecked(group.readEntry("HideDisconnectedServers", true));
    hideBlacklistedServersAction->setChecked(group.readEntry("HideBlacklistedServers", true));
    serverView->setSortInterval( group.readEntry("ServerViewSort", 0) );
    serverView->initialise();
    applyPreferences();
}

void ServerPage::saveState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "ServerPage" );
    group.writeEntry("HideDisconnectedServers", hideDisconnectedServersAction->isChecked());
    group.writeEntry("HideBlacklistedServers", hideBlacklistedServersAction->isChecked());
    group.writeEntry("ServerViewSort", serverView->sortInterval());
    serverView->saveLayout();
}

void ServerPage::contextServer(QTreeWidgetItem*,const QPoint& pt)
{
    QMenu *pop = dynamic_cast<QMenu*>( (KMLDonkey::App->factory())->container("server_actions", KMLDonkey::App) );
    if (!pop)
        KMLDonkey::App->showBadInstallDialog();
    else
        pop->popup(pt);
}

void ServerPage::actionConnectServer()
{
    QList<QTreeWidgetItem *> list = serverView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ServerInfoItem* it = (ServerInfoItem*) i;

        ServerInfo* si = KMLDonkey::App->donkey->findServerNo(it->fileNo());
        if (!si) continue;
        KMLDonkey::App->donkey->connectServer(si->serverNo());
    }
}

void ServerPage::actionConnectMoreServers()
{
    KMLDonkey::App->donkey->connectMoreServers();
}

void ServerPage::actionDisconnectServer()
{
    QList<QTreeWidgetItem *> list = serverView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ServerInfoItem* it = (ServerInfoItem*) i;

        ServerInfo* si = KMLDonkey::App->donkey->findServerNo(it->fileNo());
        if (!si)
            continue;
        KMLDonkey::App->donkey->disconnectServer(si->serverNo());
    }
}

void ServerPage::actionAddServer()
{
    AddServerDialog *dlg = new AddServerDialog(this);
    dlg->exec();
    dlg->deleteLater();
}

void ServerPage::actionRemoveServer()
{
    QList<QTreeWidgetItem *> list = serverView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ServerInfoItem* it = (ServerInfoItem*) i;

        ServerInfo* si = KMLDonkey::App->donkey->findServerNo(it->fileNo());
        if (si)
            KMLDonkey::App->donkey->removeServer(si->serverNo());
    }
}

void ServerPage::actionRemoveOldServers()
{
    KMLDonkey::App->donkey->cleanOldServers();
}

void ServerPage::actionBlacklistServer()
{
    QList<QTreeWidgetItem *> list = serverView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ServerInfoItem* it = (ServerInfoItem*) i;
        KMLDonkey::App->donkey->blacklistServer(it->fileNo());
    }
}

void ServerPage::actionRenameServer()
{
    QList<QTreeWidgetItem *> list = serverView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ServerInfoItem* it = (ServerInfoItem*) i;

        ServerInfo* si = KMLDonkey::App->donkey->findServerNo(it->fileNo());
        if (!si)
            continue;
		
		serverView->editItem( it );
        return;
    }
}

void ServerPage::actionTogglePreferred()
{
    QList<QTreeWidgetItem *> list = serverView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ServerInfoItem* it = (ServerInfoItem*) i;
        ServerInfo* si = KMLDonkey::App->donkey->findServerNo(it->fileNo());
        if (si)
            KMLDonkey::App->donkey->setServerPreferred(si->serverNo(), !si->serverPreferred());
    }
}

void ServerPage::copyServerToClipboard(QList<ServerInfo *>& servers, ClipFormat format)
{
    QClipboard* clip = QApplication::clipboard();
    QStringList out;
    foreach( ServerInfo* server, servers )
    {
        switch (format) {
        case URL:
            out.append("ed2k://|server|" + server->serverAddress() + "|" + QString::number(server->serverPort()) + "|/");
            break;
        case HTML:
            out.append(QString("<a href=\"%1\">%2</a>")
                .arg(("ed2k://|server|" + server->serverAddress() + "|" + QString::number(server->serverPort()) + "|/").toHtmlEscaped(),
                     (server->serverAddress() + ":" + QString::number(server->serverPort())).toHtmlEscaped()));
            break;
        default:
            break;
        }
    }
    clip->setText(out.join("\n"), QClipboard::Selection);
}

void ServerPage::actionServerCopyURL()
{
    QList<ServerInfo *> servers;
    QList<QTreeWidgetItem *> list = serverView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ServerInfoItem* it = (ServerInfoItem*) i;
        ServerInfo* si = KMLDonkey::App->donkey->findServerNo(it->fileNo());
        if (si)
            servers.append(si);
    }
    copyServerToClipboard(servers, URL);
}

void ServerPage::actionServerCopyHTML()
{
    QList<ServerInfo *> servers;
    QList<QTreeWidgetItem *> list = serverView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ServerInfoItem* it = (ServerInfoItem*) i;
        ServerInfo* si = KMLDonkey::App->donkey->findServerNo(it->fileNo());
        if (si)
            servers.append(si);
    }
    copyServerToClipboard(servers, HTML);
}

void ServerPage::actionHideDisconnectedServers(bool show)
{
    hideDisconnectedServersAction->setChecked(show);
    foreach (ServerInfo *si, KMLDonkey::App->donkey->connectedServers()) {
        serverUpdated(si->serverNo());
    }
}

void ServerPage::actionHideBlacklistedServers(bool show)
{
    hideBlacklistedServersAction->setChecked(show);
    foreach (ServerInfo *si, KMLDonkey::App->donkey->connectedServers()) {
        serverUpdated(si->serverNo());
    }
}

void ServerPage::serverUpdated( int server )
{
    ServerInfo* si = KMLDonkey::App->donkey->findServerNo( server );
    ServerInfoItem* df = serverItemList.value( server );

    if ( !si || si->serverState() == ClientInfo::Removed ||
       ( hideDisconnectedServersAction->isChecked() && ( si->serverState() == ClientInfo::NotConnected || si->serverState() == ClientInfo::NotConnected2 ) ) ||
       ( hideBlacklistedServersAction->isChecked()  &&   si->serverState() == ClientInfo::Blacklisted ) )
    {
        if ( df )
            delete serverItemList.take( server );
        return;
    }

    if ( df )
        df->refresh();
    else
    {
        df = new ServerInfoItem( serverView, server );
        serverItemList.insert( server, df );
    }
}

void ServerPage::serverRemoved(int server)
{
    delete serverItemList.take(server);
}

void ServerPage::actionActivatePage()
{
    KMLDonkey::App->activatePage(this);
}

void ServerPage::networkUpdated(int)
{
    if (!m_addServerAction)
        return;

    foreach (Network *nw, KMLDonkey::App->donkey->availableNetworks()) {
        if (nw->networkEnabled()
            && (nw->networkFlags() & Network::NetworkHasServers)) {
            m_addServerAction->setEnabled(true);
            return;
        }
    }
    m_addServerAction->setEnabled(false);
}
