#include <QLocale>
/* -*- c++ -*-
 *
 * statspagerates.cpp
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

#include <QPainter>
#include <QPixmap>
#include <QList>
#include <QPaintEvent>

#include <QDebug>
#include <KLocalizedString>

#include "kmldonkey.h"
#include "donkeyprotocol.h"
#include "prefs/prefs.h"
#include "statspagerates.h"

StatsWidget::StatsWidget( QWidget *parent, long defaultmaxvalue )
    : QFrame( parent )
    , m_pixmap( 0 )
{
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    
	QPalette p;
	p.setColor(backgroundRole(), Qt::white );
	setPalette( p );

    maxvalue = defaultmaxvalue;
    minvalue = defaultmaxvalue;
    m_pixmap = 0;
}


StatsWidget::~StatsWidget()
{
}


void StatsWidget::clear()
{
    qDeleteAll(stats);
    stats.clear();
    render();
}


void StatsWidget::addStat( const QString &statid )
{
    Q_ASSERT( ! stats.contains(statid) );
    stats[statid] = new StatsWidgetObject();
}


void StatsWidget::removeStat( const QString& statid )
{
    Q_ASSERT( stats.contains(statid) );
    StatsWidgetObject* stat = stats[statid];
    stats.remove( statid );
    delete stat;
}


void StatsWidget::setStatEnable( const QString &statid, const bool on )
{
    Q_ASSERT( stats.contains(statid) );
    stats[statid]->enable = on;
}


void StatsWidget::setStatColor( const QString &statid, const QColor& color )
{
    Q_ASSERT( stats.contains(statid) );
    stats[statid]->color = color;
}


void StatsWidget::addStatValue( const QString &statid, long value )
{
    Q_ASSERT( stats.contains(statid) );

    if ( value < 0 )
        value = 0; // negative values aren't evaluated
    else if ( value > maxvalue )
        maxvalue = value;

    if ( value < minvalue )
        minvalue = value;

    // cleanup the numbers a bit
    long e = maxvalue - minvalue;
    if(e > 100000) e = 100000;
    else if(e > 10000) e = 10000;
    else if(e > 1000) e = 1000;
    else if(e > 100) e = 100;
    else e = 10;

    long tmp;
    if ((tmp = minvalue % e))
        minvalue -= tmp;

    if ((tmp = maxvalue % e))
        maxvalue += e-tmp;

    if ( maxvalue - minvalue <= 0 )
        minvalue = 0; // we don't want divide something with negativ numbers or 0

    int maxv = 1024; // default is to remember enough items to fill a 2048-Screen
    if ( width() > (maxv * 2) )
        maxv = width() + 1 / 2; // if we've more space to fill

    while ( stats[statid]->values.count() > maxv ) // remove older values
        stats[statid]->values.pop_back();

    stats[statid]->values.insert( stats[statid]->values.begin(), value );
    render();
}


void
StatsWidget::render()
{
    if ( !m_pixmap )
        m_pixmap = new QPixmap( width(), height() );
    else
        if ( m_pixmap->width() != width() || m_pixmap->height() != height() )
            *m_pixmap = m_pixmap->scaled( width(), height() );

    QPainter painter( m_pixmap );
    painter.fillRect( 0, 0, width(), height(), Qt::white ); // clear the painting-area

    int w, pos, prevpos;
    int h = height() - 4;
    int steps = 2;

    int vrange = maxvalue - minvalue;
    if ( vrange >= 3 )
    {
        // paint grid
        int vsteps = vrange / 10;
        if ( vsteps < 1 )
            vsteps = 1;

        int psteps = (int)( (float)h / vrange * vsteps );
        int r = maxvalue;

        painter.setPen( Qt::lightGray );
        for ( int i = psteps; i < height(); i += ( psteps > 0 ) ? psteps : 100 )
        {
            r -= vsteps;
            painter.drawLine( 0, i, width(), i );
            painter.drawText( 3, i - 1, QLocale().toString(double(r), 'f', 0) );
        }
    }

    StatsWidgetObject *so;
    QMap<QString, StatsWidgetObject*>::iterator statsit;
    QList<long>::iterator it;

    for ( statsit = stats.begin(); statsit != stats.end(); ++statsit )
    {
        so = statsit.value();
        if ( !so )
            continue;

        w = width();
        prevpos = -1;
        painter.setPen( so->color );

        for ( it = so->values.begin(); it != so->values.end() && w >= 0; ++it )
        {
            pos = h - (int)( (float)h / ( maxvalue - minvalue ) * ( (*it) - minvalue ) );

            if ( prevpos >= 0 )
                painter.drawLine( w, pos, w + steps, prevpos );

            prevpos = pos;
            w = ( w > steps ) ? w - steps : ( w > 0 ) ? 0 : -1;
        }
    }

    painter.end();
    update();
}


void
StatsWidget::paintEvent( QPaintEvent* )
{
    if ( m_pixmap )
	{
		QPainter p(this);
		p.drawPixmap( QPoint(0, 0), *m_pixmap );
	}
}


StatsPageRates::StatsPageRates( QWidget* parent )
    : VBoxWidget( parent )
{
    bool ok;
    int i;
    int maxvalue = 15;
    QMap<QString, QString> options = KMLDonkey::App->donkey->optionsList();

    i = options["max_hard_download_rate"].toInt( &ok );
    if ( ok && i > maxvalue )
        maxvalue = i;

    i = options["max_hard_upload_rate"].toInt( &ok );
    if ( ok && i > maxvalue )
        maxvalue = i;

    statswidget = new StatsWidget( this, maxvalue * 1000 );

    statswidget->addStat( "tdl" );
    statswidget->addStat( "tul" );
}

