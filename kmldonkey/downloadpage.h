#include <KToggleAction>
/* -*- c++ -*-
 *
 * downloadpage.h
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006 Christian Muehlhaeuser <chris@chris.de>
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

#ifndef __KMLDONKEY_DOWNLOADPAGE_H__
#define __KMLDONKEY_DOWNLOADPAGE_H__

#include <QHash>
#include <QObject>
#include <KLineEdit>
#include <QList>

#include "clipboard.h"
#include "page.h"

class InfoList;
class KActionCollection;
class QSplitter;
class QTimer;

class DownloadPage : public QWidget, public KMLDonkeyPage, public ClipboardHelper
{
    Q_OBJECT

    public:
        DownloadPage( QWidget* parent = 0 );

        virtual void setupActions( KActionCollection* );
        virtual QStringList supportedGenericActions();
        virtual void handleGenericAction( const QString& );
        virtual void plugGenericActions( QObject*, const char* );
        virtual void deactivatePageActions();

        virtual void configurePrefsDialog( KMLDonkeyPreferences* prefs = 0 );
        virtual void applyPreferences( KMLDonkeyPreferences* prefs = 0 );
        virtual void saveState(KSharedConfigPtr);
        virtual void restoreState(KSharedConfigPtr);

        virtual void clear();
        void setFilter( const QString &filter ) { m_filter = filter; }

        InfoList* downloadView;
        InfoList* downloadedView;

        KToggleAction* hidePausedAction;
        KToggleAction* hideUnpausedAction;
        KToggleAction* sortDownloadingFirstAction;
        KToggleAction* sortPausedLastAction;

    signals:
        void genericActionsChanged( KMLDonkeyPage* );

    public slots:
        void fileUpdated( int fn );
        void fileAdded( int fn, bool downloadstarted );
        void fileRemoved( int fn );
        void fileRenamed( int fileno, const QString& name );

        // Download actions
        void actionPauseFile();
        void actionResumeFile();
        void actionCancelFile();
        void actionRenameFile();
        void actionRenameFileInPlace();
        void actionRetryConnect();
        void actionPreviewFile();
        void actionOpenFile();
        void actionVerifyFile();
        void actionGetFileFormat();
        void actionFileInformation();
        void actionCompleteFileInformation();
        void actionHidePaused( bool );
        void actionHideUnpaused( bool );
        void actionSortDownloadingFirst( bool );
        void actionSortPausedLast( bool );

        void actionFilePriorityVeryLow() { setFilePriority( -20 ); }
        void actionFilePriorityLow() { setFilePriority( -10 ); }
        void actionFilePriorityNormal() { setFilePriority( 0 ); }
        void actionFilePriorityHigh() { setFilePriority( 10 ); }
        void actionFilePriorityVeryHigh() { setFilePriority( 20 ); }

        // Complete actions
        void actionCommitFile();
        void actionCommitFileAs();
        void commitFileAs( int fileno, const QString& name );

        void actionActivatePage();

    private slots:
        void slotSetFilterTimeout();
        void slotSetFilter();
        void updateDownloadView();

    protected slots:
        void contextDownload( QTreeWidgetItem*, const QPoint& );
        void contextDownloaded( QTreeWidgetItem*, const QPoint& );
        void downloadDoubleClick( QTreeWidgetItem* );
        void clearDownloadSelection();
        void clearDownloadedSelection();

    protected:
        void copyFileToClipboard( ClipFormat format );
        void copyCompleteToClipboard( ClipFormat format );
        void setFilePriority( int );

    private:
        QTimer* m_timer;
        QSplitter* downloadSplitter;
        KLineEdit* m_filterEdit;
        QString m_filter;

        QHash<int, DownloadFile *> downloadFileList;
        QHash<int, DownloadedFile *> downloadedFileList;

        QList<QAction *> downloadActions;
        QList<QAction *> downloadedActions;

        void autoRenameFile( int filenum, const QString& filename );
};

#endif
