#include <QSet>
#include <QRegularExpression>
#include <KConfigGroup>
/* -*- c++ -*-
 *
 * downloadpage.cpp
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

#include <QCheckBox>
#include <QRegExp>
#include <QMenu>
#include <QSplitter>
#include <QTimer>
#include <QPushButton>
#include <QList>
#include <QList>
#include <QFrame>

#include <QAction>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <kconfig.h>
#include <QDebug>
#include <QTreeWidget>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <knotification.h>
#include <QSpinBox>
#include <kxmlguifactory.h>
#include <ktoolbar.h>

#include "infodialog.h"
#include "infolist.h"
#include "kmldonkey.h"
#include "prefs/prefs.h"
#include "renamedialog.h"

#include "downloadpage.h"

DownloadPage::DownloadPage( QWidget* parent )
    : QWidget( parent )
    , KMLDonkeyPage()
    , ClipboardHelper()
    , m_timer( new QTimer( this ) )
{
    setObjectName("downloadPage");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    downloadSplitter = new QSplitter( this );
    downloadSplitter->setObjectName("downloadSplitter");
    downloadSplitter->setOrientation( Qt::Vertical );
    layout->addWidget( downloadSplitter );

    QWidget *downloadBox = new QWidget( downloadSplitter );
    QVBoxLayout* l = new QVBoxLayout(downloadBox);
    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
    downloadBox->setLayout(l);
    {
        QWidget *searchBox = new QWidget( downloadBox );
        QHBoxLayout* searchlayout = new QHBoxLayout(searchBox);
        searchlayout->setSpacing(8);
        searchlayout->setContentsMargins(12, 8, 12, 8);
        searchBox->setLayout(searchlayout);
        //searchBox->setContentsMargins( 4 ,  4 ,  4 ,  4 );
        l->addWidget( searchBox );

        m_filterEdit = new KLineEdit( /*i18n( "Filter here" ),*/ searchBox );
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(i18n("Filter downloads…"));
        m_filterEdit->setAccessibleName(i18n("Filter downloads"));
        m_filterEdit->addAction(QIcon::fromTheme(QStringLiteral("edit-find")), QLineEdit::LeadingPosition);
        searchlayout->addWidget( m_filterEdit );

        downloadView = new InfoList( downloadBox, "downloadView", true );
        l->addWidget( downloadView, 1 );
        downloadView->addColumn( i18n( "File name" ), 320 );
        downloadView->addColumn( i18n( "Speed" ), 60 );
        downloadView->addColumn( i18n( "Progress %" ) );
        downloadView->addColumn( i18n( "Received" ) );
        downloadView->addColumn( i18n( "Size" ) );
        downloadView->addColumn( i18n( "ETA" ) );
        downloadView->addColumn( i18n( "Priority" ) );
        downloadView->addColumn( i18n( "Status" ) );
        downloadView->addColumn( i18n( "Network" ) );
        downloadView->addColumn( i18n( "Availability" ) );
        downloadView->addColumn( i18n( "Hash" ), 320 );
        downloadView->addColumn( i18n( "Last seen" ), 60 );
        downloadView->addColumn( i18n( "Age" ), 60 );
        downloadView->addColumn( i18n( "File type" ), 320 );
    }

    downloadedView = new InfoList( downloadSplitter, "downloadedView", true );
    downloadedView->addColumn( i18n( "File name" ), 320 );
    downloadedView->addColumn( i18n( "Size" ) );
    downloadedView->addColumn( i18n( "Format" ) );
    downloadedView->addColumn( i18n( "Network" ) );
    downloadedView->addColumn( i18n( "Hash" ), 320 );
    downloadedView->addColumn( i18n( "File type" ), 340 );

    connect( m_timer,        SIGNAL( timeout() ), SLOT( slotSetFilter() ) );
    connect( m_filterEdit,   SIGNAL( textChanged( const QString& ) ), SLOT( slotSetFilterTimeout() ) );
    connect( m_filterEdit,   SIGNAL( returnPressed() ), SLOT( slotSetFilter() ) );

    connect( downloadView,   SIGNAL( contextMenu( QTreeWidgetItem*, const QPoint& ) ),
             this,             SLOT( contextDownload( QTreeWidgetItem*, const QPoint& ) ) );
    connect( downloadView,   SIGNAL( itemDoubleClicked(QTreeWidgetItem*,int)),
             this,             SLOT( downloadDoubleClick( QTreeWidgetItem* ) ) );
    connect( downloadView,   SIGNAL( itemActivated(QTreeWidgetItem*,int)),
             this,             SLOT( downloadDoubleClick( QTreeWidgetItem* ) ) );
    connect( downloadedView, SIGNAL( contextMenu( QTreeWidgetItem*, const QPoint& ) ),
             this,             SLOT( contextDownloaded( QTreeWidgetItem*, const QPoint& ) ) );

    connect( downloadView,   SIGNAL( itemSelectionChanged() ), SLOT( clearDownloadedSelection() ) );
    connect( downloadedView, SIGNAL( itemSelectionChanged() ), SLOT( clearDownloadSelection() ) );

    connect( KMLDonkey::App->donkey, SIGNAL( updatedDownloadFiles() ), this, SLOT( updateDownloadView() ) );
    connect( KMLDonkey::App->donkey, SIGNAL( updatedDownloadedFiles() ), this, SLOT( updateDownloadView() ) );
    connect( KMLDonkey::App->donkey, SIGNAL( fileUpdated( int, DonkeyProtocol::FromCore ) ), this, SLOT( fileUpdated( int ) ) );
    connect( KMLDonkey::App->donkey, SIGNAL( fileAdded( int, bool ) ), this, SLOT( fileAdded( int, bool ) ) );
    connect( KMLDonkey::App->donkey, SIGNAL( fileRemoved( int ) ), this, SLOT( fileRemoved( int ) ) );

    downloadSplitter->setStretchFactor(0,2);
    downloadSplitter->setStretchFactor(1,1);
}

