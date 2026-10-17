/* -*- c++ -*-
 *
 * searchcore.cpp
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006      Christian Muehlhaeuser <chris@chris.de>
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
#include <QLabel>
#include <QLayout>
#include <QRegExp>
#include <QScrollArea>
#include <QFrame>

#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <QDebug>
#include <klineedit.h>
#include <KLocalizedString>
#include <QSpinBox>
#include <QPushButton>
#include "autobox.h"

#include "kmldonkey.h"
#include "search.h"
#include "searchcore.h"
#include "searchcorewidgets.h"

#include <network.h>
#include <searchinfo.h>
#include <searchquery.h>

long int
filesizeStr2Int( QString filesize )
{
    QString s = filesize.toLower().trimmed();
    if ( s.isEmpty() )
        return 0;

    long int result;
    bool ok;

    if ( s.endsWith( "mb" ) || s.endsWith( "kb" ) )
        s.remove( s.length() - 1, 1 );

    if ( s.endsWith("m") ) // megabytes
        result = s.left( s.length() - 1 ).trimmed().toLong() * 1048576;
    else
        if ( s.endsWith( "k" ) ) // kilobytes
            result = s.left( filesize.length() - 1 ).trimmed().toLong() * 1024;
        else
            if ( s.endsWith( "b" ) ) // bytes
                result = s.left( filesize.length() - 1 ).trimmed().toLong();
            else
            {
                // only numbers means bytes as well
                result = s.toLong( &ok );
                if ( !ok )
                    result = 0;
            }

    return ( result < 0 ? 0 : result );
}


SearchCore::SearchCore( QWidget *parent, SearchPage *page )
    : VBoxWidget( parent )
    , m_page( page )
    , m_form( 0 )
{
    setObjectName("searchCore");
    searchNum = 0;
    maxHits = 500;
    searchType = 0;

    setMargin( 0 );
    setSpacing( 2 );

    // Form selector. The different forms are defined in
    // DonkeyProtocol::definedSearches(). So, they are core-depended
    // and configurable that way.
    m_formcombo = new KComboBox( this );
    connect( m_formcombo, SIGNAL( activated( int ) ), this, SLOT( activateFormCombo( int ) ) );

    // The scrollview is used to display the formwidgets.
    m_formscroll = new QScrollArea( this );
    m_formscroll->setFrameShape( QFrame::NoFrame );
    m_formscroll->setWidgetResizable( true );
    setStretchFactor( m_formscroll, 1 );

    // Following widgets are form independed. They are
    // always around.
    VBoxWidget* box = new VBoxWidget( this );
    box->setContentsMargins( 2 ,  2 ,  2 ,  2 );
    box->setSpacing( 0 );

    // Max Hits
    HBoxWidget *mhbox = new HBoxWidget( box );
    mhbox->setContentsMargins( 0 ,  0 ,  0 ,  0 );
    mhbox->setSpacing( 0 );
    new QLabel( i18n( "Max hits:" ), mhbox );
    maxhitsEdit = new QSpinBox( mhbox );
    mhbox->setStretchFactor( maxhitsEdit, 1 );
    maxhitsEdit->setRange(1, 10000);

    maxhitsEdit->setValue( maxHits );

    // Type
    HBoxWidget *typebox = new HBoxWidget( box );
    typebox->setContentsMargins( 0 ,  0 ,  0 ,  0 );
    typebox->setSpacing( 0 );
    new QLabel(i18nc( "search type", "Type:" ), typebox );
    searchtypeCombo = new KComboBox( typebox );
    typebox->setStretchFactor( searchtypeCombo, 1 );
    searchtypeCombo->insertItem(0, i18nc( "remote search", "Remote" ) );
    searchtypeCombo->insertItem(1, i18nc( "local search", "Local" ) );
    searchtypeCombo->insertItem(2, i18nc( "subscription search", "Subscribe" ));
    searchtypeCombo->setCurrentIndex( searchType );

    // Network
    HBoxWidget *netbox = new HBoxWidget( box );
    netbox->setContentsMargins( 0 ,  0 ,  0 ,  0 );
    netbox->setSpacing( 0 );
    new QLabel( i18n( "Network:" ), netbox );
    searchnetCombo = new KComboBox( netbox );
    netbox->setStretchFactor( searchnetCombo, 1 );
    searchnetCombo->insertItem( 0, i18n( "All Networks" ) );
    searchnetCombo->setCurrentItem( 0 );
    connect( KMLDonkey::App->donkey, SIGNAL( networkUpdated( int ) ), this, SLOT( setNetworks( int ) ) );

    // Start search button
    startButton = new QPushButton( i18nc( "start a search", "Search" ), box );
    startButton->setEnabled( false );
    connect( startButton, SIGNAL( clicked() ), this, SLOT( startSearch() ) );

    // Update form
    if ( KMLDonkey::App->donkey->isConnected() )
        updatePages();

    connect( KMLDonkey::App->donkey, SIGNAL( definedSearchesUpdated() ), this, SLOT( updatePages() ) );
}


void SearchCore::restoreState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "SearchCore" );
    activateDef = group.readEntry( "DefinedSearch", activateDef );
    maxHits = group.readEntry( "maxHits", maxHits );
    maxhitsEdit->setValue( maxHits );

    int i = group.readEntry( "Type", searchType );
    if ( i >= 0 && i <= 2 )
    {
        searchType = i;
        searchtypeCombo->setCurrentIndex( searchType );
    }

    searchNetwork = group.readEntry<QString>( "Network", QString() );
}


void SearchCore::saveState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "SearchCore" );
    group.writeEntry( "DefinedSearch", activateDef );
    group.writeEntry( "maxHits", maxhitsEdit->value() );
    group.writeEntry( "Type", searchtypeCombo->currentIndex() );
    group.writeEntry( "Network", searchnetCombo->currentText() );
}


void SearchCore::clear()
{
    clearFormWidget();
    clearFormQueries();
    searchnetCombo->setCurrentItem( 0 );

    for ( int i = searchnetCombo->count() - 1; i > 0; i-- )
        searchnetCombo->removeItem( i );
}


void SearchCore::activateActions()
{
    startButton->setEnabled( true );
}


void SearchCore::deactivateActions()
{
    startButton->setEnabled( false );
}


void SearchCore::clearFormWidget()
{
    delete m_form;
    m_form = 0;
}


void SearchCore::clearFormQueries()
{
    m_formqueries.clear();
    m_formcombo->clear();
}


void SearchCore::setNetworks( int no )
{
    Network *net = KMLDonkey::App->donkey->findNetworkNo( no );
    if ( !net )
        return;

    for ( int i = searchnetCombo->count() - 1; i > 0; i-- )
        if ( searchnetCombo->itemText( i ) == net->networkName() )
        {
            // already on the list?
            if ( !net->networkEnabled() )
                searchnetCombo->removeItem( i );
            return;
        }

    if ( !net->networkEnabled() )
        return;
    if ( !( net->networkFlags() & Network::NetworkHasSearch ) )
        return;

    searchnetCombo->insertItem( searchnetCombo->count(), net->networkName() );
    if ( searchNetwork == net->networkName() )
        searchnetCombo->setCurrentIndex( searchnetCombo->count() - 1 );
}


void SearchCore::updatePages()
{
    clearFormWidget();
    clearFormQueries();

    int defitem = -1;
    QString defsearch = KMLDonkey::App->donkey->definedSearch();
    QMap<QString, SearchQuery*> searches = KMLDonkey::App->donkey->definedSearches();

    for ( QMap<QString, SearchQuery*>::iterator it = searches.begin(); it != searches.end(); ++it )
    {
        m_formcombo->insertItem( m_formcombo->count(), it.key() );
        m_formqueries[ it.key() ] = it.value();
        if ( it.key() == activateDef || (it.key() == defsearch && defitem < 0 ) )
            defitem = m_formcombo->count() - 1;
    }

    if ( defitem >= 0 )
    {
        m_formcombo->setCurrentIndex( defitem );
        activateFormCombo( defitem );
    }
}


void SearchCore::activateFormCombo( int index )
{
    if ( m_formcombo->currentIndex() != index )
        return;

    clearFormWidget();
    SearchQuery *query = m_formqueries[ m_formcombo->currentText() ];
    if ( !query )
        return;

    activateDef = m_formcombo->currentText();
    m_form = new VBoxWidget( m_formscroll );
    m_form->setContentsMargins( 2 ,  2 ,  2 ,  2 );
    m_form->setSpacing( 0 );

    // Cause we don't handle SearchQuery->operation() at this time we just
    // have to create a temp SearchCoreWidget and delete it after the job
    // is done (means after the SearchCoreWidget-structur gots build).
    // The parent-QWidget m_form gots passed through the temp widget to it's
    // first valid child. So, we don't have to worry about this one ;)
    SearchCoreWidget* tmpwidget = new SearchCoreWidget( this, m_form, 0 );
    m_formwidget = tmpwidget->setQuery( query );
    delete tmpwidget;
    tmpwidget = 0;

    // It's possible that m_formwidget contains NULL here. That would mean
    // there isn't any valid SearchCoreWidget-child under m_formwidget and
    // the SearchQuery is just invalid.
    if ( !m_formwidget )
    {
        clearFormWidget();
        return;
    }

    m_form->setStretchFactor( new QWidget( m_form ), 1 );
    m_formscroll->setWidget( m_form );
}


void SearchCore::startSearch()
{
    if ( !KMLDonkey::App->donkey->isConnected() || !m_form )
        return;

    SearchQuery *query = m_formwidget->getQuery();
    if ( !query )
    {
        qDebug() << "SearchCore::startSearch() Invalid SearchQuery!";
        return;
    }
    qDebug() << "SearchCore::startSearch() " << query->getQuerystring(true);

    // Max hits
    maxHits = maxhitsEdit->value();

    // Network
    int network = 0;
    if ( searchnetCombo->currentIndex() > 0 )
    {
        Network *nw = KMLDonkey::App->donkey->findNetworkName(searchnetCombo->currentText());
        if (nw) network = nw->networkNo();
    }

    // Type
    DonkeyProtocol::SearchType type;
    switch ( searchtypeCombo->currentIndex() )
    {
        case 1:
            type = DonkeyProtocol::LocalSearch;
            break;

        case 2:
            type = DonkeyProtocol::SubscribeSearch;
            break;

        default:
            type = DonkeyProtocol::RemoteSearch;
            break;
    }

    // Start the search only if searchtermins like keywords or artist got
    // defined cause to search e.g. for filesize only doesn't make sense.
    QString label = m_formwidget->getDescription();
    if ( !label.isEmpty() )
    {
        // Get a free new searchNum
        do { ++searchNum; }
        while (KMLDonkey::App->donkey->findSearchNo(searchNum) != 0);
        // Add a new searchtabresult
        m_page->addSearchResult( searchNum, query, label );
        // Let's start the search now
        KMLDonkey::App->donkey->startSearch( searchNum, query, maxHits, type, network );
    }

    // free the temp SearchQuery created above
    delete query;
}

/*  TODO
    // Keywords
    QString keywords = keywordsEdit->text().trimmed();
    if(! keywords.isEmpty()) {
        QRegExp rxkeywords("([\\s]+|^)(\\-|\\+)[\\s]*");
        keywords.replace(rxkeywords, "\\1\\2");
        QStringList keywordlist = QStringList::split(" ", keywords);
        for(QStringList::Iterator it = keywordlist.begin(); it != keywordlist.end(); ++it) {
            if((*it).startsWith("-"))
                query = new QueryAndNot(query ? query : qand, new QueryKeywords((QString&)QString(), (*it).mid(1)));
            else
                qand->append(new QueryKeywords((QString&)QString(), (*it).startsWith("+") ? (*it).mid(1) : (*it)));
        }
        tablabel = keywords;
    }
*/

