/* -*- c++ -*-
 *
 * shareddirs.h
 *
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
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

#ifndef __kmldonkey_shareddirs_h__
#define __kmldonkey_shareddirs_h__

#include "porteddialog.h"
#include <donkeyprotocol.h>

class SharedDirsDialog;

class applySharesConsoleCallback : public ConsoleCallbackInterface
{
public:
    void callback(const QString& cmd, const QString& res);
    int count;
    QStringList list;
};

class sharesConsoleCallback : public ConsoleCallbackInterface
{
public:
    void callback(const QString& cmd, const QString& res);
    SharedDirsDialog *dlg;
};

class SharedDirsDialog : public PortedDialog
{
    Q_OBJECT
public:
    SharedDirsDialog(QWidget* parent);
    ~SharedDirsDialog();

    void setEnabled();
    QTreeWidgetItem* addItem(QString dir, int prio);

    QTreeWidget *view;
    QMap<QString,int> shareddirs;

protected slots:
    void slotSelectionChanged();
    void slotAdd();
    void slotRemove();

    void slotEditChanged();
    void slotPrioChanged();
    void slotDirPath();

    void applyChanges();
    void closeDialog();

private:
    bool enabled;
    QPushButton *btnAdd;
    QPushButton *btnRemove;

    QFrame *editBox;
    KLineEdit *editDir;
    QSpinBox *editPrio;

    applySharesConsoleCallback *applyCallback;
};

#endif // __kmldonkey_shareddirs_h__
