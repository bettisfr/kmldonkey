#include <QLocale>
/* -*- c++ -*-
 *
 * infolist.cpp
 *
 * Copyright (C) 2003, 2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006       Christian Muehlhaeuser <chris@chris.de>
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

#include <QDebug>
#include <KSharedConfig>
#include <KLocalizedString>
#include <QTreeWidget>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <QMimeDatabase>
#include <QMenu>
#include <ktoggleaction.h>
#include <kiconloader.h>

#include <QVariant>
#include <QPainter>
#include <QCursor>
#include <QPixmap>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QIcon>
#include <QApplication>
#include <QLineEdit>

#include <sys/types.h>
#include <time.h>

#include "infolist.h"
#include "kmldonkey.h"
#include "imageloader.h"

#include "searchinfo.h"
#include "clientinfo.h"
#include "serverinfo.h"
#include "fileinfo.h"
#include "network.h"

InfoList::InfoList( QWidget* parent, const char* name, bool headervisiblepopup )
    : QTreeWidget( parent ), 
    m_pDelegate( NULL )
{
    setObjectName(name ? name : "InfoList");
    m_name = name;

    setFont(KMLDonkey::App->listFont);
	
	setSelectionBehavior( QTreeWidget::SelectRows );
	setSelectionMode( QTreeWidget::ExtendedSelection );
    setAllColumnsShowFocus( true );

	QHeaderView * h = header();
	if( h )
	{
		h->setSortIndicatorShown( true );
		h->setSectionResizeMode( QHeaderView::Fixed );
	}
	setSortingEnabled( true );
	
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    setMinimumSize( 1, 1 );
	
    m_columnCount = 0;
    m_sortInterval = 0;
    m_sortTimer = new QTimer( this );
    connect( m_sortTimer, SIGNAL( timeout() ), this, SLOT( sortTimer() ) );

    if ( headervisiblepopup )
    {
        m_headerMenu = new QMenu( this );
        m_headerMenu->setTitle( i18n( "Show" ) );
    }
    else
        m_headerMenu = 0;
	
	// set up delegate:
	m_pDelegate = new ItemDelegate( this );
	setItemDelegate( m_pDelegate );
}


InfoList::~InfoList()
{
    saveLayout();
	
	if( m_pDelegate )
	{
		setItemDelegate( NULL );
		
		delete m_pDelegate;
		m_pDelegate = NULL;
	}
}


void
InfoList::initialise()
{
	// initialize columns:
	setColumnCount( m_columnCount );
	
	QTreeWidgetItem * hi = headerItem();
	QHeaderView * h = header();
    for ( int i = 0; i < m_columnCount; i++ )
    {
		hi->setText( i, m_columns[i] );
		h->setSectionResizeMode( i, m_columnResizeMode[i] );
		h->resizeSection( i, m_columnWidth[i] );
    }
    
    // event filter, used to detect mouse r-click on header:
    // (see http://www.qtcentre.org/threads/25622-Capture-mouse-event-on-QHeaderView)
    h->viewport()->installEventFilter( this );

	// restore settings:
    KSharedConfigPtr conf = KSharedConfig::openConfig();
    if ( !m_name.isEmpty() )
    {
        restoreLayout( conf.data(), m_name );
    }
}


void
InfoList::saveLayout()
{
    KSharedConfigPtr conf = KSharedConfig::openConfig();
    if ( m_name.isEmpty() )
        return;

    saveLayout( conf.data(), m_name );
}


void InfoList::saveLayout( KSharedConfig * cfg, const QString &name )
{
    KConfigGroup group = cfg->group( name );

	// save column widths;
    QStringList widths;
    const int colCount = model()->columnCount();
    for ( int i = 0; i < colCount; ++i )
	{
		int w = 0;
		if( isColumnHidden( i ) )
			w = m_columnWidth[i];	// hidden columns return 0, so avoid it & store default width for it
		else
			w = columnWidth( i );
		
        widths << QString::number( w );
	}

    group.writeEntry( "ColumnWidths", widths );
	
	// save hide state:
    if ( m_headerMenu )
    {
        QStringList hideHeader;
        for ( int i = 0; i < m_columnCount; i++ )
		{
			if( isColumnHidden( i ))
                hideHeader.append( m_columns[i] );
		}
			
        group.writeEntry( "HideHeader", hideHeader );
    }
    
	// save header state:
	QHeaderView * hv = header();
	if( hv )
	{
		group.writeEntry( "HeaderState", hv->saveState() );
	}
}

void InfoList::restoreLayout( const KSharedConfig * cfg, const QString &name )
{
	KConfigGroup group = cfg->group( name );
	QStringList cols = group.readEntry( "ColumnWidths", QStringList() );
	hideHeader = group.readEntry( "HideHeader", QStringList() );

	// restore column widths & hide state:
	m_columnActions.clear();
	
	QStringList::ConstIterator it = cols.constBegin();
	const QStringList::ConstIterator itEnd = cols.constEnd();
	int i = 0;
	for ( ; it != itEnd; ++it, ++i )
	{
		setColumnWidth( i, (*it).toInt() );
		
        bool hidecol = m_headerMenu && hideHeader.indexOf( m_columns[i] ) >= 0;
        if ( m_headerMenu )
        {
            // m_headerMenu->insertItem( m_columns[i], i );
            QAction * a = new KToggleAction( m_columns[i], this );
				a->setCheckable( true );
				a->setChecked( !hidecol );
				connect( a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), 
							this, SLOT(toggleColumnVisible(Qt::MouseButtons, Qt::KeyboardModifiers)) );
            m_headerMenu->addAction( a );
			m_columnActions[a] = i;
        }
        if ( hidecol )
            hideColumn( i );
	}

	// restore header state:
	QHeaderView * hv = header();
	if( hv )
	{
		QByteArray state = group.readEntry( "HeaderState", QByteArray() );
		if( state.size() > 0 )
		{
			hv->restoreState( state );
		}
	}
	
}


QList<QTreeWidgetItem *>
InfoList::selectedItems() const
{
    QList<QTreeWidgetItem *> list;
    QTreeWidgetItemIterator it( const_cast<InfoList*>(this), QTreeWidgetItemIterator::Selected|QTreeWidgetItemIterator::NotHidden );
	while( *it )
        list.append( *it++ );
    return list;
}


int
InfoList::addColumn( const QString& label, int width, QHeaderView::ResizeMode resizeMode, bool hideColumn )
{
    m_columns.insert( m_columnCount, label );
    m_columnWidth.insert( m_columnCount, width );
    m_columnResizeMode.insert( m_columnCount, resizeMode );
    if ( hideColumn ) 
		hideHeader.append( label );
    return m_columnCount++;
}


int
InfoList::sortInterval()
{
    return m_sortInterval;
}


void
InfoList::setSortInterval( int sec )
{
    m_sortInterval = sec;
    if ( m_sortTimer->isActive() )
    {
        if ( m_sortInterval > 0 )
            m_sortTimer->start( m_sortInterval * 1000 );
        else
            m_sortTimer->stop();
    }
    else
        if ( m_sortInterval > 0 )
            m_sortTimer->start( m_sortInterval * 1000 );
}

void
InfoList::sortTimer()
{
	sortItems( sortColumn(), Qt::AscendingOrder ); // workaround
}



void
InfoList::showColumn( int c )
{
    setColumnHidden( c, false );
	
	// update all items for this column:
	QTreeWidgetItemIterator it( this );
	while( *it )
	{
		InfoItem * item = dynamic_cast<InfoItem *>( *it++ );
		if( item )
			item->refreshColumn( c );
	}
}


void
InfoList::hideColumn( int c )
{
    setColumnHidden( c, true );
}


void InfoList::toggleColumnVisible( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers )
{
	Q_UNUSED( buttons );
	Q_UNUSED( modifiers );
	
	QAction * a = dynamic_cast<QAction *>( sender() );
	if( a )
	{
		int c = m_columnActions[a];
		if( isColumnHidden(c)) 
			showColumn( c );
		else
			hideColumn( c );
	}
}


bool
InfoList::eventFilter( QObject *watched, QEvent *event )
{
    if ( watched == header()->viewport() )
    {
        switch ( event->type() )
        {
            case QEvent::MouseButtonPress:
                if ( m_headerMenu && static_cast<QMouseEvent*>(event)->button() == Qt::RightButton )
                    m_headerMenu->popup( QCursor::pos() );
                break;

            default:
                break;
        }
    }

    return QTreeWidget::eventFilter( watched, event );
}


void
InfoList::focusInEvent( QFocusEvent* event )
{
    QTreeWidget::focusInEvent( event );
    emit gotFocus();
    emit focusChanged();
}


void
InfoList::focusOutEvent( QFocusEvent* event )
{
    QTreeWidget::focusInEvent( event );
    emit lostFocus();
    emit focusChanged();
}


void /*virtual*/ InfoList::contextMenuEvent( QContextMenuEvent * cm )
{
	QTreeWidgetItem * item = itemAt( cm->pos() );
	QPoint p = cm->globalPos();
	emit contextMenu( item, p );
}


