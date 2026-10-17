/* -*- c++ -*-
 *
 * renamedialog.h
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

#ifndef __kmldonkey_renamedialog_h__
#define __kmldonkey_renamedialog_h__

#include "porteddialog.h"

#include <donkeyprotocol.h>

class KLineEdit;
class KComboBox;

class RenameDialog : public PortedDialog
{
    Q_OBJECT
public:
    RenameDialog(DonkeyProtocol* protocol, int file);

protected slots:
    virtual void slotDefault();
    virtual void slotTry();
    virtual void slotOk();

signals:
    void renameFile(int fileno, const QString& name);

private:
    DonkeyProtocol* donkey;
    int fileno;

    KLineEdit *nameView, *sizeView, *hashView, *formatView;
    KComboBox* nameEdit;

};

#endif // __kmldonkey_renamedialog_h__
