#include <KConfigGroup>
/* -*- c++ -*-
 *
 * friendpage.cpp
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

#include <QMenu>
#include <QClipboard>
#include <QTimer>
#include <QLayout>
#include <QList>
#include <QList>
#include <QVBoxLayout>

#include <KCoreAddons>
#include <QDebug>
#include <KLocalizedString>
#include <kconfig.h>
#include <QAction>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <QTextBrowser>
#include <klineedit.h>
#include "kdeinputdialog.h"
#include <QTabWidget>
#include <kxmlguifactory.h>

#include "donkeyprotocol.h"
#include "clientinfo.h"
#include "network.h"
#include "infolist.h"
#include "prefs/prefs.h"
#include "kmldonkey.h"
#include "friendpage.h"

FriendPage::FriendPage(QWidget* parent)
    : VBoxWidget(parent)
    , KMLDonkeyPage()
    , ClipboardHelper()
{
    setObjectName("friendPage");
    setMargin(8);
    setSpacing(8);

    friendHSplitter = new QSplitter(this);
	friendHSplitter->setObjectName("friendHSplitter");
    friendHSplitter->setOrientation(Qt::Horizontal);

    friendView = new InfoList(friendHSplitter, "friendView", true);
    friendView->addColumn( i18n( "Name" ) );
    friendView->addColumn( i18n( "Network" ) );
    friendView->addColumn( i18n( "Type" ) );
    friendView->addColumn( i18n( "Location" ) );
    friendView->addColumn( i18n( "State" ) );

    friendVSplitter = new QSplitter(friendHSplitter);
	friendVSplitter->setObjectName("friendVSplitter");
    friendVSplitter->setOrientation(Qt::Vertical);

    fileTab = new QTabWidget(friendVSplitter);
    fileTab->setObjectName("fileTab");
    fileTab->setDocumentMode(true);
    fileTab->setTabsClosable(true);
    fileTab->setMovable(true);

    chatTab = new QTabWidget(friendVSplitter);
    chatTab->setObjectName("chatTab");
    chatTab->setDocumentMode(true);
    chatTab->setTabsClosable(true);
    chatTab->setMovable(true);

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(mightUpdateShareList()));
    timer->start(100);

    connect(friendView, SIGNAL(contextMenu(QTreeWidgetItem*, const QPoint&)),
            this, SLOT(contextFriend(QTreeWidgetItem*, const QPoint&)));
    connect(friendView, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(openFriendFiles(QTreeWidgetItem*, int)));
    connect(friendView, SIGNAL(itemSelectionChanged()), SLOT(pleaseUpdateActions()));
    connect(friendView, SIGNAL(gotFocus()), SLOT(pleaseUpdateActions()));

    connect(fileTab, &QTabWidget::tabCloseRequested, this, [this](int index){closeFileList(fileTab->widget(index));});
    connect(chatTab, &QTabWidget::tabCloseRequested, this, [this](int index){closeChat(chatTab->widget(index));});

    connect(KMLDonkey::App->donkey, SIGNAL(friendUpdated(int)), this, SLOT(friendUpdated(int)));
    connect(KMLDonkey::App->donkey, SIGNAL(friendRemoved(int)), this, SLOT(friendRemoved(int)));
    connect(KMLDonkey::App->donkey, SIGNAL(clientFileListing(int,const QString&,int)), this, SLOT(receiveShare(int,const QString&,int)));
    connect(KMLDonkey::App->donkey, SIGNAL(messageFromClient(int, const QString&)), SLOT(messageReceived(int, const QString&)));
}

FriendPage::~FriendPage()
{
    qDeleteAll(friendList);
    friendList.clear();

    qDeleteAll(chats);
    chats.clear();

    qDeleteAll(shares);
    shares.clear();
}

void FriendPage::handleGenericAction(const QString& action)
{
    if (action == "copy_url") copyFileToClipboard(URL);
    else if (action == "copy_html") copyFileToClipboard(HTML);
    else if (action == "copy_hash") copyFileToClipboard(Hash);
    else if (action == "search_download") actionDownload();
    else if (action == "search_force_download") actionForceDownload();
    else if (action == "search_download_as") actionDownloadAs();
}

void FriendPage::deactivatePageActions()
{
    m_chatAction->setEnabled(false);
    enableActionList(friendActions, false);
    enableActionList(shareActions, false);
}

QStringList FriendPage::supportedGenericActions()
{
    QStringList actions;
    QList<QTreeWidgetItem *> fl = friendView->selectedItems();
    InfoList* fileView = static_cast<InfoList*>(fileTab->currentWidget());

    if (friendView->hasFocus()) {
        enableActionList(shareActions, false);
        if (!fl.isEmpty()) enableActionList(friendActions, true);
        else enableActionList(friendActions, false);
        bool chatEnabled = false;
        QList<QTreeWidgetItem *>::iterator it;
        for ( it = fl.begin(); it != fl.end(); ++it) {
            ClientInfo* cl = KMLDonkey::App->donkey->findClientNo(static_cast<ClientItem*>(*it)->fileNo());
            if (cl) {
                Network* nw = KMLDonkey::App->donkey->findNetworkNo(cl->clientNetwork());
                if (nw && nw->networkFlags() & Network::NetworkHasChat)
                    chatEnabled = true;
            }
        }
        m_chatAction->setEnabled(chatEnabled);
    }

    else if (fileView && fileView->hasFocus()) {
        enableActionList(friendActions, false);
        if (!currentFileListSelection().isEmpty()) {
            enableActionList(shareActions, true);
            actions.append("copy_url");
            actions.append("copy_html");
            actions.append("copy_hash");
            actions.append("search_download");
            actions.append("search_force_download");
            actions.append("search_download_as");
        }
        else enableActionList(shareActions, false);
    }

    else {
        enableActionList(friendActions, false);
        enableActionList(shareActions, false);
    }

    return actions;
}

void FriendPage::plugGenericActions(QObject* object, const char* slot)
{
    connect(this, SIGNAL(genericActionsChanged(KMLDonkeyPage*)), object, slot);
}

void FriendPage::pleaseUpdateActions()
{
    emit genericActionsChanged(static_cast<KMLDonkeyPage*>(this));
}

void FriendPage::setupActions(KActionCollection* actionCollection)
{
    QAction* action = new QAction(QIcon::fromTheme("edit-find-user"), i18n("&Search for Friend..."), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionAddFriend()));
    actionCollection->addAction("add_friend", action);
    KMLDonkey::App->addCoreAction(action);

    action = new QAction(QIcon::fromTheme("list-add-user"), i18n("C&onnect to Friend"), this);

    connect(action, SIGNAL(triggered()), this, SLOT(actionConnectFriend()));
    actionCollection->addAction("connect_friend", action);
    friendActions.append(action);

    m_chatAction = new QAction(QIcon::fromTheme("x-office-contact"), i18n("Open Friend &Chat"), this);
    m_chatAction->setEnabled(false);
    connect(m_chatAction, SIGNAL(triggered()), this, SLOT(actionOpenChat()));
    actionCollection->addAction("open_friend_chat", m_chatAction);

    action = new QAction(QIcon::fromTheme("list-remove-user"), i18n("&Remove Friend"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionRemoveFriend()));
    actionCollection->addAction("remove_friend", action);
    friendActions.append(action);

    m_removeAllAction = new QAction(QIcon::fromTheme("list-remove-user"), i18n("Remove All Friends"), this);
    m_removeAllAction->setEnabled(false);
    connect(m_removeAllAction, SIGNAL(triggered()), this, SLOT(actionRemoveAllFriends()));
    actionCollection->addAction("remove_all_friends", m_removeAllAction);

    action = new QAction(QIcon::fromTheme("x-office-contact"), i18n("Activate Friends Page"), this);
    action->setIconText(i18n("Friends"));
    connect(action, SIGNAL(triggered()), this, SLOT(actionActivatePage()));
    actionCollection->addAction("activate_page_friends", action);

    deactivatePageActions();
}

void FriendPage::applyPreferences(KMLDonkeyPreferences*)
{
    if (KMLDonkey::App->listFont !=  friendView->font())
        friendView->setFont(KMLDonkey::App->listFont);

    QHash<int, InfoList *>::const_iterator it;
    for ( it = fileViews.begin(); it != fileViews.end(); ++it)
        if (KMLDonkey::App->listFont !=  it.value()->font())
            it.value()->setFont(KMLDonkey::App->listFont);
}

void FriendPage::clear()
{
    closeAllFileLists();

    qDeleteAll(friendList);
    friendList.clear();

    qDeleteAll(chats);
    chats.clear();

    qDeleteAll(shares);
    shares.clear();

    friendView->clear();
    m_removeAllAction->setEnabled(false);
}

void FriendPage::restoreState(KSharedConfigPtr conf)
{
    friendView->initialise();

    KConfigGroup group = conf->group( "FriendPage" );

    QList<int> hsplitter = group.readEntry( "HSplitter", QList<int>() );
    if( hsplitter.count() > 0 )
        friendHSplitter->setSizes(hsplitter);

    QList<int> vsplitter = group.readEntry( "VSplitter", QList<int>() );
    if( vsplitter.count() > 0 )
        friendVSplitter->setSizes(vsplitter);

    applyPreferences();
}

void FriendPage::saveState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "FriendPage" );
    group.writeEntry("FriendHSplitter", friendHSplitter->sizes());
    group.writeEntry("FriendVSplitter", friendVSplitter->sizes());

    friendView->saveLayout();

    QHash<int, InfoList *>::const_iterator it = fileViews.begin();
    if ( it != fileViews.end() )
        it.value()->saveLayout();
}

void FriendPage::closeAllFileLists()
{
    qDeleteAll( fileViews );
    fileViews.clear();
    pleaseUpdateShareList.clear();
    pleaseUpdateActions();
}

void FriendPage::openFileList(int client)
{
    InfoList* fileView = fileViews.value(client);
    if (!fileView) {
        ClientInfo* cl = KMLDonkey::App->donkey->findClientNo(client);
        fileView = new InfoList(fileTab, "fileView");
        fileView->setFont(KMLDonkey::App->listFont);
        fileView->addColumn( i18n( "Filename" ) );
        fileView->addColumn( i18n( "Network" ) );
        fileView->addColumn( i18n( "Size" ) );
        fileView->addColumn( i18n( "Format" ) );
        fileView->addColumn( i18n( "Comment" ) );
        fileView->addColumn( i18n( "Hash" ) );
        fileView->initialise();
        fileTab->addTab(fileView, QIcon::fromTheme("x-office-contact"), cl->clientName());
        fileViews.insert(client, fileView);
        constructFileList(client);
        connect(fileView, SIGNAL(contextMenu(QTreeWidgetItem*,const QPoint&)),
                this, SLOT(contextFile(QTreeWidgetItem*,const QPoint&)));
        connect(fileView, SIGNAL(itemSelectionChanged()), SLOT(pleaseUpdateActions()));
        connect(fileView, SIGNAL(gotFocus()), SLOT(pleaseUpdateActions()));
        connect(fileView, SIGNAL(itemActivated(QTreeWidgetItem*, int)),
                this, SLOT(fileDoubleClick(QTreeWidgetItem*)));
    }
    fileTab->setCurrentIndex( fileTab->indexOf(fileView) );
}

void FriendPage::closeFileList(QWidget* widget)
{
    InfoList* fileView = static_cast<InfoList*>(widget);
    QHash<int, InfoList *>::const_iterator it;
    for ( it = fileViews.begin(); it != fileViews.end(); ++it)
        if (it.value() == fileView) {
            it.value()->saveLayout();
            fileViews.remove( it.key() );
            delete fileView;
            pleaseUpdateActions();
            return;
        }
}

void FriendPage::friendUpdated(int client)
{
    ClientInfo* cl = KMLDonkey::App->donkey->findClientNo(client);
    if (!cl) return;

    ClientItem* it = friendList[client];
    if (it) {
        it->refresh();
        return;
    }
    it = new ClientItem(friendView, client);
    friendList.insert(client, it);
    if (! m_removeAllAction->isEnabled())
        m_removeAllAction->setEnabled(true);
}

void FriendPage::friendRemoved(int client)
{
    delete friendList.take(client);
    delete shares.take(client);
    delete fileViews.take(client);
    if (friendList.count() < 1)
        m_removeAllAction->setEnabled(false);
}

void FriendPage::contextFriend(QTreeWidgetItem*, const QPoint& pt)
{
    QMenu *pop = dynamic_cast<QMenu *>( (KMLDonkey::App->factory())->container("friend_actions", KMLDonkey::App) );
    if (!pop)
        KMLDonkey::App->showBadInstallDialog();
    else
        pop->popup(pt);
}

void FriendPage::contextFile(QTreeWidgetItem*, const QPoint& pt)
{
    QMenu *pop = dynamic_cast<QMenu *>( (KMLDonkey::App->factory())->container("friend_share_actions", KMLDonkey::App) );
    if (!pop)
        KMLDonkey::App->showBadInstallDialog();
    else
        pop->popup(pt);
}

void FriendPage::actionAddFriend()
{
    bool ok;
    QString name = KdeInputDialog::getText( this, i18n("Search for Friend"),
                                          i18n("Enter the name of the friend you are searching for:"),
                                          QLineEdit::Normal, QString(), &ok );
    if (ok)
        KMLDonkey::App->donkey->searchForFriend(name);
}

void FriendPage::actionRemoveFriend()
{
    QList<QTreeWidgetItem *> list = friendView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ClientItem* it = (ClientItem*) i;
        KMLDonkey::App->donkey->removeFriend(it->fileNo());
    }
}

void FriendPage::actionRemoveAllFriends()
{
    if (KMessageBox::warningTwoActions(this, i18n("Should all friends be removed?"),
                                  i18n("Remove all Friends"), KGuiItem(i18n("Remove")), KGuiItem(i18n("Cancel"))) != KMessageBox::PrimaryAction) return;
    KMLDonkey::App->donkey->removeAllFriends();
}

void FriendPage::actionConnectFriend()
{
    QList<QTreeWidgetItem *> list = friendView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ClientItem* it = (ClientItem*) i;
        KMLDonkey::App->donkey->connectFriend(it->fileNo());
    }
}

void FriendPage::actionOpenChat()
{
    QList<QTreeWidgetItem *> list = friendView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        ClientItem* it = (ClientItem*) i;
        ClientInfo* cl = KMLDonkey::App->donkey->findClientNo(it->fileNo());
        if (cl) {
            ChatWidget* chat = chats.value(cl->clientNo());
            if (!chat) {
                chat = new ChatWidget(cl->clientNo(), chatTab);
                chatTab->addTab(chat, QIcon::fromTheme("dialog-close"), cl->clientName());
                chats.insert(cl->clientNo(), chat);
                // Workaround for a strange rendering bug
                chatTab->hide();
                chatTab->show();
            } else {
                chatTab->setCurrentIndex( chatTab->indexOf( chat ) );
            }
        }
    }
}

void FriendPage::receiveShare(int clno, const QString& dir, int result)
{
    FriendShares* sh = shares.value(clno);
    if (!sh) {
        shares.insert(clno, new FriendShares());
        sh = shares[clno];
        QList<QTreeWidgetItem *> list = friendView->selectedItems();
        foreach( QTreeWidgetItem * i, list )
        {
            ClientItem* it = (ClientItem*) i;
            if (it->fileNo() == clno)
                it->setFilesListed(true);
        }
    }

    const ResultInfo* si = KMLDonkey::App->donkey->findClientFile(result);
    if (!si) { qDebug() << "No result #" << result << " available!"; return; }

    QString dirname = dir;
    if (dirname.isEmpty()) dirname = ".";
    QList<int>& files = sh->dirs[dirname];
    if (!files.contains(si->resultNo()))
        files.append(si->resultNo());
    if (fileViews.contains(clno))
        pleaseUpdateShareList.insert(clno, true);
}

void FriendPage::constructFileList(int clno)
{
    InfoList* fileView = fileViews.value(clno);
    if (!fileView)
        return;

    fileView->clear();
    FriendShares* sh = shares.value(clno);
    if (!sh) {
        DonkeyMessage out(DonkeyProtocol::GetClient_files);
        out.writeInt32(clno);
        KMLDonkey::App->donkey->sendMessage(out);
        return;
    }

    QMap<QString,QList<int> >::iterator it;
    for (it = sh->dirs.begin(); it != sh->dirs.end(); ++it) {
        QList<int>::iterator lit;
        for (lit = it.value().begin(); lit != it.value().end(); ++lit) {
            new ClientFile(fileView, *lit);
        }
    }
}

void FriendPage::openFriendFiles(QTreeWidgetItem* item, int)
{
	if( item) 
	{
		ClientItem* cl = (ClientItem*)item;
		openFileList(cl->fileNo());
	}
}

void FriendPage::mightUpdateShareList()
{
    QMap<int,bool>::Iterator it;
    for (it = pleaseUpdateShareList.begin(); it != pleaseUpdateShareList.end(); ++it)
        if (it.value())
            constructFileList(it.key());
    pleaseUpdateShareList.clear();
}

QList<QTreeWidgetItem *> FriendPage::currentFileListSelection()
{
    QList<QTreeWidgetItem *> list;
    InfoList* fileView = static_cast<InfoList*>(fileTab->currentWidget());
    if (!fileView)
        return list;
    list = fileView->selectedItems();
    return list;
}

void FriendPage::fileDoubleClick(QTreeWidgetItem*)
{
    downloadSelected(false, true);
}

void FriendPage::downloadSelected(bool force, bool ask)
{
    QList<QTreeWidgetItem *> list = currentFileListSelection();
    foreach( QTreeWidgetItem * i, list )
    {
        ClientFile* it = (ClientFile*) i;
        const ResultInfo* fi = KMLDonkey::App->donkey->findClientFile(it->fileNo());
        if (!fi)
            continue;
        QStringList names;
        if (ask || !fi->resultName().length()) {
            bool ok;
            QString filename = KdeInputDialog::getText( this, i18n("Download As"),
                                                      i18n("Choose a filename for the new download:"),
                                                      QLineEdit::Normal, fi->resultName(), &ok );
            if (!ok)
                continue;
            names.append(filename);
            if (filename != fi->resultName())
                names.append(fi->resultName());
        } else {
            names.append(fi->resultName());
        }
        KMLDonkey::App->donkey->startDownload(names, fi->resultNo(), force);
    }
}

void FriendPage::actionDownload()
{
    downloadSelected(false, false);
}

void FriendPage::actionForceDownload()
{
    downloadSelected(true, false);
}

void FriendPage::actionDownloadAs()
{
    downloadSelected(false, true);
}


void FriendPage::copyFileToClipboard(ClipFormat format)
{
    QStringList l;
    QList<QTreeWidgetItem *> list = currentFileListSelection();
    foreach( QTreeWidgetItem * i, list )
    {
        ClientFile* it = (ClientFile*) i;
        const ResultInfo* fi = KMLDonkey::App->donkey->findClientFile(it->fileNo());
        if (!fi)
            continue;
        l.append(fi->resultName());
        l.append(fi->resultUid("ed2k"));
        l.append(QString::number((long int)fi->resultSize()));
    }
    copyToClipboard(l, format);
}

void FriendPage::actionActivatePage()
{
    KMLDonkey::App->activatePage(this);
}

void FriendPage::messageReceived(int client, const QString& message)
{
    if (!chats.value(client)) {
        ClientInfo* cl = KMLDonkey::App->donkey->findClientNo(client);
        if (!cl) return;
        ChatWidget* chat = new ChatWidget(client, chatTab);
        chatTab->addTab(chat, QIcon::fromTheme("dialog-close"), cl->clientName());
        chats.insert(client, chat);
        // Workaround for a strange rendering bug
        chatTab->hide();
        chatTab->show();
        chat->messageFromClient(client, message);
    }
}

void FriendPage::closeChat(QWidget* widget)
{
    ChatWidget* chat = static_cast<ChatWidget*>(widget);
    QHash<int, ChatWidget *>::const_iterator it;
    for ( it = chats.begin(); it != chats.end(); ++it)
        if (it.value() == chat) {
            chats.remove(it.key());
            delete chat;
            // pleaseUpdateActions();
            return;
        }
}



ChatWidget::ChatWidget(int client, QWidget* parent)
    : QWidget(parent)
{
    clno = client;

    QVBoxLayout* layout = new QVBoxLayout(this);

    output = new QTextBrowser(this);
    layout->addWidget(output);

    input = new KLineEdit(this);
    layout->addWidget(input);

    connect(input, &QLineEdit::returnPressed, this, [this]{
        const QString text=input->text();
        input->completionObject()->addItem(text);
        messageEntered(text);
    });
    connect(KMLDonkey::App->donkey, SIGNAL(messageFromClient(int, const QString&)), SLOT(messageFromClient(int, const QString&)));
}

void ChatWidget::messageFromClient(int client, const QString& message)
{
    if (client != clno) return;
    const ClientInfo* cl = KMLDonkey::App->donkey->findClientNo(client);
    output->append(QString("<b>&lt;%1&gt;</b> %2\n")
                   .arg((cl ? cl->clientName() : QString::number(client)).toHtmlEscaped()).arg( message.toHtmlEscaped() ));
}

void ChatWidget::messageEntered(const QString& message)
{
    QString nick = KMLDonkey::App->donkey->getOption("client_name");
    if (nick.isEmpty()) nick = "MLDonkey";
    output->append( QString("%1 <b>&lt;%2&gt;</b>").arg(nick.toHtmlEscaped()).arg(message.toHtmlEscaped()) );
    KMLDonkey::App->donkey->sendPrivateMessage(clno, message);
    input->clear();
}