InfoList::ItemDelegate * InfoList::itemDelegate() const
{
	return dynamic_cast<InfoList::ItemDelegate *>( QTreeWidget::itemDelegate() );
}


void
InfoItemCacheEntry::update( const QString& str )
{
    m_str = str;
    m_isNum = false;
}


void
InfoItemCacheEntry::update( const QString& str, double num )
{
    m_str = str;
    m_num = num;
    m_isNum = true;
}


int
InfoItemCacheEntry::compare( const InfoItemCacheEntry* i ) const
{
    // We assume compares are never made against an entry with a different type.
    if ( m_isNum )
        return ( m_num < i->m_num ) ? -1 : ( m_num > i->m_num ) ? 1 : 0;
    else
        return m_str.toLower().localeAwareCompare( i->m_str.toLower() );
}


//
// ItemDelegate
//

InfoList::ItemDelegate::ItemDelegate( QObject * parent /*= NULL*/ )
	: QStyledItemDelegate( parent )
{
}
	
InfoList * InfoList::ItemDelegate::parentList() const
{
	return dynamic_cast<InfoList *>( parent() );
}

void /*virtual*/ InfoList::ItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	bool done = false;
	
    painter->save();
	
	InfoList * pList = parentList();
	if( pList )
	{
		InfoItem * item = dynamic_cast<InfoItem *>( pList->itemFromIndex( index ) );
		if( item )
			done = item->paint( painter, option, index );
	}
	
	if( done == false )
		QStyledItemDelegate::paint( painter, option, index );

    painter->restore();
}


