#include <QMimeData>
#include <QTextCodec>
#include <QFontDatabase>
#include <KConfigGroup>
/* -*- c++ -*-
 *
 * kmldonkey.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006 Christian Muehlhaeuser <chris@chris.de>
 * Copyright (C) 2008,2009 Gioacchino Mazzurco <gmazzurco89@gmail.com>
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

#include "kmldonkey.h"
#include "infolist.h"
#include "prefs/prefs.h"
#include "fontselector.h"
#include "serverpage.h"
#include "downloadpage.h"
#include "friendpage.h"
#include "sharepage.h"
#include "statspage.h"
#include "consolepage.h"
#include "search.h"
#include "mlconfig.h"
#include "submitdialog.h"
#include "systemtray.h"
#include "hostselectaction.h"

#include <QLabel>
#include <QActionGroup>
#include <QList>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QModelIndex>
#include <QPainter>
#include <QClipboard>
#include <QCheckBox>
#include <QSignalMapper>
#include <QTimer>
#include <QToolTip>
#include <QFile>

#include <KSharedConfig>
#include <KLocalizedString>
#include <QDebug>
#include <ktoolbar.h>
#include <QMenuBar>
#include <QStatusBar>
#include <QStandardPaths>
#include <kshortcutsdialog.h>
#include <kconfig.h>
#include <QUrl>
#include <QIcon>
#include <kmessagebox.h>
#include <QSpinBox>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kcharsets.h>
#include <kedittoolbar.h>
#include <KStandardShortcut>
#include <QAction>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <QApplication>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <knotification.h>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <KGuiItem>
#include <QTabWidget>
#include <kpagewidget.h>
#include <kpagewidgetmodel.h>
#include <QIcon>
#include <kxmlguiwindow.h>
#include <QFileDialog>

#include <donkeyprotocol.h>
#include <hostmanager.h>
#include <hostdialog.h>
#include <donkeyhost.h>
#include <emulecollection.h>

#include <KCoreAddons>
#include <KShortcutsDialog>
#include <QDesktopServices>

#include "kmldview.h"

KMLDonkey* KMLDonkey::App = 0; // global access to the static instance-pointer

void enableActionList(QList<QAction *> list, bool enable)
{
    foreach( QAction * a, list )
        a->setEnabled(enable);
}

KMLDonkey::KMLDonkey() : KXmlGuiWindow(){
    setCaption("KMLDonkey");
    setObjectName("KMLDonkeyMainWindow");

    setXMLFile(QStringLiteral("kmldonkeyui.rc"));
    //setXMLFile( KStandardDirs::locateLocal( "data", "kmldonkey/kmldonkeyui.rc" ) );
    //setLocalXMLFile( KStandardDirs::locateLocal( "data", "kmldonkey/kmldonkeyui.rc" ) );

    App = this;
    showStatusbarLabels = false;
    persistentReconnect = true;
    reconnect = 0;
    prefs = 0;
    donkey = new DonkeyProtocol(false, this);
    hostManager = new HostManager(this);
    lastHost = hostManager->defaultHostName();
    m_currentPage = 0;
    coreVersion = i18nc("Core version string not (yet) available", "Not Available");

    viewCreator = new KmldView(this);

    connect(qApp, SIGNAL(aboutToQuit()), SLOT(finalise()));

    setAcceptDrops(true);
    setCentralWidget(buildInterface());
    setupActions();
    setStateConfigGroup(QStringLiteral("MainWindow"));
    setAutoSaveSettings();
    restoreState( KSharedConfig::openConfig() );

    tray = new SystemTray(this);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkReconnect()));
    timer->start(5000);

    connect(donkey, SIGNAL(signalConnected()), this, SLOT(coreConnected()));
    connect(donkey, SIGNAL(signalDisconnected(int)), this, SLOT(coreDisconnected(int)));
    connect(donkey, SIGNAL(clientStats(int64, int64, int64, int, int, int, int, int, int, int, QMap<int,int>*)),
            this, SLOT(updateStatus(int64, int64, int64, int, int, int, int, int, int, int, QMap<int,int>*)));
    connect(donkey, SIGNAL(coreVersion(const QString&)), this, SLOT(coreVersionReceived(const QString&)));

    connect(hostManager, SIGNAL(hostListUpdated()), SLOT(hostListUpdated()));

    if (!factory()->container("download_actions", this)) {
        showBadInstallDialog();
        deleteLater();
    }

    QTimer::singleShot(0, this, SLOT(connectToCore()));
}

KMLDonkey::~KMLDonkey(){}

void KMLDonkey::finalise()
{
    if (m_finalised) return;
    m_finalised = true;
    // Save while all pages still exist and before disconnect clears them.
    saveState(KSharedConfig::openConfig());
    viewCreator->saveStates();

    if (donkey->isConnected()) {
        if (searchPage)
            searchPage->stopAllSearches();
        donkey->disconnectFromCore(); // needed to get the sock.flush() and in general a nicer beaviour to "logout" correct
    }

}

bool KMLDonkey::queryExit()
{
    // We need to handle the isHidden()-state at queryExit() cause if Quit
    // was choosen from the trayicon-popupmenu then we've called the
    // functions in that order; queryExit(), hide() (yes, hide() is always
    // called here *grr*), finalise(), ...
    hideMainWin = isHidden();
    return true;
}

void KMLDonkey::show()
{
    QByteArray args;
    QDataStream stream(&args, QIODevice::WriteOnly);
    if (hideMainWin && (showTrayIcon || closeToTrayIcon)) { // eval the hideMainWin only if we're able to restore the mainwindow ;-)
        stream << false;
    }
    else {
        KMainWindow::show();
        stream << true;
    }
    hideMainWin = false;
    updateTrayIcon();
    //emitDCOPSignal("kmldonkeyAppeared(bool)", args);
}

void KMLDonkey::hide()
{
    KMainWindow::hide();
    updateTrayIcon();
    QByteArray args;
    QDataStream stream(&args, QIODevice::WriteOnly);
    stream << false;
    //emitDCOPSignal("kmldonkeyAppeared(bool)", args);
}

void KMLDonkey::addPage(KMLDonkeyPage* page, const QIcon& iconset, const QString& label)
{
    QWidget* p = dynamic_cast<QWidget*>(page);
    if (!p) return;
    KPageWidgetItem* item = pageModel->addPage(p, ""); //p->objectName());
    item->setName(label);
    item->setHeader(""); // hide the header
    item->setIcon(iconset);
    QModelIndex index = pageModel->index(item);
    Q_ASSERT(index.isValid());
    pages.append(page);
    page->plugGenericActions(this, SLOT(pageActionsChanged(KMLDonkeyPage*)));
}

void KMLDonkey::removePage(KMLDonkeyPage* page)
{
    QWidget* p = dynamic_cast<QWidget*>(page);
    if (!p) return;
    if( KPageWidgetItem* item = pageModel->item(pageModel->index(pages.indexOf(page), 0)) )
        pageModel->removePage(item);
    pages.removeAll(page);
}

void KMLDonkey::activatePage(KMLDonkeyPage* page)
{
    QWidget* p = dynamic_cast<QWidget*>(page);
    if (!p) return;
    if( KPageWidgetItem* item = pageModel->item(pageModel->index(pages.indexOf(page), 0)) )
        pageTab->setCurrentPage(item);
}

void KMLDonkey::activatePage(int page)
{
    if( KPageWidgetItem *item = pageModel->item( pageModel->index(page, 0) ) )
        pageTab->setCurrentPage(item);
}

void KMLDonkey::activateNextPage()
{
    QModelIndex index = pageModel->index( pageTab->currentPage() );
    if( index.isValid() )
        activatePage( index.row() + 1 );
}

void KMLDonkey::activatePreviousPage()
{
    QModelIndex index = pageModel->index( pageTab->currentPage() );
    if( index.isValid() )
        activatePage( index.row() - 1 );
}

QWidget* KMLDonkey::buildInterface()
{
    pageTab = new KPageWidget(this);
    pageTab->setObjectName("pages");
    pageTab->setFaceType( KPageView::Tabbed ); //Auto,Plain,List,Tree,Tabbed
    pageTab->layout()->setContentsMargins(0, 0, 0, 0);

    pageModel = new KPageWidgetModel(pageTab);
    pageTab->setModel(pageModel);

    serverPage = new ServerPage(pageTab);
    searchPage = new SearchPage(pageTab);
    downloadPage = new DownloadPage(pageTab);
    sharePage = new SharePage(pageTab);
    statsPage = new StatsPage(pageTab);
    friendPage = new FriendPage(pageTab);
    consolePage = new ConsolePage(pageTab);

    // Statusbar
    statInfo = new QLabel(this);
    statInfo->setObjectName("connectionStatus");
    statInfo->setTextFormat(Qt::PlainText);
    statServer = new QLabel(this);
    statFiles = new QLabel(this);
    statShare = new QLabel(this);
    statTransfer = new QLabel(this);
    statRate = new QLabel(this);
    statBandwidth = new QLabel(this);
    for (QLabel* label : {statServer, statFiles, statShare, statTransfer, statRate, statBandwidth}) {
        label->setContentsMargins(8, 2, 8, 2);
        label->setTextFormat(Qt::PlainText);
    }
    statInfo->setContentsMargins(8, 2, 8, 2);

    statServer->setToolTip(i18n("Server connected/total"));
    statFiles->setToolTip(i18n("Files downloading/finished"));
    statShare->setToolTip(i18n("Shared files total"));
    statTransfer->setToolTip(i18n("Transfer down/up"));
    statRate->setToolTip(i18n("Speed down/up"));
    statBandwidth->setToolTip(i18n("Bandwidth limit down/up"));

    statusBar()->addWidget(statInfo,1);
    statusBar()->addPermanentWidget(statServer);
    statusBar()->addPermanentWidget(statFiles);
    statusBar()->addPermanentWidget(statShare);
    statusBar()->addPermanentWidget(statTransfer);
    statusBar()->addPermanentWidget(statRate);
    statusBar()->addPermanentWidget(statBandwidth);

    // Interface done, we now need to restore some state that needs to be done before anything else is called.

    KSharedConfigPtr conf = KSharedConfig::openConfig();
    KConfigGroup stategroup = conf->group("State");

    addPage(serverPage, QIcon::fromTheme("network-server"), i18n("Servers"));
    addPage(searchPage, QIcon::fromTheme("system-search"), i18n("Search"));
    addPage(downloadPage, QIcon::fromTheme("go-down"), i18n("Downloads"));
    addPage(sharePage, QIcon::fromTheme("go-up"), i18n("Uploads"));
    addPage(friendPage, QIcon::fromTheme("system-users"), i18n("Friends"));
    addPage(statsPage, QIcon::fromTheme("view-statistics"), i18n("Statistics"));
    addPage(consolePage, QIcon::fromTheme("utilities-terminal"), i18n("Console"));

    connect(pageTab, &KPageView::currentPageChanged, this, &KMLDonkey::currentPageChanged);

    return pageTab;
}

void KMLDonkey::setupActions()
{
#if 0 //sebsauer
    KStandardAction::quit(qApp, SLOT(quit()), actionCollection());
#else
    actionCollection()->addAction("quit", KStandardAction::quit(qApp, SLOT(quit()), this));
#endif

    //m_toolbarAction = new dynamic_cast<KToggleAction*>( KStandardAction::create(KStandardAction::ShowToolbar, this, SLOT(optionsShowToolbar()), this) );
    auto* navigationGroup = new QActionGroup(this);
    auto* sidebarAction = new QAction(QIcon::fromTheme("view-list-details"), i18n("Sidebar Navigation"), this);
    auto* tabsAction = new QAction(QIcon::fromTheme("tab-new"), i18n("Tabbed Navigation"), this);
    for (QAction* navigation : {sidebarAction, tabsAction}) {
        navigation->setCheckable(true);
        navigationGroup->addAction(navigation);
    }
    actionCollection()->addAction("navigation_sidebar", sidebarAction);
    actionCollection()->addAction("navigation_tabs", tabsAction);
    connect(sidebarAction, &QAction::triggered, this, [this] { pageTab->setFaceType(KPageView::List); });
    connect(tabsAction, &QAction::triggered, this, [this] { pageTab->setFaceType(KPageView::Tabbed); });

    m_toolbarAction = new KToggleAction(i18n( "&Show Toolbar" ), this);
    actionCollection()->addAction("toolBar", m_toolbarAction);
    connect( m_toolbarAction, SIGNAL( toggled( bool ) ), SLOT( optionsShowToolbar() ) );

    m_statusbarAction = KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), this);
    actionCollection()->addAction("statusBar", m_statusbarAction);

    actionCollection()->addAction("configure_keyBindings", KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), this));
    actionCollection()->addAction("configure_Toolbars", KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), this));
    actionCollection()->addAction("configure_kmldonkey", KStandardAction::preferences(this, SLOT(optionsPreferences()), this));
    actionCollection()->addAction("configure_Notifications", KStandardAction::configureNotifications(this, SLOT(optionsNotifications()), this));

    QAction* action = new QAction(QIcon::fromTheme("network-server"), i18n("Connections..."), this);
    action->setIconText(i18n("Connections"));
    actionCollection()->addAction("configure_connection", action);
    connect(action, SIGNAL(triggered()), this, SLOT(optionsConfigureConnection()));

    m_connectAction = new HostSelectAction(i18n("&Connect to Core"), "network-connect", hostManager, this);
    m_connectAction->setIconText(i18n("Connect"));
    actionCollection()->addAction("connect_core", m_connectAction);
    connect(m_connectAction, SIGNAL(triggered()), SLOT(actionConnectCore()));
    connect(m_connectAction, SIGNAL(hostSelected(const QString&)), SLOT(actionConnectCore(const QString&)));

    action = new QAction(/*QIcon::fromTheme("network-bandwidth"),*/ i18n("Toggle Band&width"), this);
    action->setIconText(i18n("Toggle Bandwidth"));
    actionCollection()->addAction("toggle_bandwidth", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionToggleBandwidth()));
    addCoreAction(action);
	
    action = new QAction(QIcon::fromTheme("network-disconnect"), i18n("&Disconnect From Core"), this);
    action->setIconText(i18n("Disconnect"));
    actionCollection()->addAction("disconnect_core", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionDisconnectCore()));
    addCoreAction(action);

    action = new QAction(QIcon::fromTheme("process-stop"), i18n("&Kill Core"), this);
    action->setIconText(i18n("Kill"));
    actionCollection()->addAction("kill_core", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionKillCore()));
    addCoreAction(action);

    action = new QAction(QIcon::fromTheme("document-open-remote"), i18n("&Submit URLs..."), this);
    action->setIconText(i18n("Submit"));
    actionCollection()->addAction("submit_url", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionSubmitUrl()));
    addCoreAction(action);


    action = new QAction(QIcon::fromTheme("document-open"), i18n("Submit Collection File..."), this);
    action->setIconText(i18n("Collection"));
    actionCollection()->addAction("submit_collection", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionSubmitCollection()));
    addCoreAction(action);

    action = new QAction(QIcon::fromTheme("document-open"), i18n("Submit Torrent File..."), this);
    action->setIconText(i18n("Torrent"));
    actionCollection()->addAction("submit_torrent", action);
    connect(action, SIGNAL(triggered()), this, SLOT(actionSubmitTorrent()));
    addCoreAction(action);

    action = new QAction(QIcon::fromTheme("mldonkey"), i18n("Configure MLDonkey..."), this);
    actionCollection()->addAction("configure_mldonkey", action);
    connect(action, SIGNAL(triggered()), this, SLOT(optionsConfigureMLDonkey()));
    addCoreAction(action);

    pageMapper = new QSignalMapper(this);
    connect(pageMapper, SIGNAL(mappedInt(int)), SLOT(activatePage(int)));
    for (int i=1; i<10; i++) {
        action = new QAction(i18n("Activate Page %1",i), this);
        actionCollection()->setDefaultShortcut(action, QKeySequence(QString("Ctrl+%1").arg(i)));
        connect(action, SIGNAL(triggered()), pageMapper, SLOT(map()));
        pageMapper->setMapping(action, i - 1);
    }

    action = new QAction(QIcon::fromTheme("go-next"), i18n("Next Page"), this);
    action->setIconText(i18n("Next"));
    actionCollection()->setDefaultShortcuts(action, KStandardShortcut::tabNext());
    actionCollection()->addAction("activate_next_page", action);
    connect(action, SIGNAL(triggered()), this, SLOT(activateNextPage()));

    action = new QAction(QIcon::fromTheme("go-previous"), i18n("Previous Page"), this);
    action->setIconText(i18n("Prev"));
    actionCollection()->setDefaultShortcuts(action, KStandardShortcut::tabPrev());
    actionCollection()->addAction("activate_previous_page", action);
    connect(action, SIGNAL(triggered()), this, SLOT(activatePreviousPage()));

    actionMapper = new QSignalMapper(this);
    connect(actionMapper, SIGNAL(mappedString(const QString&)), SLOT(handleGenericAction(const QString&)));

    QAction* copy = KStandardAction::copy(0, 0, this);
    copy->setText(i18n("Copy to Clipboard as &URL"));
    copy->setIconText(i18n("Copy"));
    addGenericAction("copy_url", copy);

    addGenericAction("copy_html", i18n("Copy to Clipboard as HTML"), i18n("Copy"), "edit-copy", QKeySequence());
    addGenericAction("copy_hash", i18n("Copy to Clipboard as Hash"), i18n("Copy"), "edit-copy", QKeySequence());
    addGenericAction("search_download", i18n("Download"), i18n("Download"), "go-down", QKeySequence());
    addGenericAction("search_force_download", i18n("Force Download"), i18n("Force"), "go-bottom", QKeySequence());
    addGenericAction("search_download_as", i18n("Download As..."), i18n("Download"), "go-down", QKeySequence());
    addGenericAction("file_information", i18n("File Information..."), i18n("Info"), "document-properties", QKeySequence());

    foreach( KMLDonkeyPage * page, pages )
        page->setupActions( actionCollection() );

    enableActionList(coreActions, false);
    createGUI();
}

