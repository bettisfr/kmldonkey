#include <QLocale>
/* -*- c++ -*-
 *
 * searchtabresult.cpp
 *
 * Copyright (C) 2003-2004      Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006           Christian Muehlhaeuser <chris@chris.de>
 * Copyright (C) 2009           Gioacchino Mazzurco <gmazzurco89@gmail.com>
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

#include <network.h>
#include <searchinfo.h>
#include <searchquery.h>

#include <QLabel>
#include <QMenu>
#include <QLayout>
#include <QPainter>
#include <QRegExp>
#include <QTimer>
#include <QFrame>
#include <QList>

#include <QDebug>
#include <KCoreAddons>
#include <kiconloader.h>
#include <QInputDialog>
#include <klineedit.h>
#include <QTreeWidget>
#include <KLocalizedString>
#include <QMimeDatabase>
#include <kxmlguifactory.h>
#include "autobox.h"

#include "imageloader.h"
#include "kmldonkey.h"
#include "searchcore.h"
#include "searchtabresult.h"
#include "downloadrenamer.h"

SearchResultItem::SearchResultItem( QTreeWidget *parent, int num, const ResultInfo *searchinfo )
    : InfoItem( parent,num )
{
    number = num;
    name = searchinfo->resultName();
    names = searchinfo->resultNames();
    filesize = searchinfo->resultSize();

    availability = 0;
    const QMap<QString,QVariant>& tags = searchinfo->resultTags();
    if( tags.contains( "availability" ) )
        availability = tags["availability"].toInt();

    alreadydone = searchinfo->resultAlreadyDone();
    Network* nw = KMLDonkey::App->donkey->findNetworkNo( searchinfo->resultNetwork() );
    nwName = (nw) ? nw->networkName() : QString::number( searchinfo->resultNetwork() );
    format = searchinfo->resultFormat();
    comment = searchinfo->resultComment();
    uids = searchinfo->resultUids();
    md4 = searchinfo->resultUid( "ed2k" );

    if ( nw )
        setIcon( 0, ImageLoader::imageForNetwork( nw ) );
    refresh();
}


SearchResultItem::~SearchResultItem()
{
}


QString SearchResultItem::xtext( int col ) const
{
    switch ( col )
    {
        case 0:
            return name;
        case 1:
            return FileInfo::humanReadableSize( filesize );
        case 2:
            return QLocale().toString(double(availability), 'f', 0);
        case 3:
            return nwName;
        case 4:
            return QMimeDatabase().mimeTypeForFile( name , QMimeDatabase::MatchExtension).comment();
        case 5:
            return format;
        case 6:
            return comment;
        case 7:
            return uids.first();

        default:
            return "ERROR";
    }
}


double SearchResultItem::numeric( int col ) const
{
    switch ( col )
    {
        case 1:
            return (double)filesize;
        case 2:
            return (double)availability;

        default:
            return 0.0;
    }
}


bool SearchResultItem::isNumeric( int col ) const
{
    switch ( col )
    {
        case 1:
        case 2:
            return true;

        default:
            return false;
    }
}


void /*virtual*/ SearchResultItem::refresh()
{
	InfoItem::refresh();
	
	QColor colText = treeWidget()->palette().color( QPalette::Text );
    if ( KMLDonkey::App->coloredViews )
    {
        if ( alreadydone )
            colText = KMLDonkey::App->colorSearchAlreadyDone;
        else
            if ( availability < (int32)KMLDonkey::App->searchThreshold )
                colText = KMLDonkey::App->colorSearchFewSources;
            else
                colText = KMLDonkey::App->colorSearchManySources;
	
		int nc = treeWidget()->columnCount();
		for ( int c = 0; c < nc; c++ )
			setForeground( c, QBrush( colText ) );
    }
}
		

SearchResultView::SearchResultView( QWidget *parent, const char *name )
    : InfoList( parent, name, true )
    //, QToolTip( viewport() )
{
    addColumn( i18n( "File name" ), 320 );
    addColumn( i18n( "Size" ), 60 );
    addColumn( i18n( "Availability" ) );
    addColumn( i18n( "Network" ) );
    addColumn( i18n( "File type" ), 320 );
    addColumn( i18n( "Format" ) );
    addColumn( i18n( "Comment" ), 200 );
    addColumn( i18n( "Hash" ), 320 );

    initialise();
}