QWidget * /*virtual*/ InfoList::ItemDelegate::createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	InfoList * pList = parentList();
	if( pList == NULL)
		return NULL;
	
	InfoItem * item = dynamic_cast<InfoItem *>( pList->itemFromIndex( index ) );
	if( item == NULL )
		return NULL;
		
	QWidget * w = NULL;
	if( index.column() == 0 )	// only rename of item is allowed
	{
		// TODO: instantiate a KLineEdit, not a QLineEdit
		w = QStyledItemDelegate::createEditor( parent, option, index );
		if( w )
		{
			QLineEdit * edit = dynamic_cast<QLineEdit *>( w );
			if( edit )
			{
				edit->setText( item->text( index.column() ) );
			}
		}
	}
	
	return w;
}
				
void /*virtual*/ InfoList::ItemDelegate::setEditorData ( QWidget * editor, const QModelIndex & index ) const
{
	// just ignore: gets called everytime InfoItem::refresh() is externally called, so it just messes up editor...
	Q_UNUSED( editor );
	Q_UNUSED( index );
}
				
void /*virtual*/ InfoList::ItemDelegate::setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const
{
	Q_UNUSED( model );
	
	InfoList * pList = parentList();
	if( pList == NULL)
		return;
	
	InfoItem * item = dynamic_cast<InfoItem *>( pList->itemFromIndex( index ) );
	if( item == NULL )
		return;

	// set data into the item:
	QLineEdit * edit = dynamic_cast<QLineEdit *>( editor );
	if( edit )
	{
		item->setData( index.column(), Qt::DisplayRole, edit->text() );
		item->refresh();
	}
}



InfoItem::InfoItem( QTreeWidget *parent, int file )
    : QTreeWidgetItem( parent )
{
    fileno = file;
}


InfoItem::~InfoItem()
{
    qDeleteAll( cache );
	cache.clear();
}

int InfoItem::width() const
{
	QRect rc = treeWidget()->visualItemRect( this );	// is this efficient...?
	return rc.width();
}

int InfoItem::height() const
{
	QRect rc = treeWidget()->visualItemRect( this );	// is this efficient...?
	return rc.height();
}


void
InfoItem::refresh()
{
    InfoList* list = (InfoList*) treeWidget();
	int nc = list->columnCount();
    for ( int c = 0; c < nc; c++ )
    {
		if( !list->isColumnHidden( c ) )
		{
			refreshColumn( c, false );
		}
    }

	emitDataChanged();
}

void InfoItem::refreshColumn( int c , bool notify /*= true*/ )
{
	InfoItemCacheEntry* e = cache[c];
	if( e == NULL )
	{
		e = new InfoItemCacheEntry();
		cache[c] = e;
	}

	if ( isNumeric( c ) )
		e->update( xtext( c ), numeric( c ) );
	else
		e->update( xtext( c ) );
	
	if( notify )
		emitDataChanged();
}

QString
InfoItem::text( int column ) const
{
    const InfoItemCacheEntry* e = cacheEntry( column );
    if ( !e )
        return QString();

    return e->text();
}

void
InfoItem::setFileNo( int file )
{
    fileno = file;
    refresh();
}


int
InfoItem::fileNo()
{
    return fileno;
}


double
InfoItem::numeric( int ) const
{
    return 0.0;
}


bool
InfoItem::isNumeric( int ) const
{
    return false;
}


const InfoItemCacheEntry*
InfoItem::cacheEntry( int column ) const
{
    return cache [column];
}


