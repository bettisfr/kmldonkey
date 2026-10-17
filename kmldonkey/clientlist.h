/* -*- c++ -*-
 *
 * clientlist.h
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

#ifndef __kmldonkey_clientlist_h__
#define __kmldonkey_clientlist_h__

#include "infolist.h"
#include "availability.h"

#include <QShowEvent>
#include <QHash>

class SourceItem;
class KConfigGroup;

class ClientList : public InfoList
{
    Q_OBJECT

public:

    explicit ClientList(int file, QWidget* parent = 0, const char* name = 0);
    virtual ~ClientList();

public slots:

    void fileSourceUpdated(int fileno, int source);
    void fileSourceRemoved(int fileno, int source);
    void clientUpdated(int client);
    void clientRemoved(int client);
    void loseCache();
	
protected slots:

    void addToFriends(Qt::MouseButtons,Qt::KeyboardModifiers);

protected:

    virtual void showEvent(QShowEvent*);
	virtual void contextMenuEvent( QContextMenuEvent * cm );

private:

    bool showsources;
    int fn;
    QHash<int, SourceItem *> sources;
};

class SourceItem : public InfoItem, public AvailabilityRenderer
{
public:
    SourceItem(ClientList* parent, int fileno, int clientno);

    int clientNo() const;

    virtual QString xtext(int column) const;
	virtual bool paint( QPainter * p, const QStyleOptionViewItem & option, const QModelIndex & index );
	virtual void refresh();
	

private:
    int cn;
};


#endif // __kmldonkey_clientlist_h__
