/* -*- c++ -*-
 *
 * kmldonkey.h
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006 Christian Muehlhaeuser <chris@chris.de>
 * Copyright (C) 2009 Gioacchino Mazzurco <gmazzurco89@gmail.com>
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

#ifndef __KMLDONKEY_KMLDONKEY_H__
#define __KMLDONKEY_KMLDONKEY_H__

#include <donkeytypes.h>
#include "page.h"

#include <KCoreAddons>
#include <kmainwindow.h>
#include <QKeySequence>
#include <QAction>
#include <kconfig.h>
#include <KXmlGuiWindow>

#include <qmap.h>
#include <QList>
#include <QDropEvent>
#include <QList>
#include <QLabel>
#include <QDragEnterEvent>

class ConsolePage;
class DonkeyProtocol;
class DownloadPage;
class FriendPage;
class HostManager;
class HostSelectAction;
class KMLDonkeyPreferences;
class SearchPage;
class ServerPage;
class SharePage;
class StatsPage;
class SystemTray;

class KPageWidget;
class KPageWidgetModel;
class KToggleAction;

class QModelIndex;
class QLabel;
class QSignalMapper;
class QTimer;
class KmldView;

extern void enableActionList( QList<QAction *> list, bool enable );

class KMLDonkey : public KXmlGuiWindow
{
        Q_OBJECT
    public:
        KMLDonkey();
        virtual ~KMLDonkey();

        static KMLDonkey* App;
        DonkeyProtocol* donkey;
        HostManager* hostManager;

        KmldView *viewCreator;

        // Configuration variables
        bool displayProgressbar;

        bool enableAutorename;
        QList<QStringList> autorenameList;

        bool coloredViews;
        QColor colorServerNotConnected, colorServerBlacklisted, colorServerConnecting, colorServerConnected;
        QColor colorSearchFewSources, colorSearchManySources, colorSearchAlreadyDone;
        int searchThreshold;
        QColor colorDownloadDownloading, colorDownloadPaused, colorDownloadLooking, colorDownloadNotAvailable, colorDownloadQueued;
        QColor colorNoSources, colorComplete, colorFewSources, colorManySources;
        int availabilityThreshold, availabilityShadingDepth;
        bool availabilityShading;
        QColor colorSourceNotConnected, colorSourceBlacklisted, colorSourceConnecting, colorSourceQueued, colorSourceDownloading;

        QFont listFont, consoleFont;

        bool advancedConfig;

        QString lastHost;

        bool sortDownloadingFirst();
        bool sortPausedLast();

        void addGenericAction(const char* name, QAction* action);
        void addGenericAction(const char* name, const QString& label, const QString& icontext, const QString& icon, const QKeySequence& shortcut);
        void addCoreAction(QAction* action);

    protected:
        virtual bool queryClose();
        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dropEvent(QDropEvent *event);

        void restoreState(KSharedConfigPtr conf);
        void saveState(KSharedConfigPtr conf);

        KPageWidget* pageTab;
        KPageWidgetModel* pageModel;

    public slots:
        void submitUrl(QString url);
        void sendConsoleMessage(const QString& txt);

        void show();
        void hide();

        void activatePage(KMLDonkeyPage* page);
        void activatePage(int page);
        void activateNextPage();
        void activatePreviousPage();

        void showBadInstallDialog();

        void finalise();
        bool queryExit();

        void optionsShowToolbar();
        void optionsShowStatusbar();
        void optionsConfigureKeys();
        void optionsConfigureToolbars();
        void optionsConfigureConnection();
        void optionsConfigureMLDonkey();
        void optionsPreferences();
        void newToolbarConfig();
        void optionsNotifications();

        void applyPreferences();

        void actionConnectCore();
        void actionConnectCore(const QString& host);
        void actionDisconnectCore();
        void actionToggleBandwidth();
        void actionKillCore();
        void actionSubmitUrl();
        void actionSubmitCollection();
	void actionSubmitTorrent();

        void changeStatusbar(const QString& text);
        void changeCaption(const QString& text);

        void connectToCore();
        void disconnectFromCore();
        void coreConnected();
        void coreDisconnected(int);
        void checkReconnect();

        void updateStatus(int64 ul, int64 dl, int64 sh, int nsh, int tul, int tdl, int uul, int udl, int ndl, int ncp, QMap<int,int>*);

	void queryBandwidth();
	void updateBandwidth( int bwdl, int bwul );

        void hostListUpdated();

        void handleGenericAction(const QString&);
        void pageActionsChanged(KMLDonkeyPage*);
        void currentPageChanged(const QModelIndex &current, const QModelIndex &previous);

        void coreVersionReceived(const QString&);

    private:
        bool m_finalised = false;
        void setupAccel();
        void setupActions();
        QWidget* buildInterface();
        void addPage(KMLDonkeyPage* page, const QIcon& iconset, const QString& label);
        void removePage(KMLDonkeyPage* page);
        void updateSupportedActions(KMLDonkeyPage*);
        void updateTrayIcon();
        void updateStatusBarTooltip();

        SystemTray* tray;

        int reconnect;

        HostSelectAction* m_connectAction;
        KToggleAction* m_toolbarAction;
        KToggleAction* m_statusbarAction;
        bool showStatusbarLabels;
        bool persistentReconnect, hideMainWin;
        QString coreCharset, coreVersion;
        bool showTrayIcon, closeToTrayIcon;

        QList<KMLDonkeyPage *> pages;

        ConsolePage* consolePage;
        ServerPage* serverPage;
        DownloadPage* downloadPage;
        FriendPage* friendPage;
        SharePage* sharePage;
        SearchPage *searchPage;
        StatsPage *statsPage;

        QLabel *statInfo, *statServer, *statFiles, *statShare, *statTransfer, *statRate, *statBandwidth;

        KMLDonkeyPreferences* prefs;

        QTimer* timer;

        QSignalMapper* pageMapper;

        QSignalMapper* actionMapper;
        QMap<QString, QAction *> genericActions;
        QList<QAction *> coreActions;

        KMLDonkeyPage* m_currentPage;
};

#endif /* __KMLDONKEY_KMLDONKEY_H__ */