bool InfoItem::operator < (const QTreeWidgetItem &other ) const	// virtual
{
	int col = treeWidget()->sortColumn();
	
	const InfoItemCacheEntry * cmpEntry1 = cacheEntry( col );
	
	const InfoItemCacheEntry * cmpEntry2 = NULL;
	const InfoItem * cmpItem = dynamic_cast<const InfoItem *>( &other );
	if( cmpItem )
		cmpEntry2 = cmpItem->cacheEntry( col );
	
	int i = 0;
	if( cmpEntry1 && cmpEntry2 )
		i = cmpEntry1->compare( cmpEntry2 );
	
    if ( i == 0 )
    {
        // stable sorting
        // stable sorting means: if two items compare equal, thir order after sorting should equal their order before
		/* TODO: QTreeWidgetItem doesn't have an itemPos() equivalent, so we need to find a way to keep stable sorting...
        i = ( this->itemPos() < static_cast<InfoItem*>(item)->itemPos() ? 1 : -1 );
        return (ascending) ? -1 * i : i;
		*/
    }
	
	return ( i < 0 );
}


bool /*virtual*/ InfoItem::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index )
{
	(void) painter;
	(void) option;
	(void) index;

	// Do nothing by default: if this function returns false, then item delegate will perform
	// default painting for the item.
	return false;
}


QVariant InfoItem::data( int column, int role ) const
{
	QVariant result;
	
	if( role == Qt::DisplayRole )
	{
		const InfoItemCacheEntry* e = cacheEntry( column );
		if ( e )
			result = e->text();
	}
	else
		result = QTreeWidgetItem::data( column, role );

    return result;
}


void /*virtual*/ InfoItem::setData( int column, int role, const QVariant &value )
{
	QTreeWidgetItem::setData( column, role, value );
}


DownloadFile::DownloadFile( QTreeWidget *parent, int file )
    : InfoItem( parent, file )
    , AvailabilityRenderer( file )
{
    FileInfo* it = KMLDonkey::App->donkey->findDownloadFileNo( file );
    if ( it )
    {
        Network *n = KMLDonkey::App->donkey->findNetworkNo( it->fileNetwork() );
        if ( n )
            setIcon( 0, QIcon( ImageLoader::imageForNetwork( n )) );
    }

    m_lastFlush = 0;
	
	setFlags( flags() | Qt::ItemIsEditable );
		
    refresh();
}


void /*virtual*/ DownloadFile::setData( int column, int role, const QVariant &value )
{
	bool done = false;
	if( role == Qt::DisplayRole )
	{
		if( column == 0 )
		{
			KMLDonkey::App->donkey->renameFile( fileno, value.toString() );
			done = true;
		}
	}
	else
		
	if( done == false )
		InfoItem::setData( column, role, value );
}


QString /*virtual*/ DownloadFile::xtext( int column ) const
{
    QString foo;
    FileInfo* it = KMLDonkey::App->donkey->findDownloadFileNo( fileno );
    if ( !it )
    {
        if ( !column )
            return i18n( "Unknown file" );

        return i18nc( "signifies absence of data in list columns", "-" );
    }

    switch ( column )
    {
        case 0: // name
            return it->fileName();
        case 1://3: // speed
            return ( it->fileSpeed() > 0 ) ? FileInfo::humanReadableSpeed( it->fileSpeed() ) : QString();
        case 2://6: // percent remaining
            return QString("%1%").arg( QLocale().toString(double(it->fileSize() ? ( it->fileDownloaded() * 100.0 ) / it->fileSize() : 0.0), 'f', 1) );
        case 3://5: // downloaded received
            return FileInfo::humanReadableSize( it->fileDownloaded() );
        case 4: // size
            return FileInfo::humanReadableSize( it->fileSize() );
        case 5://7: // eta
            return FileInfo::calculateETA( it );
        case 6://1: // Priority
            return FileInfo::humanReadablePriority( it->filePriority() );
        case 7://2: // status
            switch ( it->fileState() )
            {
                case FileInfo::Downloading:
                    if ( !it->fileSpeed() )
                        return i18nc( "looking for file sources", "Looking" );
                    return i18nc( "file is being downloaded", "Downloading" );

                case FileInfo::Paused: return i18nc( "file is paused", "Paused" );
                case FileInfo::Complete: return i18nc( "file is complete", "Complete" );
                case FileInfo::Shared: return i18nc( "file is being shared", "Shared" );
                case FileInfo::Cancelled: return i18nc( "file has been canceled", "Canceled" );
                case FileInfo::New: return i18nc( "new file added to list", "New" );
                case FileInfo::Aborted: return i18nc( "file has been aborted with a reason", "Aborted: %1", it->fileAbortedMsg() );
                case FileInfo::Queued: return i18nc( "file has been queued", "Queued" );
                default: return i18nc( "unknown host state (unknown ID code)", "Unknown (%1)", it->fileState() );
            }

            return i18nc( "unknown file state", "Unknown" );
        case 8: { // network
            Network *network = KMLDonkey::App->donkey->findNetworkNo(it->fileNetwork());
            return network ? network->networkName() : i18n("Unknown network");
        }
        case 9: // availability
            return QString();
        case 10: // hash
            return it->fileUid();
        case 11: // last seen
            return FileInfo::humanReadableTime( (time_t)it->fileLastSeen(), true );
        case 12: // age
        {
            time_t age = time( 0 ) - it->fileAge();
            return FileInfo::humanReadableTime( age, true );
        }
        case 13: // filetype
            return QMimeDatabase().mimeTypeForFile( it->fileName() , QMimeDatabase::MatchExtension).comment();

        default:
            return "ERROR!";
    }
}