void DownloadPage::updateDownloadView()
{
    QSet<int> present;
    for (FileInfo *file : KMLDonkey::App->donkey->downloadFiles()) {
        present.insert(file->fileNo());
        fileUpdated(file->fileNo());
    }
    for (FileInfo *file : KMLDonkey::App->donkey->downloadedFiles()) {
        present.insert(file->fileNo());
        fileUpdated(file->fileNo());
    }
    for (int id : downloadFileList.keys()) if (!present.contains(id)) fileRemoved(id);
    for (int id : downloadedFileList.keys()) if (!present.contains(id)) fileRemoved(id);
}

void DownloadPage::slotSetFilterTimeout() //SLOT
{
	m_timer->setSingleShot( true );
    m_timer->start( 280 ); //stops the timer for us first
}

void DownloadPage::slotSetFilter() //SLOT
{
    m_timer->stop();
    setFilter( m_filterEdit->text() );
    updateDownloadView();
}

void DownloadPage::handleGenericAction( const QString& action )
{
    if ( downloadView->hasFocus() )
    {
        if (action == "copy_url") copyFileToClipboard( URL );
        else if (action == "copy_html") copyFileToClipboard( HTML );
        else if (action == "copy_hash") copyFileToClipboard( Hash );
        else if (action == "file_information") actionFileInformation();
    }
    else if ( downloadedView->hasFocus() )
    {
        if (action == "copy_url") copyCompleteToClipboard( URL );
        else if (action == "copy_html") copyCompleteToClipboard( HTML );
        else if (action == "copy_hash") copyCompleteToClipboard( Hash );
        else if (action == "file_information") actionCompleteFileInformation();
    }
}

void DownloadPage::deactivatePageActions()
{
    enableActionList( downloadActions, false );
    enableActionList( downloadedActions, false );
}

