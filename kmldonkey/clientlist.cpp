/* -*- c++ -*-
 *
 * clientlist.cpp
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

#include <KLocalizedString>
#include <QDebug>
#include <KSharedConfig>
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QShowEvent>
#include <QPalette>
#include <QHeaderView>
#include "clientlist.h"
#include "kmldonkey.h"
#include "clientinfo.h"

#include "network.h"

SourceItem::SourceItem(ClientList* parent, int fileno, int clientno)
    : InfoItem( parent, fileno)
    , AvailabilityRenderer(fileno, clientno)
{
    cn = clientno;
    if (!KMLDonkey::App->donkey->findClientNo(cn))
        KMLDonkey::App->donkey->refreshClientInfo(cn);
}

int SourceItem::clientNo() const 
{ 
	return cn; 
}

QString SourceItem::xtext(int column) const
{
    ClientInfo* cl = KMLDonkey::App->donkey->findClientNo(cn);
    if (!cl) {
        if (column) 
			return QString();
        return i18n("Unknown");
    }

    switch (column) {
        case 0: // Name
            return cl->clientName();
        case 1: { // Network
            Network* net = KMLDonkey::App->donkey->findNetworkNo(cl->clientNetwork());
            if (!net) 
				return i18n("Unknown");
            return net->networkName();
        }
        case 2: // Type
            switch (cl->clientType()) {
            case ClientInfo::NormalClient:
                return i18n("Normal");
            case ClientInfo::FriendClient:
                return i18n("Friend");
            case ClientInfo::ContactClient:
                return i18n("Contact");
            default:
                return i18n("Unknown");
            }
        case 3: // Kind
            return cl->clientKind();
        case 4: // State
            switch (cl->clientState()) {
            case ClientInfo::NotConnected:
            case ClientInfo::NotConnected2: 
				return i18n("Not connected");
            case ClientInfo::Connecting: 
				return i18n("Connecting");
            case ClientInfo::Initiating: 
				return i18n("Initiating");
            case ClientInfo::Downloading: 
				return i18n("Downloading");
            case ClientInfo::Connected:
            case ClientInfo::Connected3: 
				return i18n("Connected");
            case ClientInfo::Connected2: 
				return i18n("Queued: %1",cl->clientQueuePosition());
            case ClientInfo::NewHost: 
				return i18n("New host");
            case ClientInfo::Removed: 
				return i18n("Removed");
            case ClientInfo::Blacklisted: 
				return i18n("Blacklisted");
            default: 
				return i18n("Unknown: %1",cl->clientState());
            }
        case 6: // Software
            if (cl->clientSoftware().isNull())
                return i18n("Unknown");
            else {
                QString software(cl->clientSoftware());
                if (!cl->clientEmuleMod().isEmpty())
                    software += i18nc("eMule mod appended to client software tag", " (%1)", cl->clientEmuleMod());
                    if (!cl->clientRelease().isEmpty())
                        software += i18nc("Client software release appended to client software tag", " %1", cl->clientRelease());
                    return software;
            }
        case 7: { // Currently downloading file
            if (!cl->clientDownloading()) 
				return QString();
            FileInfo* fi = KMLDonkey::App->donkey->findDownloadFileNo(cl->clientDownloading());
            if (!fi) {
                qDebug() << "Invalid file ID from downloading client: " << cl->clientDownloading();
                return QString();
            }
            return fi->fileName();
        }
        default:
            return QString();
    }
}

bool /*virtual*/ SourceItem::paint( QPainter * p, const QStyleOptionViewItem & option, const QModelIndex & index )
{
	int col = index.column();
	if( treeWidget()->isColumnHidden( col ) == false )
	{
		if (col == 5) 
		{
			p->setBrush( Qt::red );
			p->setPen( Qt::green );
			paintAvailability( *p, option.rect );
			return true;
		}
	}
	
	return InfoItem::paint( p, option, index );
}