bool KMLDonkey::sortDownloadingFirst()
{
    return downloadPage ? downloadPage->sortDownloadingFirstAction->isChecked() : false;
}

bool KMLDonkey::sortPausedLast()
{
    return downloadPage ? downloadPage->sortPausedLastAction->isChecked() : false;
}

void KMLDonkey::addGenericAction(const char* name, QAction* action)
{
    genericActions[name] = action;
    actionCollection()->addAction(name, action);
    connect(action, SIGNAL(triggered()), actionMapper, SLOT(map()));
    actionMapper->setMapping(action, name);
}

void KMLDonkey::addGenericAction(const char* name, const QString& label, const QString& icontext, const QString& icon, const QKeySequence& shortcut)
{
    QAction* action = new QAction(QIcon::fromTheme(icon), label, this);
    action->setObjectName(name);
    action->setIconText(icontext);
    actionCollection()->setDefaultShortcut(action, shortcut);
    addGenericAction(name, action);
}

void KMLDonkey::handleGenericAction(const QString& action)
{
    m_currentPage->handleGenericAction(action);
}

void KMLDonkey::updateSupportedActions(KMLDonkeyPage* page)
{
	QStringList supported;
    if (page)
        supported = page->supportedGenericActions();
		
	QMap<QString, QAction *>::const_iterator it = genericActions.begin();
	while( it != genericActions.end() ) 
	{
		it.value()->setEnabled( supported.contains( it.key()) );
		it++;
	}
}