bool DownloadFile::operator < (const QTreeWidgetItem &other ) const // virtual
{
    bool sortdownloadingfirst = KMLDonkey::App->sortDownloadingFirst();
    bool sortpausedlast = KMLDonkey::App->sortPausedLast();

    if ( sortdownloadingfirst || sortpausedlast )
    {
        FileInfo* it = KMLDonkey::App->donkey->findDownloadFileNo( fileno );
        FileInfo* it2 = KMLDonkey::App->donkey->findDownloadFileNo( ((DownloadFile*) &other )->fileNo() );
	
	if (it == NULL || it2 == NULL)
	  return 0;

        if ( sortdownloadingfirst )
        {
            if ( it->fileSpeed() < it2->fileSpeed() && it->fileSpeed() == 0 )
                return false;
            else if ( it->fileSpeed() > it2->fileSpeed() && it2->fileSpeed() == 0 )
                return true;
        }

        if ( sortpausedlast )
        {
            if ( it->fileState() == FileInfo::Paused && it->fileState() != it2->fileState() )
                return false;
            if ( it2->fileState() == FileInfo::Paused && it->fileState() != it2->fileState() )
                return true;
        }
    }

    return InfoItem::operator < ( other );
}


double DownloadFile::numeric( int col ) const
{
    FileInfo* it = KMLDonkey::App->donkey->findDownloadFileNo( fileno );
    if ( !it )
        return 0.0;

    switch ( col )
    {
        case 6://1:
            return (double)it->filePriority();
        case 4:
            return (double)it->fileSize();
        case 3://5:
            return (double)it->fileDownloaded();
        case 2://6:
            return it->fileSize() ? ( (double)it->fileDownloaded() * 100.0 ) / (double)it->fileSize() : 0.0;
        case 5://7:
            return FileInfo::calculateETANumeric( it );
        case 1://3:
            return it->fileSpeed();
        case 9:
            return (double)it->fileSources().size();
        case 11:
            return (double)( (time_t)it->fileLastSeen() );
        case 12:
            return (double)( time(0) - (time_t)it->fileAge() );

        default:
            return 0.0;
    }
}


bool DownloadFile::isNumeric( int col ) const
{
    switch ( col )
    {
        case 6://1:
        case 1://3:
        case 4:
        case 3://5:
        case 2://6:
        case 5://7:
        case 9:
        case 11:
        case 12:
            return true;

        default:
            return false;
    }
}


bool /*virtual*/ DownloadFile::paint( QPainter * p, const QStyleOptionViewItem & option, const QModelIndex & index )
{
	int col = index.column();
	if( treeWidget()->isColumnHidden( col ) == false )
	{
		if ( col == 9 )
		{
			p->setBrush( Qt::red );
			p->setPen( Qt::green );
			paintAvailability( *p, option.rect );
			return true;
		}

		if ( col == 2  && KMLDonkey::App->displayProgressbar )
		{
			FileInfo* it = KMLDonkey::App->donkey->findDownloadFileNo( fileno );
			if( it )
			{
				QStyleOptionProgressBar opt;
				opt.rect = option.rect;
				opt.minimum = 0;
				opt.maximum = 100;
				int progress = it->fileSize() ? (int)( ( (double)it->fileDownloaded() * 100.0 ) / (double)it->fileSize() ) : 0 ;
				opt.progress = progress;
				opt.text = QString("%1%").arg( progress );
				opt.textVisible = true;
				QApplication::style()->drawControl( QStyle::CE_ProgressBar, &opt, p, 0 );
				return true;
			}
		}
	}

	// else just peform default painting:
	return InfoItem::paint( p, option, index );
}