QStringList DownloadPage::supportedGenericActions()
{
    QStringList actions;
    QList<QTreeWidgetItem *> dl = downloadView->selectedItems();
    QList<QTreeWidgetItem *> cl = downloadedView->selectedItems();

    if ( !dl.isEmpty() )
    {
        enableActionList( downloadActions, true );
        actions.append( "copy_url" );
        actions.append( "copy_html" );
        actions.append( "copy_hash" );
        actions.append( "file_information" );
    }
    else
        enableActionList( downloadActions, false );

    if ( !cl.isEmpty() )
    {
        enableActionList( downloadedActions, true );
        actions.append( "copy_url" );
        actions.append( "copy_html" );
        actions.append( "copy_hash" );
        actions.append( "file_information" );
    }
    else
        enableActionList( downloadedActions, false );

    return actions;
}

void DownloadPage::plugGenericActions( QObject* object, const char* slot )
{
    connect( this, SIGNAL( genericActionsChanged( KMLDonkeyPage* ) ), object, slot );
}

void DownloadPage::setupActions( KActionCollection* actionCollection )
{
    QAction* action = new QAction( QIcon::fromTheme("media-playback-pause"), i18n( "&Pause" ), this );
    connect(action, SIGNAL(triggered()), this, SLOT(actionPauseFile()));
    actionCollection->addAction("pause_selected", action);
    downloadActions.append(action);

    action = new QAction( QIcon::fromTheme("media-playback-start"), i18n( "&Resume" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionResumeFile()));
    actionCollection->addAction("resume_selected", action);
    downloadActions.append(action);

    action = new QAction( QIcon::fromTheme("media-playback-stop"), i18n( "&Cancel" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionCancelFile()));
    actionCollection->addAction("cancel_selected", action);
    downloadActions.append(action);

    action = new QAction( QIcon::fromTheme("edit-rename"), i18n( "Re&name" ), this);
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::Key_F2));
    connect(action, SIGNAL(triggered()), this, SLOT(actionRenameFileInPlace()));
    actionCollection->addAction("rename_inplace", action);
    downloadActions.append(action);

    action = new QAction( QIcon::fromTheme("document-properties"), i18n( "Ren&ame To..." ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionRenameFile()));
    actionCollection->addAction("rename_selected", action);
    downloadActions.append(action);

    action = new QAction( QIcon::fromTheme("view-refresh"), i18n( "Re&try Connect" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionRetryConnect()));
    actionCollection->addAction("retry_selected", action);
    downloadActions.append(action);

    action = new QAction( QIcon::fromTheme("view-preview"), i18n( "Pre&view" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionPreviewFile()));
    actionCollection->addAction("preview_selected", action);
    downloadActions.append(action);

    action = new QAction( QIcon::fromTheme("select-rectangular"), i18n( "Verify Chunks" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionVerifyFile()));
    actionCollection->addAction("verify_selected", action);
    downloadActions.append(action);

    action = new QAction( QIcon::fromTheme("view-refresh"), i18n( "Get &Format Info" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionGetFileFormat()));
    actionCollection->addAction("get_format_selected", action);
    downloadActions.append(action);

    hidePausedAction = new KToggleAction(i18n( "&Hide Paused Downloads" ), this);
    actionCollection->addAction("hide_paused_downloads", hidePausedAction);
    connect( hidePausedAction, SIGNAL( toggled( bool ) ), SLOT( actionHidePaused( bool ) ) );

    hideUnpausedAction = new KToggleAction(i18n( "Show Only Paused Downloads" ), this);
    actionCollection->addAction("hide_unpaused_downloads", hideUnpausedAction);
    connect( hideUnpausedAction, SIGNAL( toggled( bool ) ), SLOT( actionHideUnpaused( bool ) ) );

    sortDownloadingFirstAction = new KToggleAction(i18n( "Sort Downloading Files First" ), this);
    actionCollection->addAction("sort_downloading_first", sortDownloadingFirstAction);
    connect( sortDownloadingFirstAction, SIGNAL( toggled( bool ) ), SLOT( actionSortDownloadingFirst( bool ) ) );

    sortPausedLastAction = new KToggleAction(i18n( "Sort Paused Files Last" ), this);
    actionCollection->addAction("sort_paused_last", sortPausedLastAction);
    connect( sortPausedLastAction, SIGNAL( toggled( bool ) ), SLOT( actionSortPausedLast( bool ) ) );

    KActionMenu* priMenu = new KActionMenu( i18n( "Priorit&y" ), this);
    actionCollection->addAction("priority_menu", priMenu);
    priMenu->setPopupMode(QToolButton::InstantPopup);
    downloadActions.append( priMenu );

    action = new QAction(QIcon::fromTheme("go-bottom"), i18n( "Very L&ow Priority" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionFilePriorityVeryLow()));
    priMenu->addAction( action );
    downloadActions.append( action );

    action = new QAction(QIcon::fromTheme("go-down"), i18n( "&Low Priority" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionFilePriorityLow()));
    priMenu->addAction( action );
    downloadActions.append( action );

    action = new QAction(QIcon::fromTheme("go-right"), i18n( "&Normal Priority" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionFilePriorityNormal()));
    priMenu->addAction( action );
    downloadActions.append( action );

    action = new QAction(QIcon::fromTheme("go-up"), i18n( "&High Priority" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionFilePriorityHigh()));
    priMenu->addAction( action );
    downloadActions.append( action );

    action = new QAction(QIcon::fromTheme("go-top"), i18n( "&Very High Priority" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionFilePriorityVeryHigh()));
    priMenu->addAction( action );
    downloadActions.append( action );

    action = new QAction(QIcon::fromTheme("view-preview"), i18n( "&Open" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionOpenFile()));
    actionCollection->addAction("open_selected", action);
    downloadedActions.append( action );

    action = new QAction(QIcon::fromTheme("document-save"), i18n( "&Commit" ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionCommitFile()));
    actionCollection->addAction("commit_selected", action);
    downloadedActions.append( action );

    action = new QAction(QIcon::fromTheme("document-save-as"), i18n( "Commit &As..." ), this);
    connect(action, SIGNAL(triggered()), this, SLOT(actionCommitFileAs()));
    actionCollection->addAction("commit_as_selected", action);
    downloadedActions.append( action );

    action = new QAction( QIcon::fromTheme("go-down"), i18n( "Activate Downloads Page" ), this );
    action->setIconText(i18n("Downloads"));
    connect(action, SIGNAL(triggered()), this, SLOT(actionActivatePage()));
    actionCollection->addAction("activate_page_download", action);

    enableActionList( downloadActions, false );
    enableActionList( downloadedActions, false );
}