void KMLDonkey::updateTrayIcon()
{
    if (showTrayIcon || (isHidden() && closeToTrayIcon)) {
#if 0 //port?
        if (!tray->isVisible())
#endif
        tray->show();
    }
    else {
#if 0 //port?
        if (tray->isVisible())
#endif
        tray->hide();
    }
}

void KMLDonkey::pageActionsChanged(KMLDonkeyPage* newPage)
{
    if (m_currentPage != newPage) return;
    updateSupportedActions(m_currentPage);
}

void KMLDonkey::currentPageChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    KPageWidgetItem *item = pageModel->item(current);
    KMLDonkeyPage* page = dynamic_cast<KMLDonkeyPage*>( item ? item->widget() : 0 );
    if (!page) return;
    if (m_currentPage && m_currentPage != page)
        m_currentPage->deactivatePageActions();
    m_currentPage = page;
    updateSupportedActions(page);
}

void KMLDonkey::addCoreAction(QAction* action)
{
    coreActions.append(action);
}

void KMLDonkey::restoreState(KSharedConfigPtr conf)
{
    KConfigGroup stategroup = conf->group("State");
    QModelIndex index = pageModel->index( pageTab->currentPage() );
    int currentPage = stategroup.readEntry("CurrentPage", index.isValid() ? index.row() : -1);

    KConfigGroup optionsgroup = conf->group("Options");
    m_toolbarAction->setChecked( optionsgroup.readEntry("ShowToolbar", true) );
    emit optionsShowToolbar();
    m_statusbarAction->setChecked( optionsgroup.readEntry("ShowStatusbar", true) );
    emit optionsShowStatusbar();
    showStatusbarLabels = optionsgroup.readEntry("ShowStatusbarLabels", showStatusbarLabels);
    showTrayIcon = optionsgroup.readEntry("ShowTrayIcon", false);
    closeToTrayIcon = optionsgroup.readEntry("CloseToTrayIcon", false);

    // Adopt the refreshed navigation once, then preserve the chosen layout.
    const bool currentInterface = optionsgroup.readEntry("InterfaceVersion", 0) >= 2;
    const auto navigation = currentInterface
        ? static_cast<KPageView::FaceType>(optionsgroup.readEntry("PageViewMode", int(KPageView::Tabbed)))
        : KPageView::Tabbed;
    pageTab->setFaceType(navigation);
    actionCollection()->action("navigation_sidebar")->setChecked(navigation != KPageView::Tabbed);
    actionCollection()->action("navigation_tabs")->setChecked(navigation == KPageView::Tabbed);

    hideMainWin = optionsgroup.readEntry("HideMainWin", false);
    displayProgressbar = optionsgroup.readEntry("displayProgressbar", true);
    persistentReconnect = optionsgroup.readEntry("persistentReconnect", persistentReconnect);
    advancedConfig = optionsgroup.readEntry("AdvancedConfig", false);
    coreCharset = optionsgroup.readEntry("CoreCharset", "iso 8859-1");
    DonkeyMessage::setStringCodec(QTextCodec::codecForName(coreCharset.toLatin1()));

    KConfigGroup fontsgroup = conf->group("Fonts");
    consoleFont = fontsgroup.readEntry("ConsoleFont", QFontDatabase::systemFont(QFontDatabase::FixedFont));
    listFont = fontsgroup.readEntry("ListFont", QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    KConfigGroup colorsgroup = conf->group("Colors");
    coloredViews = colorsgroup.readEntry("coloredViews", true);

    colorServerNotConnected = colorsgroup.readEntry("NotConnected", QColor(0x99, 0x00, 0x00));
    colorServerBlacklisted = colorsgroup.readEntry("Blacklisted", QColor(0xAA, 0xAA, 0xAA));
    colorServerConnecting = colorsgroup.readEntry("Connecting", QColor(0x00, 0x00, 0x80));
    colorServerConnected = colorsgroup.readEntry("Connected", QColor(0x15, 0x70, 0x15));

    colorSearchFewSources = colorsgroup.readEntry("SearchFewSources", QColor(0x99, 0x00, 0x00));
    colorSearchManySources = colorsgroup.readEntry("SearchManySources", QColor(0x15, 0x70, 0x15));
    colorSearchAlreadyDone = colorsgroup.readEntry("SearchAlreadyDone", QColor(0xAA, 0xAA, 0xAA));
    searchThreshold = colorsgroup.readEntry("SearchThreshold", 5);

    colorDownloadDownloading = colorsgroup.readEntry("Downloading", QColor(0x15, 0x70, 0x15));
    colorDownloadPaused = colorsgroup.readEntry("Paused", QColor(0xAA, 0xAA, 0xAA));
    colorDownloadLooking = colorsgroup.readEntry("Looking", QColor(0x99, 0x00, 0x00));
    colorDownloadNotAvailable = colorsgroup.readEntry("NotAvailable", QColor(0xFF, 0x00, 0x00));
    colorDownloadQueued = colorsgroup.readEntry("Queued", QColor(0x00, 0x00, 0x00));

    colorNoSources  = colorsgroup.readEntry("NoSources", QColor(Qt::red));
    colorComplete = colorsgroup.readEntry("Complete", QColor(Qt::green));
    colorFewSources = colorsgroup.readEntry("FewSources", QColor::fromHsv(240, 255, 128));
    colorManySources = colorsgroup.readEntry("ManySources", QColor::fromHsv(240, 128, 255));

    availabilityThreshold = colorsgroup.readEntry("Threshold", 25);
    availabilityShadingDepth = colorsgroup.readEntry("ShadingDepth", 250);
    availabilityShading = colorsgroup.readEntry("Shading", true);

    colorSourceNotConnected = colorsgroup.readEntry("SourceNotConnected", QColor(0x99, 0x00, 0x00));
    colorSourceBlacklisted = colorsgroup.readEntry("SourceBlacklisted", QColor(0xAA, 0xAA, 0xAA));
    colorSourceConnecting = colorsgroup.readEntry("SourceConnecting", QColor(0x00, 0x00, 0x80));
    colorSourceQueued = colorsgroup.readEntry("SourceQueued", QColor(0xA0, 0x64, 0x09));
    colorSourceDownloading = colorsgroup.readEntry("SourceDownloading", QColor(0x15, 0x70, 0x15));

    // Pages
    foreach( KMLDonkeyPage * page, pages )
        page->restoreState( conf );

    KConfigGroup shortcuts = conf->group("ActionCollection");
    actionCollection()->readSettings(&shortcuts);
    activatePage(currentPage);
}

void KMLDonkey::saveState(KSharedConfigPtr conf)
{
    KConfigGroup windowGroup = conf->group("MainWindow");
    saveMainWindowSettings(windowGroup);
    KSharedConfig::openStateConfig()->sync();
    KConfigGroup stategroup = conf->group("State");
    stategroup.writeEntry("FirstRun", false);

    QModelIndex index = pageModel->index( pageTab->currentPage() );
    stategroup.writeEntry("CurrentPage", index.isValid() ? index.row() : -1);

    KConfigGroup optionsgroup = conf->group("Options");
    optionsgroup.writeEntry("ShowToolbar", m_toolbarAction->isChecked());
    optionsgroup.writeEntry("ShowStatusbar", m_statusbarAction->isChecked());
    optionsgroup.writeEntry("ShowStatusbarLabels", showStatusbarLabels);
    optionsgroup.writeEntry("ShowTrayIcon", showTrayIcon);
    optionsgroup.writeEntry("CloseToTrayIcon", closeToTrayIcon);
    optionsgroup.writeEntry("InterfaceVersion", 2);
    optionsgroup.writeEntry("PageViewMode", (int)pageTab->faceType());
    optionsgroup.writeEntry("HideMainWin", hideMainWin);
    optionsgroup.writeEntry("displayProgressbar", displayProgressbar);
    optionsgroup.writeEntry("persistentReconnect", persistentReconnect);
    optionsgroup.writeEntry("AdvancedConfig", advancedConfig);
    optionsgroup.writeEntry("CoreCharset", coreCharset);
	
    // Pages
    foreach( KMLDonkeyPage * page, pages )
        page->saveState(conf);

    KConfigGroup shortcuts = conf->group("ActionCollection");
    actionCollection()->writeSettings(&shortcuts);
    conf->sync();
}

void KMLDonkey::actionConnectCore()
{
    reconnect = 0;
    connectToCore();
}

void KMLDonkey::actionConnectCore(const QString& host)
{
    if (hostManager->validHostName(host)) {
        lastHost = host;
        reconnect = 0;
        connectToCore();
    }
}

void KMLDonkey::actionDisconnectCore()
{
    reconnect = 0;
    disconnectFromCore();
}

void KMLDonkey::actionKillCore()
{
    if (KMessageBox::warningTwoActions(this,
                                    i18n("Really kill the MLDonkey-Core?\n\nBe warned, that you cannot restart the core with KMLDonkey."),
                                    i18n("Kill Core"),
                                    KGuiItem(i18n("Kill Core")),
                                    KGuiItem(i18n("Cancel")),
                                    "KillCoreConfirm" ) == KMessageBox::PrimaryAction)
        KMLDonkey::App->donkey->killCore();
}

void KMLDonkey::actionSubmitUrl()
{
    SubmitDialog *dlg = findChild<SubmitDialog*>();
    if (! dlg)
        dlg = new SubmitDialog(this);
    dlg->show();
}

void KMLDonkey::actionSubmitCollection()
{
    QString url = QFileDialog::getOpenFileName(this, i18n("Open Collection File"), QString(), "Collections (*.emulecollection);;All files (*)");
    if (url.isEmpty())
        return;
    EmuleCollection my_collection;
    if (my_collection.Open(url.toLocal8Bit().data())){
	    QStringList urls;
	    for(uint i = 0; i < my_collection.GetFileCount(); i++){
		    urls << QString::fromLocal8Bit(my_collection.GetEd2kLink(i).c_str());
	    }
	    SubmitDialog *dlg = findChild<SubmitDialog*>();
	    if (! dlg) dlg = new SubmitDialog(this);
	    dlg->show();
	    dlg->setText(urls.join("\n"));
    }
    else KMessageBox::error(this, i18n("Failed to open collection file %1.", url), QString(), KMessageBox::PlainText);
    
}

void KMLDonkey::actionSubmitTorrent(){
    qDebug() << "KMLDonkey::actionSubmitTorrent() called";
    QString filePath = QFileDialog::getOpenFileName(this, i18n("Open Torrent File"), QString(), "Torrents (*.torrent);;All files (*)");
    qDebug() << filePath;
    if (filePath.isEmpty()) return;
        
    donkey->submitUrl( QUrl::fromLocalFile(filePath) );
}


bool KMLDonkey::queryClose()
{
    if (closeToTrayIcon && QObject::sender() == 0) {
        hide();
        return false;
    }
    // KMainWindow may delete this window before aboutToQuit is emitted.
    finalise();
    return true;
}

void KMLDonkey::dragEnterEvent(QDragEnterEvent* event)
{
    event->setAccepted( (event->mimeData()->hasUrls() || event->mimeData()->text().startsWith("ed2k:")) );
}

void KMLDonkey::dropEvent(QDropEvent* event)
{
    if( !(event->mimeData()->hasUrls() || event->mimeData()->text().startsWith("ed2k:")) )
        return;

    if (event->mimeData()->text().startsWith("ed2k:")) {
        donkey->submitUrl(event->mimeData()->text().trimmed());
        return;
    }
    foreach ( const QUrl &url, event->mimeData()->urls() ) {
        donkey->submitUrl(url);
    }
}

void KMLDonkey::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void KMLDonkey::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void KMLDonkey::optionsConfigureKeys()
{
    KShortcutsDialog dialog(this);
    dialog.addCollection(actionCollection());
    dialog.configure();
}

void KMLDonkey::optionsConfigureToolbars()
{
    KConfigGroup cg( KSharedConfig::openConfig(), "MainWindow");
    saveMainWindowSettings( cg );

    KEditToolBar dlg( actionCollection() );
    connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(newToolbarConfig()));
    dlg.exec();
}