void /*virtual*/ DownloadFile::refresh()
{
	InfoItem::refresh();
	
	// adjust text color:
	QColor colText = treeWidget()->palette().color( QPalette::Text );
    if ( KMLDonkey::App->coloredViews )
    {
        FileInfo* it = KMLDonkey::App->donkey->findDownloadFileNo( fileno );
        if ( it )
        {
            switch ( it->fileState() )
            {
                case FileInfo::Downloading:
                    if ( it->fileSpeed() > 0 )
                        colText = KMLDonkey::App->colorDownloadDownloading;
                    else if ( it->fileLastSeen()/86400 >= 100 ) // 100 days or older
                        colText = KMLDonkey::App->colorDownloadNotAvailable;
                    else
                        colText = KMLDonkey::App->colorDownloadLooking;
                    break;

                case FileInfo::Paused:
                    colText = KMLDonkey::App->colorDownloadPaused;
                    break;

                case FileInfo::Queued:
                    colText = KMLDonkey::App->colorDownloadQueued;
                    break;

                default:
                    break;
            }
        }

		int nc = treeWidget()->columnCount();
		for ( int c = 0; c < nc; c++ )
			setForeground( c, QBrush( colText ) );
    }
}


void DownloadFile::flush()
{
    // Hardcoded 10 second limit on availability refresh,
    // I doubt this is an unreasonable minimum for anyone.
    if ( m_lastFlush + 10 < time( 0 ) )
    {
        loseCache();
        m_lastFlush = time( 0 );
    }
}


DownloadedFile::DownloadedFile( QTreeWidget *parent, int file )
    : InfoItem( parent, file )
{
    FileInfo* it = KMLDonkey::App->donkey->findDownloadedFileNo( file );
    if ( !it )
        it = KMLDonkey::App->donkey->findDownloadFileNo( fileno );

    if ( it )
    {
        Network *n = KMLDonkey::App->donkey->findNetworkNo( it->fileNetwork() );
        if ( n )
            setIcon( 0, QIcon(ImageLoader::imageForNetwork( n )) );
    }

    refresh();
}


QString DownloadedFile::xtext( int column ) const
{
    FileInfo* it = KMLDonkey::App->donkey->findDownloadedFileNo( fileno );
    if ( !it )
        it = KMLDonkey::App->donkey->findDownloadFileNo( fileno );

    if ( !it )
    {
        if ( !column )
            return i18n( "Unknown file" );

        return i18nc( "signifies absence of data in list columns", "-" );
    }

    switch ( column )
    {
        case 0: // name
            return it->fileName();
        case 1: // size
            return FileInfo::humanReadableSize( it->fileSize() );
        case 2: // format
            return it->fileFormatInfo();
        case 3: // network
            return KMLDonkey::App->donkey->findNetworkNo( it->fileNetwork() )->networkName();
        case 4: // hash
            return it->fileUid();
        case 5: // filetype
            return QMimeDatabase().mimeTypeForFile( it->fileName() , QMimeDatabase::MatchExtension).comment();

        default:
            return "ERROR!";
    }
}


double DownloadedFile::numeric( int col ) const
{
    FileInfo* it = KMLDonkey::App->donkey->findDownloadedFileNo( fileno );
    if ( !it )
        it = KMLDonkey::App->donkey->findDownloadFileNo( fileno );
    if ( !it )
        return 0.0;

    switch ( col )
    {
        case 1:
            return (double)it->fileSize();

        default:
            return 0.0;
    }
}


bool DownloadedFile::isNumeric( int col ) const
{
    switch ( col )
    {
        case 1:
            return true;

        default:
            return false;
    }
}



ServerInfoItem::ServerInfoItem( QTreeWidget *parent, int file )
    : InfoItem( parent, file )
{
    ServerInfo* it = KMLDonkey::App->donkey->findServerNo( fileno );
    if ( it )
    {
        Network *n = KMLDonkey::App->donkey->findNetworkNo( it->serverNetwork() );
        if ( n )
            setIcon( 0, QIcon(ImageLoader::imageForNetwork( n )) );
    }

	setFlags( flags() | Qt::ItemIsEditable );
    refresh();
}


void /*virtual*/ ServerInfoItem::setData( int column, int role, const QVariant &value )
{
	bool done = false;
	if( role == Qt::DisplayRole )
	{
		if( column == 0 )
		{
			KMLDonkey::App->donkey->renameServer( fileno, value.toString() );
			done = true;
		}
	}
	else
		InfoItem::setData( column, role, value );
}