void /*virtual*/ SourceItem::refresh()
{
	InfoItem::refresh();
	
	QColor colText = treeWidget()->palette().color( QPalette::Text );
    if (KMLDonkey::App->coloredViews) {
        ClientInfo* cl = KMLDonkey::App->donkey->findClientNo(cn);
        if (cl) {
            switch (cl->clientState()) {
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



ClientList::ClientList(int fileno, QWidget* parent, const char* name)
    : InfoList(parent, name)
{
    setObjectName(name ? name : "ClientList");
    fn = fileno;
    showsources = true;

    QHeaderView * h = header();
    if( h )
    {
		h->setSortIndicatorShown( true );
		h->setSectionResizeMode( QHeaderView::Interactive );
    }
    setFont(KMLDonkey::App->listFont);

	addColumn( i18n("Name") );
	addColumn( i18n("Network") );
	addColumn( i18n("Type") );
	addColumn( i18n("Location") );
	addColumn( i18n("State") );
	addColumn( i18n("Availability") );
	addColumn( i18n("Software") );
	addColumn( i18n("Currently downloading") );
    
    initialise();
}

ClientList::~ClientList()
{
    qDeleteAll( sources );
}


void /*virtual*/ ClientList::contextMenuEvent( QContextMenuEvent * cm )
{
	SourceItem * item = dynamic_cast<SourceItem *>( itemAt( cm->pos() ) );
	if (!item) 
		return;
    
	QPoint p = cm->globalPos();
    ClientInfo* cl = KMLDonkey::App->donkey->findClientNo( item->clientNo() );
    if (!cl) 
		return;
    
	QMenu* pop = new QMenu(this);
		pop->setTitle(i18n("Sources"));
		
		QAction * a = new QAction( QIcon::fromTheme("edit_add"), i18n("&Add to Friends"), this );
			connect( a , SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), 
						this, SLOT(addToFriends(Qt::MouseButtons,Qt::KeyboardModifiers)) );
		pop->addAction( a );
    pop->popup(p);
}

void ClientList::addToFriends(Qt::MouseButtons,Qt::KeyboardModifiers)
{
    foreach( QTreeWidgetItem * i, selectedItems() ) {
        SourceItem* item = dynamic_cast<SourceItem*>(i);
        Q_ASSERT(item);
        KMLDonkey::App->donkey->addClientFriend(item->clientNo());
    }
}

void ClientList::showEvent(QShowEvent*)
{
    if (! showsources) 
		return;
    showsources = false;

    FileInfo* const fi = KMLDonkey::App->donkey->findDownloadFileNo(fn);
    if (fi) {
        int i = 0;
        setUpdatesEnabled(false);
        setSortingEnabled( false );
        const QHash<int,QByteArray>& src = fi->fileSources();
        QHash<int,QByteArray>::ConstIterator it = src.begin();
        for ( ; it != src.end(); ++it) {
            ++i;
            delete sources.take( it.key() );
			
			SourceItem * item = new SourceItem( this, fn, it.key() );
            sources.insert( it.key(), item );
			item->refresh();
        }
        setSortingEnabled( true );
        setUpdatesEnabled(true);
    }

    connect(KMLDonkey::App->donkey, SIGNAL(fileSourceUpdated(int,int)), this, SLOT(fileSourceUpdated(int,int)));
    connect(KMLDonkey::App->donkey, SIGNAL(fileSourceRemoved(int,int)), this, SLOT(fileSourceRemoved(int,int)));
    connect(KMLDonkey::App->donkey, SIGNAL(clientUpdated(int)), this, SLOT(clientUpdated(int)));
    connect(KMLDonkey::App->donkey, SIGNAL(clientRemoved(int)), this, SLOT(clientRemoved(int)));
}

void ClientList::fileSourceUpdated(int fileno, int source)
{
    if (fileno != fn) 
		return;
    SourceItem * cl = sources[source];
    if (cl == NULL) 
	{
		cl = new SourceItem(this, fn, source);
        sources.insert( source, cl );
	}
	cl->refresh();
}

void ClientList::fileSourceRemoved(int fileno, int source)
{
    if (fileno != fn)
        return;
    delete sources.take( source );
}

void ClientList::clientUpdated(int client)
{
    if (!sources[client]) 
        return;
    
    ClientInfo* const cl = KMLDonkey::App->donkey->findClientNo(client);
    switch (cl->clientState()) {
        case ClientInfo::Removed:
            delete sources.take(client) ;
            break;
        default:
            sources[client]->refresh();
            break;
    }
}

void ClientList::clientRemoved(int client)
{
    delete sources.take( client );
}

void ClientList::loseCache()
{
    QTreeWidgetItemIterator it(this);
	while( *it )
	{
        SourceItem* cl = dynamic_cast<SourceItem*>( *it );
		if( cl )
			cl->loseCache();
	
		++it;
	}
}