void KMLDonkey::newToolbarConfig()
{
    // ...if you use any action list, use plugActionList on each here...
    //createGUI();
    //createGUI(); //createGUI();

    KConfigGroup cg( KSharedConfig::openConfig(), "MainWindow");
    applyMainWindowSettings( cg );

    createGUI(); //createGUI();
}

void KMLDonkey::optionsPreferences()
{
   if (! prefs) { // on first access create the KMLDonkeyPreferences instance
        prefs = new KMLDonkeyPreferences(this);
        connect(prefs, SIGNAL(applyClicked()), this, SLOT(applyPreferences()));
        connect(prefs, SIGNAL(okClicked()), this, SLOT(applyPreferences()));
    }

    prefs->generalPage->showToolbarCheckbox->setChecked( m_toolbarAction->isChecked() );
    prefs->generalPage->showStatusbarCheckbox->setChecked( m_statusbarAction->isChecked() );
    prefs->generalPage->statusbarLabelsCheckbox->setChecked( showStatusbarLabels );

    switch( pageTab->faceType() ) {
        case KPageView::List: prefs->generalPage->pageViewModeComboBox->setCurrentIndex(0); break;
        case KPageView::Tree: prefs->generalPage->pageViewModeComboBox->setCurrentIndex(1); break;
        case KPageView::Tabbed: prefs->generalPage->pageViewModeComboBox->setCurrentIndex(2); break;
        default: break;
    }

    prefs->generalPage->persistentReconnectCheckbox->setChecked( persistentReconnect );
    prefs->generalPage->advancedConfigCheckbox->setChecked( advancedConfig );

    QComboBox *selector = prefs->generalPage->encodingSelector;
    for (int i = 0; i < selector->count(); i++) 
	{
		QString charSet = selector->itemText(i);
		if( charSet.isEmpty() == false 
				&& coreCharset == KCharsets::charsets()->encodingForName( charSet ) ) {
            selector->setCurrentIndex(i);
            break;
        }
    }

    prefs->generalPage->showTrayiconCheckBox->setChecked( showTrayIcon );
    prefs->generalPage->showTrayOnCloseCheckBox->setChecked( closeToTrayIcon );

    prefs->listsPage->progressbarCheckBox->setChecked( displayProgressbar );

    prefs->colorPage->coloredListsCheckbox->setChecked(coloredViews);
    emit prefs->colorPage->coloredLists(coloredViews);

    prefs->colorPage->colorServerNotConnected->setColor(colorServerNotConnected);
    prefs->colorPage->colorServerBlacklisted->setColor(colorServerBlacklisted);
    prefs->colorPage->colorServerConnecting->setColor(colorServerConnecting);
    prefs->colorPage->colorServerConnected->setColor(colorServerConnected);

    prefs->colorPage->colorSearchFewSources->setColor(colorSearchFewSources);
    prefs->colorPage->colorSearchManySources->setColor(colorSearchManySources);
    prefs->colorPage->colorSearchAlreadyDone->setColor(colorSearchAlreadyDone);
    prefs->colorPage->searchSourcesThresholdSlider->setValue(searchThreshold);

    prefs->colorPage->downloadingColorSelect->setColor(colorDownloadDownloading);
    prefs->colorPage->pausedColorSelect->setColor(colorDownloadPaused);
    prefs->colorPage->lookingColorSelect->setColor(colorDownloadLooking);
    prefs->colorPage->notavailableColorSelect->setColor(colorDownloadNotAvailable);
    prefs->colorPage->queuedColorSelect->setColor(colorDownloadQueued);

    prefs->colorPage->noSourcesColorSelect->setColor(colorNoSources);
    prefs->colorPage->fewSourcesColorSelect->setColor(colorFewSources);
    prefs->colorPage->manySourcesColorSelect->setColor(colorManySources);
    prefs->colorPage->completeColorSelect->setColor(colorComplete);

    prefs->colorPage->availabilityThresholdSlider->setValue(availabilityThreshold);
    prefs->colorPage->availabilityShadingSlider->setValue(availabilityShadingDepth);
    prefs->colorPage->availabilityShadingCheckbox->setChecked(availabilityShading);

    prefs->colorPage->colorSourceNotConnected->setColor(colorSourceNotConnected);
    prefs->colorPage->colorSourceBlacklisted->setColor(colorSourceBlacklisted);
    prefs->colorPage->colorSourceConnecting->setColor(colorSourceConnecting);
    prefs->colorPage->colorSourceQueued->setColor(colorSourceQueued);
    prefs->colorPage->colorSourceDownloading->setColor(colorSourceDownloading);

    prefs->fontPage->consoleFontSelect->setFont(consoleFont);
    prefs->fontPage->listFontSelect->setFont(listFont);

    foreach( KMLDonkeyPage * page, pages )
        page->configurePrefsDialog(prefs);

    prefs->exec();
}