SearchResult::SearchResult( QWidget *parent, const QString &tablabel, int sno )
    : SearchTabInterface( parent, "searchResult", tablabel, sno )
    , visibleCount( 0 )
{
    statusBox = new HBoxWidget(this);

    HBoxWidget *filterbox = new HBoxWidget(statusBox);
    new QLabel(i18n("Filter"), filterbox);
    filterEdit = new KLineEdit(filterbox);
    connect(filterEdit, SIGNAL( textChanged(const QString &) ), this, SLOT( filterChanged() ));
    filterTimer = new QTimer(this);
    connect(filterTimer, SIGNAL( timeout() ), this, SLOT( filterTimerDone() ));

    statusLabel = new QLabel(i18nc("zero of zero search results", "0/0"), statusBox);
    statusLabel->setFrameStyle(QFrame::Sunken + QFrame::Box);
    searchLabel = new KSqueezedTextLabel(statusBox);
    searchLabel->setFrameStyle(QFrame::Sunken + QFrame::Box);
    statusBox->setStretchFactor(searchLabel,1);

    resultView = new SearchResultView( this, "searchResult" );
    connect(resultView, SIGNAL(contextMenu(QTreeWidgetItem*, const QPoint&)),
            this, SLOT(contextSearchResult(QTreeWidgetItem*, const QPoint&)));
    connect(resultView, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(doubleClick(QTreeWidgetItem*)));
}


SearchResult::~SearchResult()
{
}


void SearchResult::contextSearchResult(QTreeWidgetItem*,const QPoint& pt)
{
    QMenu *pop = dynamic_cast<QMenu*>( (KMLDonkey::App->factory())->container("search_actions", KMLDonkey::App) );
    if (!pop)
        KMLDonkey::App->showBadInstallDialog();
    else
        pop->popup(pt);
}


void SearchResult::doubleClick(QTreeWidgetItem*)
{
    DownloadSelectedItems(false, true);
}


void SearchResult::addItem( const ResultInfo *searchinfo )
{
    if ( m_hashes.indexOf( searchinfo->resultUid( "ed2k" ) ) >= 0 )
        return;
    m_hashes.append( searchinfo->resultUid( "ed2k" ) );

    SearchResultItem* item = new SearchResultItem( resultView, searchinfo->resultNo(), searchinfo );
    bool visible = ! filterItem( item );
    if ( visible ) 
		visibleCount++;
    item->setHidden( !visible );

    setStatusLabel();
}


void SearchResult::DownloadSelectedItems(bool force, bool askForFilename)
{
    QStringList names;
    QString filename;
    bool ok;
    QList<QTreeWidgetItem *> list = resultView->selectedItems();
    foreach( QTreeWidgetItem * i, list )
    {
        SearchResultItem *item = (SearchResultItem *) i;
        filename = item->text(0);
        if (askForFilename || ! filename.length()) {
            filename = QInputDialog::getText( this, i18n("Download As"),
                                              i18n("Choose a filename for the new download:"),
                                              QLineEdit::Normal, filename, &ok );
            if (! ok)
                continue;
        }
        names.clear();
        names.append(filename);
        if (filename != item->text(0))
            new DownloadRenamer(item->getEd2kHash(), filename);
        KMLDonkey::App->donkey->startDownload(names, item->getNum(), force);
    }
}


void SearchResult::filterChanged()
{
    filterTimer->stop();
    if (resultView->topLevelItemCount() > 0 ) 
	{
		filterTimer->setSingleShot( true );
		filterTimer->start(500);
	}
}


void SearchResult::filterTimerDone()
{
    visibleCount = 0;
    QString str = filterEdit->text().toLower().trimmed().simplified();
    filters = str.split(" " );
    QTreeWidgetItemIterator it(resultView);
    bool visible;
    for ( ; *it; ++it ) {
		QTreeWidgetItem * item = *it;
        visible = ! filterItem( (SearchResultItem*) item );
        if(visible) 
			visibleCount++;
        item->setHidden( !visible );
        if (item->isSelected() && ! visible) 
			item->setSelected(false);
    }
    setStatusLabel();
}


void SearchResult::setStatusLabel()
{
    statusLabel->setText( QString("%1/%2").arg(QLocale().toString(double(visibleCount), 'f', 0))
                                          .arg(QLocale().toString(double(resultView->topLevelItemCount()), 'f', 0)) );
}


bool SearchResult::filterItem(SearchResultItem *item)
{
    bool minsize;
    int64 size;
    for (int i = 0; i < filters.count(); i++) {
        minsize = filters[i].startsWith(">");
        if (minsize || filters[i].startsWith("<")) { // filter max/min filesize
            size = filesizeStr2Int( filters[i].right(filters[i].length() - 1) );
            if (minsize) {
                if (item->getSize() <= size) 
					return true;
            }
            else {
                if (item->getSize() >= size) 
					return true;
            }
        }
        else { // filter filename
            if (filters[i].startsWith("~")) { // regular expression
                QRegExp rx( filters[i].right(filters[i].length() - 1) );
                if (rx.indexIn( item->text(0) ) < 0) 
					return true;
            }
            else if (! item->text(0).contains(filters[i], Qt::CaseInsensitive))
                return true;
        }
    }
    return false;
}