void DownloadPage::clear()
{
    deactivatePageActions();

    qDeleteAll( downloadedFileList );
    downloadedFileList.clear();

    qDeleteAll( downloadFileList );
    downloadFileList.clear();

    downloadView->clear();
    downloadedView->clear();
}

void DownloadPage::clearDownloadSelection()
{
    downloadView->clearSelection();
    emit genericActionsChanged( static_cast<KMLDonkeyPage*>( this ) );
}

void DownloadPage::clearDownloadedSelection()
{
    downloadedView->clearSelection();
    emit genericActionsChanged( static_cast<KMLDonkeyPage*>( this ) );
}

void DownloadPage::configurePrefsDialog( KMLDonkeyPreferences* prefs )
{
    if ( !prefs )
        return;

    prefs->renamerPage->enableAutorenameCheckbox->setChecked( KMLDonkey::App->enableAutorename );
    emit prefs->renamerPage->enableAutorename( KMLDonkey::App->enableAutorename );

    prefs->renamerPage->renlisteditor->addRows( KMLDonkey::App->autorenameList );
    prefs->listsPage->hidePausedCheckbox->setChecked( hidePausedAction->isChecked() );
    prefs->listsPage->hideUnpausedCheckbox->setChecked( hideUnpausedAction->isChecked() );
    prefs->listsPage->SortDownloadingFirstCheckBox->setChecked( sortDownloadingFirstAction->isChecked() );
    prefs->listsPage->SortPausedLastCheckBox->setChecked( sortPausedLastAction->isChecked() );

    if ( downloadView->sortInterval() > 0 )
    {
        prefs->listsPage->autoDownloadResortCheckbox->setChecked( true );
        prefs->listsPage->downloadResortEntry->setValue( downloadView->sortInterval() );
    }
    else
        prefs->listsPage->autoDownloadResortCheckbox->setChecked( false );
}