void KMLDonkey::applyPreferences()
{
    if (! prefs) return; // just to be sure ;-)

    m_toolbarAction->setChecked( prefs->generalPage->showToolbarCheckbox->isChecked() );
    emit optionsShowToolbar();
    m_statusbarAction->setChecked( prefs->generalPage->showStatusbarCheckbox->isChecked() );
    emit optionsShowStatusbar();
    showStatusbarLabels = prefs->generalPage->statusbarLabelsCheckbox->isChecked();
    persistentReconnect = prefs->generalPage->persistentReconnectCheckbox->isChecked();
    advancedConfig = prefs->generalPage->advancedConfigCheckbox->isChecked();

    if (prefs->generalPage->showTrayiconCheckBox->isChecked() != showTrayIcon ||
        prefs->generalPage->showTrayOnCloseCheckBox->isChecked() != closeToTrayIcon) {
            closeToTrayIcon = prefs->generalPage->showTrayOnCloseCheckBox->isChecked();
            showTrayIcon = prefs->generalPage->showTrayiconCheckBox->isChecked();
            updateTrayIcon();
    }

    KPageView::FaceType faceType = pageTab->faceType();
    QModelIndex index = pageModel->index( pageTab->currentPage() );
    int currentIndex = index.isValid() ? index.row() : -1;
    switch( prefs->generalPage->pageViewModeComboBox->currentIndex() ) {
        case 0: faceType = KPageView::List; break;
        case 1: faceType = KPageView::Tree; break;
        case 2: faceType = KPageView::Tabbed; break;
        default: break;
    }
    if( pageTab->faceType() != faceType )
        pageTab->setFaceType(faceType);
    activatePage(currentIndex);

    QString newCharset = KCharsets::charsets()->encodingForName(prefs->generalPage->encodingSelector->currentText());
    if (newCharset != coreCharset) {
        coreCharset = newCharset;
        DonkeyMessage::setStringCodec(QTextCodec::codecForName(coreCharset.toLatin1()));
        connectToCore();
    }

    displayProgressbar = prefs->listsPage->progressbarCheckBox->isChecked();

    KSharedConfigPtr conf = KSharedConfig::openConfig();

    {
        KConfigGroup colorsgroup = conf->group("Colors");

        coloredViews = prefs->colorPage->coloredListsCheckbox->isChecked();
        colorsgroup.writeEntry("coloredViews", coloredViews);

        colorServerNotConnected = prefs->colorPage->colorServerNotConnected->color();
        colorServerBlacklisted = prefs->colorPage->colorServerBlacklisted->color();
        colorServerConnecting = prefs->colorPage->colorServerConnecting->color();
        colorServerConnected = prefs->colorPage->colorServerConnected->color();
        colorsgroup.writeEntry("NotConnected", colorServerNotConnected);
        colorsgroup.writeEntry("Blacklisted", colorServerBlacklisted);
        colorsgroup.writeEntry("Connecting", colorServerConnecting);
        colorsgroup.writeEntry("Connected", colorServerConnected);

        colorSearchFewSources = prefs->colorPage->colorSearchFewSources->color();
        colorSearchManySources = prefs->colorPage->colorSearchManySources->color();
        colorSearchAlreadyDone = prefs->colorPage->colorSearchAlreadyDone->color();
        searchThreshold = prefs->colorPage->searchSourcesThresholdSlider->value();
        colorsgroup.writeEntry("SearchFewSources", colorSearchFewSources);
        colorsgroup.writeEntry("SearchManySources", colorSearchManySources);
        colorsgroup.writeEntry("SearchAlreadyDone", colorSearchAlreadyDone);
        colorsgroup.writeEntry("SearchThreshold", searchThreshold);

        colorDownloadDownloading = prefs->colorPage->downloadingColorSelect->color();
        colorDownloadPaused = prefs->colorPage->pausedColorSelect->color();
        colorDownloadLooking = prefs->colorPage->lookingColorSelect->color();
        colorDownloadNotAvailable = prefs->colorPage->notavailableColorSelect->color();
        colorDownloadQueued = prefs->colorPage->queuedColorSelect->color();
        colorsgroup.writeEntry("Downloading", colorDownloadDownloading);
        colorsgroup.writeEntry("Paused", colorDownloadPaused);
        colorsgroup.writeEntry("Looking", colorDownloadLooking);
        colorsgroup.writeEntry("NotAvailable", colorDownloadNotAvailable);
        colorsgroup.writeEntry("Queued", colorDownloadQueued);

        colorNoSources = prefs->colorPage->noSourcesColorSelect->color();
        colorFewSources = prefs->colorPage->fewSourcesColorSelect->color();
        colorManySources = prefs->colorPage->manySourcesColorSelect->color();
        colorComplete = prefs->colorPage->completeColorSelect->color();

        availabilityThreshold = prefs->colorPage->availabilityThresholdSlider->value();
        availabilityShadingDepth = prefs->colorPage->availabilityShadingSlider->value();
        availabilityShading = prefs->colorPage->availabilityShadingCheckbox->isChecked();
        colorsgroup.writeEntry("NoSources", colorNoSources);
        colorsgroup.writeEntry("FewSources", colorFewSources);
        colorsgroup.writeEntry("ManySources", colorManySources);
        colorsgroup.writeEntry("Complete", colorComplete);
        colorsgroup.writeEntry("Threshold", availabilityThreshold);
        colorsgroup.writeEntry("ShadingDepth", availabilityShadingDepth);
        colorsgroup.writeEntry("Shading", availabilityShading);

        colorSourceNotConnected = prefs->colorPage->colorSourceNotConnected->color();
        colorSourceBlacklisted = prefs->colorPage->colorSourceBlacklisted->color();
        colorSourceConnecting = prefs->colorPage->colorSourceConnecting->color();
        colorSourceQueued = prefs->colorPage->colorSourceQueued->color();
        colorSourceDownloading = prefs->colorPage->colorSourceDownloading->color();
        colorsgroup.writeEntry("SourceNotConnected", colorSourceNotConnected);
        colorsgroup.writeEntry("SourceBlacklisted", colorSourceBlacklisted);
        colorsgroup.writeEntry("SourceConnecting", colorSourceConnecting);
        colorsgroup.writeEntry("SourceQueued", colorSourceQueued);
        colorsgroup.writeEntry("SourceDownloading", colorSourceDownloading);
    }

    {
        KConfigGroup fontsgroup = conf->group("Fonts");

        listFont = prefs->fontPage->listFontSelect->font();
        consoleFont = prefs->fontPage->consoleFontSelect->font();

        fontsgroup.writeEntry("ConsoleFont", consoleFont);
        fontsgroup.writeEntry("ListFont", listFont);
    }

    foreach( KMLDonkeyPage * page, pages )
        page->applyPreferences(prefs);

    saveState( conf ); // finally save everything
}

