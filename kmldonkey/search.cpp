#include <KConfigGroup>
/* -*- c++ -*-
 *
 * search.cpp
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
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

#include <QLabel>
#include <QSplitter>
#include <QShowEvent>
#include <QTimer>
#include <QCheckBox>
#include <QLayout>
#include <QClipboard>
#include <QRegExp>
#include <QMenu>
#include <QList>
#include <QMenu>

#include <QDebug>
#include <KLocalizedString>
#include <kconfig.h>
#include <QIcon>
#include <klineedit.h>
#include <QPushButton>
#include <kcombobox.h>
#include <QAction>
#include <kactioncollection.h>
#include <QMimeDatabase>
#include <kmessagebox.h>
#include <QTabWidget>
#include <kxmlguifactory.h>

#include "kmldonkey.h"
#include "search.h"
#include "prefs/prefs.h"

#include <searchquery.h>
#include <searchinfo.h>
#include <network.h>

#include <KCoreAddons>

SearchPage::SearchPage( QWidget *parent )
    : VBoxWidget(parent)
    , KMLDonkeyPage()
    , ClipboardHelper()
{
    setObjectName("searchPage");
    setMargin( 0 );
    setSpacing( 0 );
    splitter = new QSplitter( this );
    splitter->setChildrenCollapsible(false);

    searchcore = new SearchCore( splitter, this );
	splitter->setStretchFactor( splitter->indexOf( searchcore), 0 );

    VBoxWidget* rightBox = new VBoxWidget( splitter );
    splitter->setStretchFactor(splitter->indexOf(rightBox), 1);
    ed2kBox = new HBoxWidget( rightBox );
    ed2kBox->setContentsMargins( 0 ,  0 ,  0 ,  0 );
    ed2kBox->setSpacing( 0 );
    QLabel* ed2kLabel = new QLabel( i18n( "Submit URL:" ), ed2kBox );
    ed2kEntry = new KLineEdit( ed2kBox );
    ed2kEntry->setObjectName("ed2kEntry");
    ed2kLabel->setBuddy( ed2kEntry );
    connect( ed2kEntry, SIGNAL( returnPressed() ), SLOT( submitED2K() ) );

    resultTabs = new QTabWidget( rightBox );
    resultTabs->setDocumentMode(true);
    resultTabs->setTabsClosable( true );
    resultTabs->setMovable( true );
    resultTabs->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(resultTabs, &QWidget::customContextMenuRequested, this, [this](const QPoint &point){tabContextMenu(resultTabs->mapToGlobal(point));});
    connect(resultTabs, &QTabWidget::tabCloseRequested, this, [this](int index){closeSearch(resultTabs->widget(index));});
    connect(resultTabs, &QTabWidget::currentChanged, this, [this](int index){currentTabChanged(resultTabs->widget(index));});

    connect( KMLDonkey::App->donkey, SIGNAL( searchUpdated( int,QList<ResultInfo*> ) ), this, SLOT( searchUpdated( int, QList<ResultInfo*> ) ) );
}

SearchPage::~SearchPage()
{
    qDeleteAll( Results );
    Results.clear();
}

void SearchPage::handleGenericAction( const QString& action )
{
    if (action == "copy_url") copySearchToClipboard(URL);
    else if (action == "copy_html") copySearchToClipboard(HTML);
    else if (action == "copy_hash") copySearchToClipboard(Hash);
    else if (action == "search_download") actionDownload();
    else if (action == "search_force_download") actionForceDownload();
    else if (action == "search_download_as") actionDownloadAs();
}

void SearchPage::deactivatePageActions()
{
    searchcore->deactivateActions();
}

QStringList SearchPage::supportedGenericActions()
{
    QStringList actions;
    SearchResult *sr = (SearchResult*)resultTabs->currentWidget();
    if (sr && sr->resultView->hasFocus() && !sr->resultView->selectedItems().isEmpty()) {
        actions.append("copy_url");
        actions.append("copy_html");
        actions.append("copy_hash");
        actions.append("search_download");
        actions.append("search_force_download");
        actions.append("search_download_as");
    }
    if( KMLDonkey::App->donkey->isConnected() )
        searchcore->activateActions();
    return actions;
}

void SearchPage::plugGenericActions(QObject* object, const char* slot)
{
    connect(this, SIGNAL(genericActionsChanged(KMLDonkeyPage*)), object, slot);
}

void SearchPage::pleaseUpdateActions()
{
    emit genericActionsChanged(static_cast<KMLDonkeyPage*>(this));
}

void SearchPage::setupActions( KActionCollection* actionCollection )
{
    QAction* action = new QAction( QIcon::fromTheme("system-search"), i18n( "Activate Search Page" ), this );
    action->setIconText(i18n("Search"));
    connect(action, SIGNAL(triggered()), this, SLOT(actionActivatePage()));
    actionCollection->addAction("activate_page_search", action);

    action = new QAction( QIcon::fromTheme("dialog-close"), i18n( "Close Current Search" ), this );
    action->setIconText( i18n( "Close" ) );
    connect(action, SIGNAL(triggered()), this, SLOT(closeCurrentSearch()));
    actionCollection->addAction("search_close", action);

    action = new QAction( QIcon::fromTheme("dialog-close"), i18n( "Close All Searches" ), this );
    action->setIconText( i18n( "Close All" ) );
    connect(action, SIGNAL(triggered()), this, SLOT(closeAllSearches()));
    actionCollection->addAction("search_closeall", action);
}

void SearchPage::configurePrefsDialog(KMLDonkeyPreferences* prefs)
{
    prefs->searchPage->activateNewTabsCheckbox->setChecked(activateNewTabs);
    prefs->searchPage->closeTabsOnDisconnectCheckbox->setChecked(closeTabsOnDisconnect);
    prefs->searchPage->showNumbersOnTabsCheckbox->setChecked(showNumbersOnTabs);
    prefs->searchPage->showSubmitURLCheckbox->setChecked(showSubmitURL);
}

void SearchPage::applyPreferences(KMLDonkeyPreferences* prefs)
{
    activateNewTabs = prefs->searchPage->activateNewTabsCheckbox->isChecked();
    closeTabsOnDisconnect = prefs->searchPage->closeTabsOnDisconnectCheckbox->isChecked();
    showNumbersOnTabs = prefs->searchPage->showNumbersOnTabsCheckbox->isChecked();

    if (showSubmitURL != prefs->searchPage->showSubmitURLCheckbox->isChecked()) {
        showSubmitURL = prefs->searchPage->showSubmitURLCheckbox->isChecked();
        if (showSubmitURL)
            ed2kBox->show();
        else
            ed2kBox->hide();
        ed2kBox->updateGeometry();
    }

    QHash<int, SearchResult *>::const_iterator it = Results.begin();
    for ( ; it != Results.end(); ++it ) {
        it.value()->setListFont(KMLDonkey::App->listFont);
        resultTabs->setTabText( resultTabs->indexOf( it.value() ), getTabLabel(it.value()));
    }
}

QString SearchPage::getTabLabel(SearchResult *tab)
{
    if (! showNumbersOnTabs) 
		return tab->getTabLabel();
    return tab->getTabLabel() + " (" + QString::number(tab->resultView->topLevelItemCount()) + ")";
}

void SearchPage::saveState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "SearchPage" );
    const QList<int> sizes = splitter->sizes();
    group.writeEntry("Splitter", sizes);
    group.writeEntry("SidebarWidth", sidebarRestorePending ? restoredSidebarWidth : sizes.value(0, 340));
    group.writeEntry("activateNewTabs", activateNewTabs);
    group.writeEntry("closeTabsOnDisconnect", closeTabsOnDisconnect);
    group.writeEntry("showNumbersOnTabs", showNumbersOnTabs);
    group.writeEntry("showSubmitURL", showSubmitURL);
    searchcore->saveState(conf);
}

void SearchPage::restoreState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "SearchPage" );

    QList<int> searchsplitter = group.readEntry( "Splitter", QList<int>() );
    restoredSidebarWidth = group.readEntry("SidebarWidth", searchsplitter.value(0, 340));
    if (restoredSidebarWidth <= 0)
        restoredSidebarWidth = 340;
    sidebarRestorePending = true;

    activateNewTabs = group.readEntry("activateNewTabs", true);
    closeTabsOnDisconnect = group.readEntry("closeTabsOnDisconnect", true);
    showNumbersOnTabs = group.readEntry("showNumbersOnTabs", true);

    showSubmitURL = group.readEntry("showSubmitURL", false);
    if (showSubmitURL)
        ed2kBox->show();
    else
        ed2kBox->hide();
    ed2kBox->updateGeometry();

    searchcore->restoreState(conf);
}

void SearchPage::showEvent(QShowEvent *event)
{
    VBoxWidget::showEvent(event);
    if (!sidebarRestorePending)
        return;

    // Wait until the page has its actual window geometry. Restoring while it
    // is hidden scales the saved sizes against the small construction size.
    QTimer::singleShot(0, this, [this]() {
        if (!sidebarRestorePending || !isVisible())
            return;
        const int available = splitter->width() - splitter->handleWidth();
        splitter->setSizes({restoredSidebarWidth, qMax(1, available - restoredSidebarWidth)});
        sidebarRestorePending = false;
    });
}

void SearchPage::clear()
{
    searchcore->clear();
    searchcore->deactivateActions();
    if(closeTabsOnDisconnect) 
		closeAllSearches();
}

void SearchPage::submitED2K()
{
    KMLDonkey::App->donkey->submitUrl(ed2kEntry->text());
    ed2kEntry->clear();
}

void SearchPage::searchUpdated( int searchnum, QList<ResultInfo*> resultInfos )
{
    SearchResult *sr = Results[searchnum];
    if ( !sr )
        return;

    Q_FOREACH(ResultInfo *resultInfo, resultInfos)
        sr->addItem( resultInfo );

    resultTabs->setTabText( resultTabs->indexOf( sr ), getTabLabel( sr ) );
}

void SearchPage::stopSearch( int id )
{
    if ( KMLDonkey::App->donkey->isConnected() )
        KMLDonkey::App->donkey->stopSearch( id );

    delete Results.take( id );
}

SearchResult* SearchPage::addSearchResult(int searchNum, SearchQuery* query, const QString& tablabel)
{
    SearchResult *sr = new SearchResult(resultTabs, tablabel, searchNum);
        connect(sr->resultView, SIGNAL(itemSelectionChanged()), SLOT(pleaseUpdateActions()));
        connect(sr->resultView, SIGNAL(gotFocus()), SLOT(pleaseUpdateActions()));
    delete Results.take( searchNum );
    Results[searchNum] = sr;

    sr->setListFont(KMLDonkey::App->listFont);
    sr->setSearchLabel(query->getQuerystring(true));
    resultTabs->addTab(sr, QIcon::fromTheme("system-search"), getTabLabel(sr)); //QIcon::fromTheme("dialog-close")
    if (activateNewTabs) 
		resultTabs->setCurrentIndex(resultTabs->count()-1);
    return sr;
}

void SearchPage::stopAllSearches()
{
    if (! KMLDonkey::App->donkey->isConnected())
        return;

    QHash<int, SearchResult *>::const_iterator it;
    for ( it = Results.begin(); it != Results.end(); ++it )
        KMLDonkey::App->donkey->stopSearch( it.key() );
}

void SearchPage::closeSearch(QWidget* widget)
{
    int id = static_cast<SearchResult*>(widget)->searchNo();
    stopSearch(id);
    pleaseUpdateActions();
}

void SearchPage::currentTabChanged( QWidget* w )
{
    Q_UNUSED(w);
}

void SearchPage::closeAllSearches()
{
    while (resultTabs->count()) {
        int id = static_cast<SearchResult*>( resultTabs->currentWidget() )->searchNo();
        stopSearch(id);
    }
    pleaseUpdateActions();
}

void SearchPage::actionDownload()
{
    SearchResult *sr = (SearchResult*)resultTabs->currentWidget();
    if (sr) 
		sr->DownloadSelectedItems(false, false);
}

void SearchPage::actionForceDownload()
{
    SearchResult *sr = (SearchResult*)resultTabs->currentWidget();
    if (sr) 
		sr->DownloadSelectedItems(true, false);
}

void SearchPage::actionDownloadAs()
{
    SearchResult *sr = (SearchResult*)resultTabs->currentWidget();
    if (sr) 
		sr->DownloadSelectedItems(false, true);
}

void SearchPage::copySearchToClipboard(ClipFormat format)
{
    SearchResult *sr = (SearchResult*)resultTabs->currentWidget();
    if (! sr)
        return;
    QStringList sl;
    QList<QTreeWidgetItem *> list = sr->resultView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        SearchResultItem *item = (SearchResultItem *) i;
        sl.append( item->getName() ); // filename
        sl.append( item->getEd2kHash() ); // filehash
        sl.append( QString::number((long)item->getSize()) ); // filesize
    }
    copyToClipboard(sl, format);
}

void SearchPage::actionActivatePage()
{
    KMLDonkey::App->activatePage( this );
}

void SearchPage::tabContextMenu( const QPoint& pt )
{
    QMenu *pop = dynamic_cast<QMenu*>( (KMLDonkey::App->factory())->container("search_actions", KMLDonkey::App) );
    if (!pop)
        KMLDonkey::App->showBadInstallDialog();
    else
        pop->popup(pt);
}

void SearchPage::closeCurrentSearch()
{
    if (resultTabs->currentWidget())
        closeSearch( resultTabs->currentWidget() );
}