void DownloadPage::applyPreferences( KMLDonkeyPreferences* prefs )
{
    const QFont& font = KMLDonkey::App->listFont;

    if ( downloadView->font() != font ) downloadView->setFont( font );
    if ( downloadedView->font() != font ) downloadedView->setFont( font );

    QTreeWidgetItemIterator it( downloadView );
	while( (*it) )
	{
        DownloadFile* cl = dynamic_cast<DownloadFile*>( (*it) );
        if ( cl )
            cl->loseCache();
		++it;
	}
    downloadView->update();

    if ( !prefs )
        return;

    if ( prefs->listsPage->hidePausedCheckbox->isChecked() != hidePausedAction->isChecked() )
         actionHidePaused( prefs->listsPage->hidePausedCheckbox->isChecked() );
    if ( prefs->listsPage->hideUnpausedCheckbox->isChecked() != hideUnpausedAction->isChecked() )
         actionHideUnpaused( prefs->listsPage->hideUnpausedCheckbox->isChecked() );

    if ( prefs->listsPage->SortDownloadingFirstCheckBox->isChecked() != sortDownloadingFirstAction->isChecked() )
         actionSortDownloadingFirst( prefs->listsPage->SortDownloadingFirstCheckBox->isChecked() );
    if ( prefs->listsPage->SortPausedLastCheckBox->isChecked() != sortPausedLastAction->isChecked() )
         actionSortPausedLast( prefs->listsPage->SortPausedLastCheckBox->isChecked() );

    if ( prefs->listsPage->autoDownloadResortCheckbox->isChecked() != ( downloadView->sortInterval() > 0) ||
         (prefs->listsPage->autoDownloadResortCheckbox->isChecked() && ( downloadView->sortInterval() != prefs->listsPage->downloadResortEntry->value() ) ) )
            downloadView->setSortInterval( prefs->listsPage->autoDownloadResortCheckbox->isChecked() ? prefs->listsPage->downloadResortEntry->value() : 0 );

    bool b = prefs->renamerPage->enableAutorenameCheckbox->isChecked();
    if ( KMLDonkey::App->enableAutorename != b || prefs->renamerPage->renlisteditor->Modified )
    {
        KMLDonkey::App->enableAutorename = b;
        KMLDonkey::App->autorenameList = prefs->renamerPage->renlisteditor->getRows();

        KSharedConfigPtr conf = KSharedConfig::openConfig();
        conf->group( "Autorename" ).deleteGroup();
        KConfigGroup group = conf->group( "Autorename" );
        group.writeEntry( "Enable", KMLDonkey::App->enableAutorename );
        int i = 0;
        for ( QList<QStringList>::Iterator it = KMLDonkey::App->autorenameList.begin();
             it != KMLDonkey::App->autorenameList.end();
             ++it )
        {
            if ( (*it).count() < 2 /*|| !(*it)[0]*/ )
                continue;
            group.writeEntry( QString::number(i)+"s", (*it)[0] );
            group.writeEntry( QString::number(i)+"r", (*it)[1] );
            i++;
        }

        if ( KMLDonkey::App->enableAutorename )
        {
            QHash<int, DownloadFile *>::const_iterator it;
            for ( it = downloadFileList.begin(); it != downloadFileList.end(); ++it )
            {
                FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it.key() );
                if ( fi )
                    autoRenameFile( fi->fileNo(), fi->fileName() );
            }
        }
    }
}

void DownloadPage::saveState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "DownloadPage" );
    group.writeEntry( "HidePausedDownloads", hidePausedAction->isChecked() );
    group.writeEntry( "HideUnpausedDownloads", hideUnpausedAction->isChecked() );
    group.writeEntry( "SortDownloadingFirst", sortDownloadingFirstAction->isChecked() );
    group.writeEntry( "SortPausedLast", sortPausedLastAction->isChecked() );
    group.writeEntry( "DownloadViewSort", downloadView->sortInterval() );
    group.writeEntry( "DownloadSplitter", downloadSplitter->sizes() );
    downloadView->saveLayout();
    downloadedView->saveLayout();
}

