/* -*- c++ -*-
 *
 * optioneditor.h
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

#ifndef __kmldonkey_optioneditor_h__
#define __kmldonkey_optioneditor_h__

#include <QList>
#include <QTableWidget>

#include <options.h>

class MyDonkeyOption
{
public:
    MyDonkeyOption();
    MyDonkeyOption(const DonkeyOption& opt, const QString& value);
    MyDonkeyOption(const MyDonkeyOption& opt);

    void setValue(const QString& value);
    void setDirty(bool dirty);
    bool isDirty() const;
    void applyOption(DonkeyProtocol* target);
    const DonkeyOption& option() const;

private:
    DonkeyOption m_option;
    QString m_value;
    bool m_dirty;
};

class OptionEditorWidget : public QTableWidget
{
    Q_OBJECT

public:
    enum { OptionIndexRole = Qt::UserRole + 1, OptionTypeRole };
    OptionEditorWidget(QWidget* parent = 0);
    
    void addOption(const DonkeyOption& option);
    void applyChangedOptions(DonkeyProtocol*);

protected slots:
    
    void optionChanged(QTableWidgetItem *item);

signals:

    void listIsDirty();

private:

    typedef QList<MyDonkeyOption> OptionList;
    OptionList m_options;

};

#endif // __kmldonkey_optioneditor_h__
