/* -*- c++ -*-
 *
 * statspagegnutella.cpp
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
#include <QShowEvent>

#include "autobox.h"
#include <QDebug>
#include <KLocalizedString>
#include <QTextBrowser>

#include "kmldonkey.h"
#include "donkeyprotocol.h"
#include "prefs/prefs.h"

#include "statspagegnutella.h"

void StatsPageGnutellaCallback::callback(const QString& /*cmd*/, const QString& res)
{
    browser->clear();
    QStringList list = res.split("\n");
    foreach (const QString &str, res.split('\n')) {
        browser->append(str);
    }
}

StatsPageGnutella::StatsPageGnutella(QWidget *parent) 
	: QScrollArea(parent)
{
    setWidgetResizable( true );

    box = new VBoxWidget();
    box->setSpacing(6);
    box->setContentsMargins(11, 11, 11, 11);
    setWidget(box);

    browser = new QTextBrowser(box);
}

void StatsPageGnutella::slotUpdate()
{
    if (! KMLDonkey::App->donkey->isConnected()) 
		return;
    KMLDonkey::App->donkey->sendConsoleMessage("gstats", new StatsPageGnutellaCallback(browser));
}

void StatsPageGnutella::showEvent(QShowEvent* /*event*/)
{
    slotUpdate();
}
