/* -*- c++ -*-
 *
 * consolepage.h
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

#ifndef __kmldonkey_consolepage_h__
#define __kmldonkey_consolepage_h__

#include <QObject>
#include "autobox.h"

#include "page.h"

class KMLDonkeyPreferences;
class QTextBrowser;
class KLineEdit;

class ConsolePage : public VBoxWidget, public KMLDonkeyPage
{
    Q_OBJECT
public:
    explicit ConsolePage(QWidget* parent = 0);

    virtual void setupActions(KActionCollection*);

    virtual void applyPreferences(KMLDonkeyPreferences* = 0);
    virtual void clear();

public slots:
    void consoleMessage(const QString& msg);
    void sendConsoleMessage(const QString&);

    void actionActivatePage();

private:
    QTextBrowser* consoleView;
    KLineEdit* consoleEntry;
    bool alreadyClear;
    QString txtBefore;
};

#endif
