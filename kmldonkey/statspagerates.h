/* -*- c++ -*-
 *
 * statspagerates.h
 *
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

#ifndef __KMLDONKEY_STATSPAGERATES_H__
#define __KMLDONKEY_STATSPAGERATES_H__

#include <QFrame>
#include <QPaintEvent>
#include <QList>
#include <QPixmap>

#include "autobox.h"

#include "statspage.h"

class StatsWidgetObject
{
    public:
        StatsWidgetObject() {}

        QList<long> values;
        bool enable;
        QColor color;
};


class StatsWidget : public QFrame
{
    public:
        StatsWidget( QWidget *parent, long defaultmaxvalue );
        ~StatsWidget();

        /* Clear the StatsWidget. Remove all statistic and there views. */
        void clear();
        void render();

        /* On the StatWidget different statistics could be shown. Each of
        * them did have an unique id. With addStat a new statistic could
        * be added.
        * \param id The source-id the stat should have/use. You have to
        *           add new values to the source-id with addStatValue.
        * \param color The used color to display the stat.
        */
        void addStat( const QString &statid );
        /* Remove an added statistic and all it's views. */
        void removeStat( const QString &statid );
        /* Sets if the statistic is enabled or disabled. */
        void setStatEnable( const QString &statid, const bool on );
        /* Sets the used color for the statistic */
        void setStatColor( const QString &statid, const QColor& color );

        /* Add a new value to the with statid defined statistic. */
        void addStatValue( const QString &statid, long value );

    protected:
        virtual void paintEvent( QPaintEvent* );

    private:
        QPixmap *m_pixmap;
        QMap<QString, StatsWidgetObject*> stats;
        long maxvalue, minvalue;
};

class StatsPageRates : public VBoxWidget
{
    public:
        StatsPageRates( QWidget* parent );
        StatsWidget *statswidget;
};

#endif /* __KMLDONKEY_STATSPAGERATES_H__ */