void KMLDonkey::optionsConfigureConnection()
{
    HostDialog::showDialog(this, hostManager, donkey);
}

void KMLDonkey::optionsConfigureMLDonkey()
{
    if (!donkey->isConnected()) {
        KMessageBox::error(this, i18n("You must connect to a running MLDonkey core before you can configure it."));
        return;
    }
    MLDonkeyConfigDialog cfg(this);
    cfg.exec();
}

void KMLDonkey::optionsNotifications()
{
    PortedDialog dialog(this);
    dialog.setWindowTitle(i18n("Notifications"));
    auto* layout = new QVBoxLayout(dialog.mainWidget());
    KConfig config("kmldonkey.notifyrc", KConfig::NoGlobals);
    const QStringList events = { "KMLDonkeyConnected", "KMLDonkeyDisconnected", "KMLDonkeyDownloadFinished",
        "KMLDonkeyDownloadAdded", "KMLDonkeyToggleBandwidth" };
    const QStringList labels = { i18n("Connected"), i18n("Disconnected"), i18n("Download finished"),
        i18n("Download added"), i18n("Bandwidth changed") };
    QList<QCheckBox*> checks;
    for (int i = 0; i < events.size(); ++i) {
        auto* check = new QCheckBox(labels[i], dialog.mainWidget());
        check->setChecked(config.group("Event/" + events[i]).readEntry("Action", QString("Popup")).contains("Popup"));
        layout->addWidget(check);
        checks.append(check);
    }
    if (dialog.exec() == QDialog::Accepted)
        for (int i = 0; i < events.size(); ++i)
            config.group("Event/" + events[i]).writeEntry("Action", checks[i]->isChecked() ? "Popup" : "None");
}

