/* -*- c++ -*-
 *
 * friendpage.h
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
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

#ifndef __kmldonkey_friendpage_h__
#define __kmldonkey_friendpage_h__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QObject>
#include <QSplitter>
#include <QTextBrowser>
#include "autobox.h"
#include <QHash>
#include <QList>

#include <searchinfo.h>

#include "clipboard.h"
#include "page.h"

#include <KCoreAddons>

class KActionCollection;
class KLineEdit;

class KMLDonkeyPreferences;
class InfoList;
class ClientFile;

class QTabWidget;

class FriendShares
{
public:
    QMap<QString,QList<int> > dirs;
};

class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    ChatWidget(int client, QWidget* parent = 0);

public slots:
    void messageFromClient(int client, const QString& message);

protected slots:
    void messageEntered(const QString& message);

private:
    void setupChatWidget(int client);

    int clno;
    QTextBrowser* output;
    KLineEdit* input;
};

class FriendPage : public VBoxWidget, public KMLDonkeyPage, public ClipboardHelper
{
    Q_OBJECT
public:
    FriendPage(QWidget* parent = 0);
    virtual ~FriendPage();

    virtual void setupActions(KActionCollection*);
    virtual QStringList supportedGenericActions();
    virtual void handleGenericAction(const QString&);
    virtual void plugGenericActions(QObject*, const char*);
    virtual void deactivatePageActions();

    virtual void applyPreferences(KMLDonkeyPreferences* prefs = 0);
    virtual void saveState(KSharedConfigPtr);
    virtual void restoreState(KSharedConfigPtr);

    virtual void clear();

signals:
    void genericActionsChanged(KMLDonkeyPage*);

public slots:
    void friendUpdated(int client);
    void friendRemoved(int client);

protected slots:
    void closeAllFileLists();
    void openFileList(int client);
    void closeFileList(QWidget*);

    void openFriendFiles(QTreeWidgetItem* item, int );

    void contextFriend( QTreeWidgetItem*,const QPoint& );
    void contextFile( QTreeWidgetItem*,const QPoint& );

    void actionAddFriend();
    void actionRemoveFriend();
    void actionRemoveAllFriends();
    void actionConnectFriend();
    void actionOpenChat();

    void fileDoubleClick(QTreeWidgetItem*);
    void actionDownload();
    void actionForceDownload();
    void actionDownloadAs();

    void actionActivatePage();

    void receiveShare(int clno, const QString& dir, int result);

    void mightUpdateShareList();

    void pleaseUpdateActions();

    void messageReceived(int client, const QString& message);
    void closeChat(QWidget*);

private:
    void constructFileList(int clno);
    QList<QTreeWidgetItem *> currentFileListSelection();
    void downloadSelected(bool force, bool ask);
    void copyFileToClipboard(ClipFormat format);

    QSplitter *friendVSplitter, *friendHSplitter;
    InfoList *friendView;
    QTabWidget* fileTab;
    QTabWidget* chatTab;

    QHash<int, ChatWidget *> chats;
    QHash<int, InfoList *> fileViews;

    QHash<int, ClientItem *> friendList;
    QHash<int, FriendShares *> shares;

    QMap<int,bool> pleaseUpdateShareList;

    QList<QAction *> friendActions, shareActions;
    QAction* m_removeAllAction;
    QAction* m_chatAction;
};

#endif