void DownloadPage::restoreState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "DownloadPage" );
    hidePausedAction->setChecked( group.readEntry( "HidePausedDownloads", false ) );
    hideUnpausedAction->setChecked( group.readEntry( "HideUnpausedDownloads", false ) );
    sortDownloadingFirstAction->setChecked( group.readEntry( "SortDownloadingFirst", false ) );
    sortPausedLastAction->setChecked( group.readEntry( "SortPausedLast", false ) );
    downloadView->setSortInterval( group.readEntry( "DownloadViewSort", 0 ) );

    QList<int> downloadsplitter = group.readEntry( "DownloadSplitter", QList<int>() );
    if( downloadsplitter.count() > 0 )
        downloadSplitter->setSizes(downloadsplitter);

    downloadView->initialise();
    downloadedView->initialise();

    KMLDonkey::App->autorenameList.clear();
#if 0
    if ( conf->hasGroup( "Autorename" ) )
    {
        conf->setGroup( "Autorename" );
        KMLDonkey::App->enableAutorename = conf->readBoolEntry( "Enable", false );
        QString s, r;
        int nr = 0;

        while ( 1 )
        {
            s = conf->readEntry<QString>( QString::number( nr ) + "s", QString() );
            if ( s.isEmpty() )
                break;

            r = conf->readEntry<QString>( QString::number( nr ) + "r", QString( "" ) );
            KMLDonkey::App->autorenameList.append( QStringList() << s << r );
            nr++;
        }
    }
    else
    {
        // default Autorename-list
        KMLDonkey::App->enableAutorename = false;
        KMLDonkey::App->autorenameList.append( QStringList() << "&auml" << "ae" );
        KMLDonkey::App->autorenameList.append( QStringList() << "&Auml" << "Ae" );
        KMLDonkey::App->autorenameList.append( QStringList() << "&ouml" << "oe" );
        KMLDonkey::App->autorenameList.append( QStringList() << "&Ouml" << "Oe" );
        KMLDonkey::App->autorenameList.append( QStringList() << "&uuml" << "ue" );
        KMLDonkey::App->autorenameList.append( QStringList() << "&Uuml" << "Ue" );
        KMLDonkey::App->autorenameList.append( QStringList() << "&szlig" << "ss" );
        KMLDonkey::App->autorenameList.append( QStringList() << "(\\.)(?=.*\\.)" << " " );
        KMLDonkey::App->autorenameList.append( QStringList() << "%20|_|([\\s]+)" << " " );
    }
#endif

    applyPreferences();
}

void DownloadPage::contextDownload( QTreeWidgetItem*, const QPoint& pt )
{
    QMenu *pop = dynamic_cast<QMenu*>( ( KMLDonkey::App->factory() )->container( "download_actions", KMLDonkey::App ) );
    if ( !pop )
        KMLDonkey::App->showBadInstallDialog();
    else
        pop->popup( pt );
}

void DownloadPage::contextDownloaded( QTreeWidgetItem*, const QPoint& pt )
{
    QMenu *pop = dynamic_cast<QMenu*>( ( KMLDonkey::App->factory() )->container( "downloaded_actions", KMLDonkey::App ) );
    if ( !pop )
        KMLDonkey::App->showBadInstallDialog();
    else
        pop->popup( pt );
}

void DownloadPage::downloadDoubleClick( QTreeWidgetItem* )
{
    actionFileInformation();
}

void DownloadPage::actionActivatePage()
{
    KMLDonkey::App->activatePage( this );
}

void DownloadPage::actionPauseFile()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        KMLDonkey::App->donkey->pauseFile( fi->fileNo(), 1 );
    }
}

void DownloadPage::actionResumeFile()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        KMLDonkey::App->donkey->pauseFile( fi->fileNo(), 0 );
    }
}