void KMLDonkey::changeStatusbar(const QString& text)
{
    // display the text on the statusbar
    statInfo->setText(text);
	statInfo->setToolTip( QString() );
}

void KMLDonkey::changeCaption(const QString& text)
{
    // display the text on the caption
    setCaption(text);
}

void KMLDonkey::hostListUpdated()
{
    if (!hostManager->validHostName(lastHost))
        lastHost = hostManager->defaultHostName();
    connectToCore();
}

void KMLDonkey::connectToCore()
{
    HostInterface* host = hostManager->hostProperties(lastHost);
    donkey->setHost(host);
    donkey->connectToCore();
    changeStatusbar( i18n("Connecting with %1 ...", QString("%1 (%2@%3:%4)").arg(host->name()).arg(donkey->username()).arg(host->address()).arg(host->port(),0)) );
}

void KMLDonkey::disconnectFromCore()
{
    donkey->disconnectFromCore();
}

void KMLDonkey::coreDisconnected(int err)
{
    // Closing the socket during application shutdown is expected. The window
    // state has already been saved; do not notify or rebuild its pages here.
    if (m_finalised) return;
    tray->tooltip = i18n("Disconnected.");
    QString statInfoTooltip;

    statServer->setText(QString());
    statFiles->setText(QString());
    statShare->setText(QString());
    statTransfer->setText(QString());
    statRate->setText(QString());
    statBandwidth->setText(QString());

    switch (err) {
    case ProtocolInterface::AuthenticationError:
        statInfoTooltip = i18n("Authentication error. Incorrect name or password.");
        //KMessageBox::error(this, statInfoTooltip);
        reconnect = 0;
        break;
    case ProtocolInterface::ConnectionRefusedError:
        if (!reconnect) {
            statInfoTooltip = i18n("Connection to the core was refused. "
                                   "Are you sure it is running?");
            //if(isVisible())
            //    KMessageBox::error(this, statInfoTooltip);
            reconnect = 0;
        }
        else
            reconnect = persistentReconnect;
        break;
    case ProtocolInterface::HostNotFoundError: {
        HostInterface* host = hostManager->hostProperties(lastHost);
        if (!reconnect) {
            statInfoTooltip = host ? i18n("Host '%1' not found.", host->address()) : i18n("No Host defined.");
            //if (isVisible())
            //    KMessageBox::error(this, statInfoTooltip);
            reconnect = 0;
        }
        else
            reconnect = persistentReconnect;
    } break;
    case ProtocolInterface::IncompatibleProtocolError:
        statInfoTooltip = i18n("Your mldonkey core uses an obsolete communication protocol. "
                               "Please upgrade it to a more recent version.");
        //if (isVisible())
        //    KMessageBox::error(this, statInfoTooltip);
        reconnect = 0;
        break;
    default:
        if (!reconnect) {
            statInfoTooltip = i18n("A read error occurred when communicating "
                                   "with the core. The connection has been terminated.");
            //if (isVisible())
            //    KMessageBox::error(this, statInfoTooltip);
            reconnect = 0;
        }
        else
            reconnect = persistentReconnect;
        break;
    }

    const QString status = statInfoTooltip.isEmpty() ? tray->tooltip : QString("%1 %2").arg(tray->tooltip).arg(statInfoTooltip);
    changeStatusbar(status);
    statInfo->setToolTip(status.toHtmlEscaped());
    tray->setToolTip(status);
    KNotification::event(QStringLiteral("KMLDonkeyDisconnected"), i18n("Disconnected"), statInfoTooltip.toHtmlEscaped(), QStringLiteral("kmldonkey"));

    foreach( KMLDonkeyPage * page, pages )
        page->clear();

    enableActionList(coreActions, false);
    updateSupportedActions(0);
}

