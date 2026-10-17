/* -*- c++ -*-
 *
 * statspage.h
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

#ifndef __kmldonkey_statspage_h__
#define __kmldonkey_statspage_h__

#include <QTabWidget>

#include "kmldonkey.h"
#include "page.h"

class StatsPageRates;
class StatsPageClients;
class StatsPageOvernet;
class StatsPageGnutella;

class StatsPage : public QTabWidget, public KMLDonkeyPage
{
    Q_OBJECT

public:
    explicit StatsPage(QWidget *parent);

    virtual void setupActions(KActionCollection*);

    virtual void configurePrefsDialog(KMLDonkeyPreferences*);
    virtual void applyPreferences(KMLDonkeyPreferences* = 0);
    virtual void saveState(KSharedConfigPtr);
    virtual void restoreState(KSharedConfigPtr);
    virtual void clear();

public slots:
    void actionActivatePage();

protected slots:
    void updateStatus(int64, int64, int64, int, int, int, int, int, int, int, QMap<int,int>*);
    void updatePages();

protected:
    void showEvent(QShowEvent *event);

private:
    bool enabled;
    int tabpos;

    StatsPageRates* ratespage;
    StatsPageClients* clientspage;
    StatsPageOvernet *overnetpage;
    StatsPageGnutella *gnutellapage;
};

#endif
