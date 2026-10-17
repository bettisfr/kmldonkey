/* -*- c++ -*-
 *
 * searchtabiface.h
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
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

#ifndef __kmldonkey_searchtabiface_h__
#define __kmldonkey_searchtabiface_h__

#include <qwidget.h>
#include "autobox.h"

class SearchTabInterface : public VBoxWidget
{
    Q_OBJECT
public:
    SearchTabInterface(QWidget *parent, const char *name, const QString &tablabel, int sno)
        : VBoxWidget(parent)
        , id(sno)
        , tabLabel(tablabel)
    {
        setObjectName(name);
    }

protected:
    int id;
    QString tabLabel;
};

#endif // __kmldonkey_searchtabiface_h__
