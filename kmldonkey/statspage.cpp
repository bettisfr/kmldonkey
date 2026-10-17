#include <KConfigGroup>
/* -*- c++ -*-
 *
 * statspage.cpp
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

#include <QLabel>
#include <QCheckBox>
#include <QTimer>

#include <QDebug>
#include <KLocalizedString>
#include <kconfig.h>
#include <QAction>

#include "kmldonkey.h"
#include "prefs/prefs.h"
#include "statspagerates.h"
#include "statspageclients.h"
#include "statspageovernet.h"
#include "statspagegnutella.h"

#include "statspage.h"

StatsPage::StatsPage(QWidget *parent)
    : QTabWidget(parent)
    , KMLDonkeyPage()
{
	setObjectName( "statsPage" );
    
	tabpos = 2;
    setContentsMargins(8, 8, 8, 8);
    setDocumentMode(true);

    // Rates
    ratespage = new StatsPageRates(this);

    ratespage->statswidget->setStatEnable("tdl", true);
    ratespage->statswidget->setStatColor("tdl", Qt::blue);

    ratespage->statswidget->setStatEnable("tul", true);
    ratespage->statswidget->setStatColor("tul", Qt::red);

    addTab(ratespage, i18n("Rates"));

    // other optional pages, handled at showEvent()
    clientspage = 0;
    overnetpage = 0;
    gnutellapage = 0;

    connect(KMLDonkey::App->donkey, SIGNAL(optionsUpdated()), this, SLOT(updatePages()));

    /*TODO
    cs
    upstats : statistics on upload
    version : print mldonkey version
    mem_stats : print memory stats
    vu : view upload credits
    id : print ID on connected servers
    dump_reliability : display the reliability of sources
    */
}

void StatsPage::setupActions(KActionCollection*)
{
}

void StatsPage::configurePrefsDialog(KMLDonkeyPreferences *prefs)
{
    prefs->generalPage->enableStatsCheckbox->setChecked(enabled);
}

void StatsPage::applyPreferences(KMLDonkeyPreferences *prefs)
{
    if (prefs) {
        if (enabled == prefs->generalPage->enableStatsCheckbox->isChecked())
            return;
        enabled = prefs->generalPage->enableStatsCheckbox->isChecked();
    }

    /*TODO: handle insertTab/removePage in KMLDonkey
    if(parent() && parent()->parent()) {
        QTabWidget *w = dynamic_cast<QTabWidget*>(parent()->parent());
        if(w) {
            if(enabled) {
                w->insertTab(this, i18n("Statistics"), tabpos);
            }
            else {
                tabpos = w->indexOf(this);
                w->removePage(this);
            }
        }
    }
    */

    if (enabled) {
        connect(KMLDonkey::App->donkey, SIGNAL(clientStats(int64, int64, int64, int, int, int, int, int, int, int, QMap<int,int>*)),
                this, SLOT(updateStatus(int64, int64, int64, int, int, int, int, int, int, int, QMap<int,int>*)));
    }
    else {
        disconnect(KMLDonkey::App->donkey, SIGNAL(clientStats(int64, int64, int64, int, int, int, int, int, int, int, QMap<int,int>*)),
                   this, SLOT(updateStatus(int64, int64, int64, int, int, int, int, int, int, int, QMap<int,int>*)));
    }
}

void StatsPage::saveState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "StatsPage" );
    group.writeEntry("StatsEnabled", enabled);
}

void StatsPage::restoreState(KSharedConfigPtr conf)
{
    KConfigGroup group = conf->group( "StatsPage" );
    enabled = group.readEntry("StatsEnabled", true);
    applyPreferences();
}

void StatsPage::clear()
{
    //rateStats->clear();
    updatePages();
}


void StatsPage::actionActivatePage()
{
    KMLDonkey::App->activatePage(this);
}

void StatsPage::showEvent(QShowEvent *event)
{
    QTabWidget::showEvent(event);
    updatePages();
}

void StatsPage::updateStatus(int64, int64, int64, int, int tul, int tdl, int, int, int, int, QMap<int,int>*)
{
    if (ratespage) {
        ratespage->statswidget->addStatValue("tdl", (long)tdl);
        ratespage->statswidget->addStatValue("tul", (long)tul);
    }
    //delete map;
}

void StatsPage::updatePages()
{
    bool enableOvernet = false;
    bool enableGnutella = false;

    if (KMLDonkey::App->donkey->isConnected()) {
        enableOvernet  = ( KMLDonkey::App->donkey->getOption("enable_overnet") == "true" );
        enableGnutella = ( KMLDonkey::App->donkey->getOption("enable_gnutella") == "true" ||
                           KMLDonkey::App->donkey->getOption("enable_gnutella2") == "true" );
    }

    // Clients
    if (KMLDonkey::App->donkey->isConnected()) {
        if (! clientspage && isVisible()) {
            clientspage = new StatsPageClients(this);
            addTab(clientspage, i18n("Clients"));
        }
    }
    else {
        delete clientspage;
        clientspage = 0;
    }

    // Overnet
    if (enableOvernet) {
        if (! overnetpage && isVisible()) {
            overnetpage = new StatsPageOvernet(this);
            addTab(overnetpage, "Overnet");
        }
    }
    else {
        delete overnetpage;
        overnetpage = 0;
    }

    // Gnutella
    if (enableGnutella) {
        if (! gnutellapage && isVisible()) {
            gnutellapage = new StatsPageGnutella(this);
            addTab(gnutellapage, "Gnutella");
        }
    }
    else {
        delete gnutellapage;
        gnutellapage = 0;
    }
}