QString /*virtual*/ ServerInfoItem::xtext( int column ) const
{
    ServerInfo* it = KMLDonkey::App->donkey->findServerNo( fileno );
    if ( !it )
    {
        if ( !column )
            return i18n( "Unknown server" );

        return i18nc( "signifies absence of data in list columns", "-" );
    }

    switch ( column )
    {
        case 0: // name
            return it->serverName();
        case 1: // network
            return KMLDonkey::App->donkey->findNetworkNo( it->serverNetwork() )->networkName();
        case 2: // status
            switch ( it->serverState() )
            {
                case ClientInfo::NotConnected:
                case ClientInfo::NotConnected2:
                    return i18nc( "not connected to host", "Not connected" );
                case ClientInfo::Connecting:
                    return i18nc( "connecting to host", "Connecting" );
                case ClientInfo::Initiating:
                    return i18nc( "initiating connection to host", "Initiating" );
                case ClientInfo::Downloading:
                    return i18nc( "downloading from host", "Downloading" );
                case ClientInfo::Connected:
                case ClientInfo::Connected2:
                case ClientInfo::Connected3:
                    return i18nc( "connected to host", "Connected" );
                case ClientInfo::NewHost:
                    return i18nc( "new host added to list", "New host" );
                case ClientInfo::Removed:
                    return i18nc( "host has been removed", "Removed" );
                case ClientInfo::Blacklisted:
                    return i18nc( "host is blacklisted", "Blacklisted" );

                default:
                    return i18nc( "unknown host state (unknown ID code)", "Unknown (%1)", it->serverState() );
            }
        case 3: // users
            return QLocale().toString(double(it->serverNUsers()), 'f', 0);
        case 4: // files
            return QLocale().toString(double(it->serverNFiles()), 'f', 0);
        case 5: // description
            return it->serverDescription();
        case 6: // address
            return it->serverAddress();
        case 7: // port
            return QString::number( it->serverPort() );
        case 8: // score
            return QString::number( it->serverScore() );
        case 9: // ID
            if ( it->serverState() == ClientInfo::Connected3 )
                return i18n( "high" );
            if ( it->serverState() == ClientInfo::Connected2 )
                return i18n( "normal" );
            if ( it->serverState() == ClientInfo::Connected )
                return i18n( "low" );
            return QString();
        case 10: // Preferred
            return it->serverPreferred() ? i18n( "Yes" ) : "";

        default:
            return "ERROR!";
    }
}


double ServerInfoItem::numeric( int col ) const
{
    ServerInfo* it = KMLDonkey::App->donkey->findServerNo( fileno );
    if ( !it )
        return 0.0;

    switch ( col )
    {
        case 3:
            return (double)it->serverNUsers();
        case 4:
            return (double)it->serverNFiles();
        case 7:
            return (double)it->serverPort();
        case 8:
            return (double)it->serverScore();

        default:
            return 0.0;
    }
}


bool ServerInfoItem::isNumeric( int col ) const
{
    switch ( col )
    {
        case 3:
        case 4:
        case 7:
        case 8:
            return true;

        default:
            return false;
    }
}

void /*virtual*/ ServerInfoItem::refresh()
{
	InfoItem::refresh();
	
	// adjust text color:
	QColor colText = treeWidget()->palette().color( QPalette::Text );
    if ( KMLDonkey::App->coloredViews )
    {
        ServerInfo* it = KMLDonkey::App->donkey->findServerNo( fileno );
        if ( it )
        {
            switch ( it->serverState() )
            {
                case ClientInfo::NotConnected:
                case ClientInfo::NotConnected2:
                    colText = KMLDonkey::App->colorServerNotConnected;
                break;

                case ClientInfo::Blacklisted:
                    colText = KMLDonkey::App->colorServerBlacklisted;
                break;

                case ClientInfo::Connecting:
                case ClientInfo::Initiating:
                    colText = KMLDonkey::App->colorServerConnecting;
                break;

                case ClientInfo::Connected:
                case ClientInfo::Connected2:
                case ClientInfo::Connected3:
                    colText = KMLDonkey::App->colorServerConnected;
                break;

                default:
                    break;
            }
        }
        
		int nc = treeWidget()->columnCount();
		for ( int c = 0; c < nc; c++ )
			setForeground( c, QBrush( colText ) );
    }
}



ClientItem::ClientItem( QTreeWidget *parent, int file )
    : InfoItem( parent, file )
{
    filesListed = false;
    ClientInfo* it = KMLDonkey::App->donkey->findClientNo( file );
    if ( it )
    {
        Network *n = KMLDonkey::App->donkey->findNetworkNo( it->clientNetwork() );
        if ( n )
            setIcon( 0, QIcon(ImageLoader::imageForNetwork( n ) ));
    }

    refresh();
}


void ClientItem::setFilesListed( bool listed )
{
    filesListed = listed;
    refresh();
}


