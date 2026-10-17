/* -*- c++ -*-
 *
 * plugin.h
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
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

#ifndef __kmldonkey_plugin_h__
#define __kmldonkey_plugin_h__

#include <QStringList>
#include <QObject>

#include "page.h"

class KMLDonkey;


class KMLDonkeyPlugin : public QObject
{
    Q_OBJECT
public:
    KMLDonkeyPlugin(KMLDonkey* app, QObject* parent = 0, const char* name = 0);

    virtual QString pluginName() = 0;
    virtual QString pluginTitle() = 0;
    virtual QString pluginIcon() = 0;
    virtual QString pluginDescription() = 0;

    KMLDonkeyPage* addPage(QWidget* parent);

protected:
    KMLDonkey* m_app;

};

#endif
