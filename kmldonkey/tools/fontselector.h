/* -*- c++ -*-
 *
 * fontselector.h
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003-2004 Sebastian Sauer <mail@dipe.org>
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

#ifndef __kmldonkey_fontselector_h__
#define __kmldonkey_fontselector_h__

#include <qfont.h>
#include <qframe.h>
#include <QLabel>

class QPushButton;
class KSqueezedTextLabel;

class FontSelector : public QFrame
{
    Q_OBJECT
public:
    FontSelector(QWidget* parent = NULL, const char* name = NULL, bool fixed = false);

    QFont& font();
    void setFont(QFont& font);

signals:
    void fontSelected(QFont&);

protected slots:
    void selectFont();

private:
    QFont ft;
    KSqueezedTextLabel* preview;
    QPushButton* chooseButton;
    bool wantFixed;
};

#endif // __kmldonkey_fontselector_h__