QString ClientItem::xtext( int column ) const
{
    ClientInfo* it = KMLDonkey::App->donkey->findClientNo( fileno );
    if ( !it )
    {
        if ( !column )
            return i18n( "Unknown client" );

        return i18nc( "signifies absence of data in list columns", "-" );
    }

    switch ( column )
    {
        case 0: // name
            return it->clientName();
        case 1: // network
            return KMLDonkey::App->donkey->findNetworkNo( it->clientNetwork() )->networkName();
        case 2: // type
            switch ( it->clientType() )
            {
                case ClientInfo::NormalClient:
                    return i18nc( "a normal client", "Normal" );
                case ClientInfo::FriendClient:
                    return i18nc( "a client on the friend list", "Friend" );
                case ClientInfo::ContactClient:
                    return i18nc( "a client on the contact list", "Contact" );

                default:
                    return i18nc( "client type is unknown", "Unknown" );
            }
        case 3: // kind
            return it->clientKind();
        case 4: // state
            switch ( it->clientState() )
            {
                case ClientInfo::NotConnected:
                case ClientInfo::NotConnected2:
                    return i18nc( "not connected to host", "Not connected" );
                case ClientInfo::Connecting:
                    return i18nc( "connecting to host", "Connecting" );
                case ClientInfo::Initiating:
                    return i18nc( "initiating connection to host", "Initiating" );
                case ClientInfo::Downloading:
                    return filesListed ? i18nc( "this friend's file list is present", "Files listed" ) : i18nc( "downloading from host", "Downloading" );
                case ClientInfo::Connected:
                case ClientInfo::Connected3:
                    return filesListed ? i18nc( "this friend's file list is present", "Files listed" ) : i18nc( "connected to host", "Connected" );
                case ClientInfo::Connected2:
                    return filesListed ? i18nc( "this friend's file list is present", "Files listed" ) :
                                         i18nc( "we're at this position in client's queue", "Queued: %1" )
                                         .arg(QLocale().toString(it->clientQueuePosition()));
                case ClientInfo::NewHost:
                    return i18nc( "new host added to list", "New host" );
                case ClientInfo::Removed:
                    return i18nc( "host has been removed", "Removed" );
                case ClientInfo::Blacklisted:
                    return i18nc( "host is blacklisted", "Blacklisted" );

                default:
                    return i18nc( "unknown host state (unknown ID code)", "Unknown (%1)", it->clientState() );
            }

        default:
            return "ERROR!";
    }
}


void /*virtual*/ ClientItem::refresh()
{
	InfoItem::refresh();
	
	QColor colText = treeWidget()->palette().color( QPalette::Text );
    if ( KMLDonkey::App->coloredViews )
    {
        ClientInfo* cl = KMLDonkey::App->donkey->findClientNo( fileno );
        if ( cl )
        {
            switch ( cl->clientState() )
            {
                case ClientInfo::NotConnected:
                case ClientInfo::NotConnected2:
                    colText = KMLDonkey::App->colorSourceNotConnected;
                break;

                case ClientInfo::Connecting:
                case ClientInfo::Initiating:
                    colText = KMLDonkey::App->colorSourceConnecting;
                break;

                case ClientInfo::Downloading:
                    colText = KMLDonkey::App->colorSourceDownloading;
                break;

                case ClientInfo::Connected:
                case ClientInfo::Connected2:
                case ClientInfo::Connected3:
                    colText = KMLDonkey::App->colorSourceQueued;
                break;

                case ClientInfo::Blacklisted:
                    colText = KMLDonkey::App->colorSourceBlacklisted;
                break;

                default:
                    break;
            }
        }
        
		int nc = treeWidget()->columnCount();
		for ( int c = 0; c < nc; c++ )
			setForeground( c, QBrush( colText ) );
    }
}


ClientFile::ClientFile( QTreeWidget *parent, int file )
    : InfoItem( parent, file )
{
    const ResultInfo* it = KMLDonkey::App->donkey->findClientFile( file );
    if ( it )
    {
        Network *n = KMLDonkey::App->donkey->findNetworkNo( it->resultNetwork() );
        if ( n )
            setIcon( 0, QIcon(ImageLoader::imageForNetwork( n )) );
    }
    refresh();
}


QString ClientFile::xtext( int column ) const
{
    const ResultInfo* it = KMLDonkey::App->donkey->findClientFile( fileno );
    if ( !it )
    {
        if ( !column )
            return i18n( "Unknown file" );

        return i18nc( "signifies absence of data in list columns", "-" );
    }

    switch ( column )
    {
        case 0: // name
            return it->resultName();
        case 1: // network
        {
            Network *net = KMLDonkey::App->donkey->findNetworkNo( it->resultNetwork() );
            return net ? net->networkName() : QString::number( it->resultNetwork() );
        }
        break;

        case 2: // size
            return FileInfo::humanReadableSize( it->resultSize() );
        case 3: // format
            return it->resultFormat();
        case 4: // comment
            return it->resultComment();
        case 5: // hash
            return it->resultUid();

        default:
            return "ERROR!";
    }
}


double ClientFile::numeric( int col ) const
{
    const ResultInfo* it = KMLDonkey::App->donkey->findClientFile( fileno );
    if ( !it )
        return 0.0;

    switch ( col )
    {
        case 2:
            return (double)it->resultSize();

        default:
            return 0.0;
    }
}


bool ClientFile::isNumeric( int col ) const
{
    switch ( col )
    {
        case 2:
            return true;

        default:
            return false;
    }
}