void DownloadPage::actionCancelFile()
{
    QStringList files;
    QList<int> fileNumbers;
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( fi ) {
            files << fi->fileName();
            fileNumbers.append(fi->fileNo());
        }
    }
    if ( KMessageBox::questionTwoActionsList( this, i18n( "Are you sure you want to cancel these files?" ),
         files, i18n( "Cancel Downloads" ), KGuiItem(i18n("Cancel Downloads")), KGuiItem(i18n("Keep Downloads")) ) == KMessageBox::PrimaryAction )
    {
        // The confirmation dialog processes core updates, which can remove
        // selected rows. Retain file IDs rather than pointers to those rows.
        for (int fileNumber : fileNumbers)
        {
            FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo(fileNumber);
            if ( !fi )
                continue;
            KMLDonkey::App->donkey->cancelFile( fi->fileNo() );
        }
    }
}

void DownloadPage::setFilePriority( int pri )
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo(it->fileNo());
        if ( !fi )
            continue;
        KMLDonkey::App->donkey->setFilePriority( fi->fileNo(), pri );
    }
}

void DownloadPage::actionRetryConnect()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        KMLDonkey::App->donkey->retryFile( fi->fileNo() );
    }
}

void DownloadPage::actionPreviewFile()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        KMLDonkey::App->donkey->previewFile( fi->fileNo() );
    }
}

void DownloadPage::actionOpenFile()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        KMLDonkey::App->donkey->previewFile( fi->fileNo() );
    }
}

void DownloadPage::actionVerifyFile()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        KMLDonkey::App->donkey->verifyFileChunks( fi->fileNo() );
    }
}

void DownloadPage::actionGetFileFormat()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        KMLDonkey::App->donkey->getFileFormat( fi->fileNo() );
    }
}

void DownloadPage::actionFileInformation()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        FileInfoDialog* d = new FileInfoDialog( fi->fileNo() );
        d->show();
    }
}

void DownloadPage::actionCompleteFileInformation()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        FileInfoDialog* d = new FileInfoDialog( fi->fileNo() );
        d->show();
    }
}

void DownloadPage::actionCommitFile()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        KMLDonkey::App->donkey->saveFile( fi->fileNo(), fi->fileName() );
    }
}

void DownloadPage::actionCommitFileAs()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        RenameDialog* d = new RenameDialog( KMLDonkey::App->donkey, fi->fileNo() );
        connect( d, SIGNAL( renameFile( int, const QString& ) ), this, SLOT( commitFileAs( int, const QString& ) ) );
        d->show();
    }
}

void DownloadPage::commitFileAs( int fileno, const QString& name )
{
    KMLDonkey::App->donkey->saveFile( fileno, name );
}

void DownloadPage::actionRenameFile()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        RenameDialog* d = new RenameDialog( KMLDonkey::App->donkey, fi->fileNo() );
        connect( d, SIGNAL( renameFile( int, const QString& ) ), this, SLOT( fileRenamed(int,QString)) );
        d->show();
    }
}

void DownloadPage::actionRenameFileInPlace()
{
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
		downloadView->editItem( it, 0 );
        return;
    }
}

void DownloadPage::fileRenamed( int fileno, const QString& name )
{
    KMLDonkey::App->donkey->renameFile( fileno, name );
}

void DownloadPage::copyFileToClipboard( ClipFormat format )
{
    QStringList l;
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( it->fileNo() );
        if ( !fi )
            continue;
        l.append( fi->fileName() );
        l.append( fi->fileUid( "ed2k" ) );
        l.append( QString::number( (long int)fi->fileSize() ) );
    }
    copyToClipboard(l, format);
}

void DownloadPage::copyCompleteToClipboard(ClipFormat format)
{
    QStringList l;
    QList<QTreeWidgetItem *> list = downloadView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        DownloadFile* it = (DownloadFile*) i;

        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo(it->fileNo() );
        if ( !fi )
            continue;
        l.append( fi->fileName() );
        l.append( fi->fileUid( "ed2k" ) );
        l.append( QString::number( (long int)fi->fileSize() ) );
    }
    copyToClipboard(l, format);
}

