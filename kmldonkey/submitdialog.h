/* -*- c++ -*-
 *
 * submitdialog.h
 *
 * Copyright (C) 2003-2004,2007 Sebastian Sauer <mail@dipe.org>
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

#ifndef __kmldonkey_submitdialog_h__
#define __kmldonkey_submitdialog_h__

#include "porteddialog.h"

class QTextEdit;

class SubmitDialog : public PortedDialog
{
    Q_OBJECT
public:
    explicit SubmitDialog(QWidget* parent);
    virtual ~SubmitDialog();

    QString text() const;
    void setText(const QString&);

private:
    QTextEdit* m_edit;

protected slots:
    void editChanged();
    void addUrl();
    void closeDialog();
};

#endif // __kmldonkey_submitdialog_h__
