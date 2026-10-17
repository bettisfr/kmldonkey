/* -*- c++ -*-
 *
 * addserver.h
 *
 * Copyright (C) 2003 Sebastian Sauer <dipesh@gmx.de>
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

#ifndef __kmldonkey_addserver_h__
#define __kmldonkey_addserver_h__

#include "porteddialog.h"

class KComboBox;
class KLineEdit;
class QSpinBox;

class AddServerDialog : public PortedDialog
{
    Q_OBJECT
public:
    AddServerDialog(QWidget* parent);

private:
    QSpinBox* m_portEntry;
    KLineEdit* m_addressEntry;
    KComboBox* m_networkEntry;

protected slots:
    void addressEditChanged();
    void addServer();
    void closeDialog();
};

#endif // __kmldonkey_addserver_h__
