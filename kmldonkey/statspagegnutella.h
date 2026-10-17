/* -*- c++ -*-
 *
 * statspagegnutella.h
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

#ifndef __kmldonkey_statspagegnutella_h__
#define __kmldonkey_statspagegnutella_h__

#include <QScrollArea>
#include <QShowEvent>

class VBoxWidget;
class QTextBrowser;

#include "statspage.h"
#include "donkeyprotocol.h"

class StatsPageGnutellaCallback : public ConsoleCallbackInterface
{
public:
    StatsPageGnutellaCallback(QTextBrowser *b) : browser(b) {}
    void callback(const QString& cmd, const QString& res);
private:
    QTextBrowser *browser;
};

class StatsPageGnutella : public QScrollArea
{
    Q_OBJECT
public:
    StatsPageGnutella(QWidget *parent);
public slots:
    void slotUpdate();
protected:
    void showEvent(QShowEvent* event);
private:
    VBoxWidget *box;
    QTextBrowser *browser;
};

#endif