void DownloadPage::fileUpdated( int fn )
{
    FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo(fn);
    if (!fi) fi = KMLDonkey::App->donkey->findDownloadedFileNo(fn);
    if (!fi) { fileRemoved(fn); return; }

    bool completed = false;
    bool del;

    switch ( fi->fileState() )
    {
        case FileInfo::Complete:
            completed = true;

        case FileInfo::Cancelled:
        case FileInfo::Aborted:
        case FileInfo::Shared:
            del = true;
            break;

        case FileInfo::Paused:
            del = hidePausedAction->isChecked() ? true : false;
            break;

        default:
            del = false;
            break;
    }

    // check if we really want to display this item
    if ( ( hideUnpausedAction->isChecked() && fi->fileState() != FileInfo::Paused ) ||
         ( !m_filter.isEmpty() && !fi->fileName().toLower().contains( m_filter.toLower() ) ) )
        del = true;

    DownloadFile* df = downloadFileList.value(fn);
    if ( df )
    {
        if ( del ) {
            delete downloadFileList.take( fn );
        }
        else
        {
            df->flush();
            df->refresh();
            del = true;
        }

        if ( completed )
            KNotification::event(QStringLiteral("KMLDonkeyDownloadFinished"), i18n("Download completed"), fi->fileName().toHtmlEscaped(), QStringLiteral("kmldonkey"));
    }

    if ( !del )
    {
        df = new DownloadFile( downloadView, fn );
        downloadFileList.insert( fn, df );

        if ( KMLDonkey::App->enableAutorename )
            autoRenameFile( fn, fi->fileName() );
        return;
    }

    switch ( fi->fileState() )
    {
        case FileInfo::Complete:
            del = false;
            break;

        default:
            del = true;
            break;
    }

    DownloadedFile* foo = downloadedFileList.value(fn);
    if ( foo )
    {
        if ( del )
            delete downloadedFileList.take( fn );
        else
        {
            foo->refresh();
            return;
        }
    }

    if ( !del )
    {
        foo = new DownloadedFile( downloadedView, fn );
        downloadedFileList.insert( fn, foo );
    }
}

void DownloadPage::fileAdded( int fn, bool downloadstarted )
{
    fileUpdated(fn);
    if ( downloadstarted )
    {
        FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo( fn );
        if ( fi )
            KNotification::event(QStringLiteral("KMLDonkeyDownloadAdded"), i18n("Download added"), fi->fileName().toHtmlEscaped(), QStringLiteral("kmldonkey"));
    }
}

void DownloadPage::fileRemoved( int fn )
{
    delete downloadFileList.take( fn );
    delete downloadedFileList.take( fn );
}

void DownloadPage::autoRenameFile( int filenum, const QString& filename )
{
    QString name = filename;
    QList<QStringList>::Iterator it;
    for ( it = KMLDonkey::App->autorenameList.begin(); it != KMLDonkey::App->autorenameList.end(); ++it )
    {
        if ( (*it).count() < 2 || (*it)[0].isEmpty() )
            continue;
        name.replace( QRegularExpression( (*it)[0] ), (*it)[1] );
    }
    if ( name != filename && ! name.isEmpty() )
    {
        fileRenamed( filenum, name );
        //qDebug() << "autorename: |" << filename << "| => |" << name << "|";
    }
}

void DownloadPage::actionHidePaused( bool hide )
{
    hidePausedAction->setChecked( hide );
    updateDownloadView();
}

void DownloadPage::actionHideUnpaused( bool hide )
{
    hideUnpausedAction->setChecked( hide );
    updateDownloadView();
}

void DownloadPage::actionSortDownloadingFirst( bool b )
{
    sortDownloadingFirstAction->setChecked( b );
    // TODO: downloadView->sort();
}

void DownloadPage::actionSortPausedLast( bool b )
{
    sortPausedLastAction->setChecked( b );
    // TODO: downloadView->sort();
}
