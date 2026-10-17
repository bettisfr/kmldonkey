/* -*- c++ -*-
 *
 * statspageovernet.h
 *
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

#ifndef __kmldonkey_statspageovernet_h__
#define __kmldonkey_statspageovernet_h__

#include <QScrollArea>
#include <QSplitter>
#include <QHideEvent>
#include <QShowEvent>

#include "autobox.h"

class QTextBrowser;
class QTreeWidget;

#include "statspage.h"
#include "donkeyprotocol.h"

class StatsPageOvernetCallback : public ConsoleCallbackInterface
{
public:
    StatsPageOvernetCallback(StatsPageOvernet *p);
    ~StatsPageOvernetCallback();

    void callback(const QString& cmd, const QString& res);

private:
    QPointer<StatsPageOvernet> page;
};


class StatsPageOvernet : public QScrollArea
{
    Q_OBJECT
public:
    StatsPageOvernet(QWidget *parent);

public:
    void parseOvmd4(const QString &res);
    void parseOvstat(const QString &res);
    void parseOvstore(const QString &res);

public slots:
    void slotUpdate();

protected:
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent *);

private:
    void clearItems();
    void addText( const QString &text );
    void addItem( QWidget * item );


    VBoxWidget               * box;
    QList<QWidget *>    items;
};

#endif