void KMLDonkey::updateStatusBarTooltip()
{
    DonkeyHost* host = (DonkeyHost*)hostManager->hostProperties(lastHost);
    statInfo->setToolTip(ki18n("Hostname: <b>%1</b><br>Host Address: <b>%2</b><br>Host Port: <b>%3</b><br>Username: <b>%4</b><br>"
                    "Core Version: <b>%7</b><br>Current GUI Protocol: <b>%5</b><br>Core's GUI Protocol: <b>%6</b>")
                    .subs(host->name().toHtmlEscaped())
                    .subs(host->address().toHtmlEscaped())
                    .subs(QString::number(host->port()))
                    .subs(host->username().toHtmlEscaped())
                    .subs(donkey->protocolVersion())
                    .subs(donkey->coreProtocolVersion())
                    .subs(coreVersion.toHtmlEscaped()).toString() );
}

void KMLDonkey::coreConnected()
{
    DonkeyHost* host = dynamic_cast<DonkeyHost*>( hostManager->hostProperties(lastHost) );
    if( donkey->getHost() && donkey->getHost() != host ) {
        host = dynamic_cast<DonkeyHost*>( donkey->getHost() );
        Q_ASSERT(host);
        lastHost = host->name();
    }

    const QString s = QString("%1@%2:%3 %4").arg(host->username()).arg(host->address()).arg(host->port()).arg(host->name());
    const QString t = i18n("Connected %1", s);
    tray->tooltip = t;
    tray->setToolTip(t);
    changeStatusbar(t);
    updateStatusBarTooltip();

    reconnect = 0;
    enableActionList(coreActions, true);
    updateSupportedActions(m_currentPage);

	queryBandwidth();
	
    KNotification::event(QStringLiteral("KMLDonkeyConnected"), i18n("Connected"), tray->tooltip.toHtmlEscaped(), QStringLiteral("kmldonkey"));
}

void KMLDonkey::checkReconnect()
{
    if (persistentReconnect && reconnect && (!donkey || !donkey->isConnected()))
        connectToCore();
}

void KMLDonkey::sendConsoleMessage(const QString& txt)
{
    consolePage->sendConsoleMessage(txt);
}

void KMLDonkey::updateStatus(int64 ul, int64 dl, int64, int nsh, int tul, int tdl, int, int, int ndl, int ncp, QMap<int,int>*)
{
    const QString server = QString("%1/%2").arg(donkey->connectedServerCount()).arg(donkey->totalServerCount());
    const QString files = QString("%1/%2").arg(ndl).arg(ncp);
    const QString share = QString("%1").arg(nsh);
    const QString transfer = QString("%1/%2").arg(FileInfo::humanReadableSize(dl)).arg(FileInfo::humanReadableSize(ul));
    const QString rate = QString("%1/%2").arg(QString::number((double)tdl / 1024.0,'f',1)).arg(QString::number((double)tul / 1024.0,'f',1));
    const QString serverLabel = i18n("Server %1", server);
    const QString filesLabel = i18n("Files %1", files);
    const QString shareLabel = i18n("Shared %1", share);
    const QString transferLabel = i18n("Transfer %1", transfer);
    const QString rateLabel = i18n("Speed %1", rate);

    statServer->setText(showStatusbarLabels ? serverLabel : server);
    statFiles->setText(showStatusbarLabels ? filesLabel : files);
    statShare->setText(showStatusbarLabels ? shareLabel : share);
    statTransfer->setText(showStatusbarLabels ? transferLabel : transfer);
    statRate->setText(showStatusbarLabels ? rateLabel : rate);

    tray->tooltip = QString("%1\n%2\n%3\n%4\n%5\n%6")
                    .arg(statInfo->text()).arg(serverLabel).arg(filesLabel).arg(shareLabel).arg(transferLabel).arg(rateLabel);
    tray->setToolTip(tray->tooltip);
		    
	queryBandwidth();
}


void KMLDonkey::coreVersionReceived(const QString& v)
{
    coreVersion = v;
    updateStatusBarTooltip();
}

void KMLDonkey::showBadInstallDialog()
{
    KMessageBox::detailedError(this, i18n("Unable to construct a GUI element. Your KMLDonkey installation is "
                                          "incomplete."),
                               i18n("Unable to construct a requested GUI element. This is most likely "
                                    "because the GUI specification data file (kmldonkeyui.rc) has been "
                                    "installed in the wrong location, or has not been installed at all. If you "
                                    "installed KMLDonkey from a binary package, please report this to the "
                                    "packager at once. If you installed from source, check that you configured "
                                    "KMLDonkey to use the same install prefix as your KDE installation "
                                    "(see the README file accompanying the KMLDonkey source for details), and "
                                    "that your system does not keep KDE files in non-standard locations "
                                    "(Mandriva Linux systems, in particular, are known to do this)."));
}

void KMLDonkey::submitUrl(QString url)
{
    donkey->submitUrl(url);
}


//
// Bandwidth management
//

class ToggleBandwidthCallback: public ConsoleCallbackInterface
{
public:
	ToggleBandwidthCallback( KMLDonkey * kmld ) 
		: m_kmld(kmld)
	{
	}
	
	virtual void callback(const QString& cmd, const QString& res)
	{
		(void) cmd;
		m_kmld->queryBandwidth();
		KNotification::event(QStringLiteral("KMLDonkeyToggleBandwidth"), i18n("Bandwidth changed"), res.toHtmlEscaped(), QStringLiteral("kmldonkey"));
	}
	
protected:
	KMLDonkey * m_kmld;
};

void KMLDonkey::actionToggleBandwidth()
{
	KMLDonkey::App->donkey->sendConsoleMessage( "bw_toggle", new ToggleBandwidthCallback( this ) );
}


class QueryBandwidthCallback: public ConsoleCallbackInterface
{
public:
	QueryBandwidthCallback( KMLDonkey * kmld ) 
		: m_kmld(kmld)
	{
	}
	
	virtual void callback(const QString& cmd, const QString& res)
	{
		(void) cmd;

		int bwdl = 0, bwul = 0;
		QString buf = res;
		QTextStream stream( &buf, QIODevice::ReadOnly );
		while( !stream.atEnd() )
		{
			QString s = stream.readLine();
			
			int pos = s.indexOf( " = " );
			if( pos > 0 )
			{
				QString key = s.left( pos );
				QString value = s.mid( pos + 3 );
				if( key == "max_hard_upload_rate" )
					bwul = value.toInt();
				else if( key == "max_hard_download_rate" )
					bwdl = value.toInt();
			}
		}
		
		m_kmld->updateBandwidth( bwdl, bwul );
	}
	
protected:
	KMLDonkey * m_kmld;
};


void KMLDonkey::queryBandwidth()
{
	// it seems there isn't a better way to query bandwidth that using 'vo' and parsing results...
	KMLDonkey::App->donkey->sendConsoleMessage( "vo", new QueryBandwidthCallback( this ) );
}

void KMLDonkey::updateBandwidth( int bwdl, int bwul )
{
    const QString bw = QString("%1/%2").arg( bwdl ).arg( bwul );
    const QString bwLabel = i18n("Bandwidth %1", bw);
    statBandwidth->setText( showStatusbarLabels ? bwLabel : bw );
}




