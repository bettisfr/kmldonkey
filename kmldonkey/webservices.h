/* -*- c++ -*-
 *
 * webservices.h
 *
 * Copyright (C) 2003, 2005 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2007 Sebastian Sauer <mail@dipe.org>
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

#ifndef __kmldonkey_webservices_h__
#define __kmldonkey_webservices_h__

#include "webbrowser.h"

#include <QShowEvent>

class FileInfoWebBrowser : public WebBrowser
{
    Q_OBJECT

public:

    explicit FileInfoWebBrowser(int fileno, QWidget* parent = 0);
    virtual void handleED2K(const ED2KURL& url);

public slots:

    virtual void loadWebPage() = 0;

protected:

    void showEvent(QShowEvent* event);

    int fn;
    bool loaded;

};

class EMugleFileInfo : public FileInfoWebBrowser
{
    Q_OBJECT

public:

    EMugleFileInfo(int fileno, QWidget* parent = 0);

public slots:

    void loadWebPage();

};

#endif // __kmldonkey_webservices_h__
