/* -*- c++ -*-
 *
 * listeditor.h
 *
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

#ifndef __kmldonkey_listeditor_h__
#define __kmldonkey_listeditor_h__

#include <QWidget>
#include <QList>

#include "autobox.h"
#include "autobox.h"
#include <QTreeWidget>

class QPushButton;
class KLineEdit;

/* The ListEditorEdit-class presents an editfield for a
 * ListEditorItem inside a ListEditor-class.
 */
class ListEditorEdit : public HBoxWidget
{
    Q_OBJECT
public:
    /* constructor */
    ListEditorEdit(QWidget *parent, int columnpos);
    /* returns the remembered column-position. Each editable
     * column in a ListEditor->list did have a ListEditorEdit. */
    inline int getColumnPos();
    /* Sets the text displayed at the ListEditorEdit. */
    inline void setText(const QString& text);

private:
    int columnpos;
    KLineEdit *edit;

signals:
    void editChanged(int columnpos, const QString& text);

protected slots:
    void slotEditChanged();
};

/* A ListEditorItem is a QTreeWidgetItem for the ListEditor->list.
 */
class ListEditorItem : public QTreeWidgetItem
{
public:
    /* constructor */
    ListEditorItem(QTreeWidget* parent, int pos);
    /* Returns the row-position of the ListViewItem */
    inline int getPosition();
    /* Set the row-position of the ListViewItem */
    inline void setPosition(int pos);

    /* Internal used to be sure the positions are keeped also after
     * a sort is done. */
    virtual bool operator < ( const QTreeWidgetItem &other ) const;
    /* Move this ListViewItem to a new position */
    void moveItem(QTreeWidgetItem *after);

protected:
    int position;
    KLineEdit *edit;
};

/* The ListEditor-class contains the main functionality to present
 * and edit a list. What I tried was to keep this class as
 * flexible as possible to be easily able to inherit new
 * classes for new purposes w/o having to rewrote the same code
 * again.
 */
class ListEditor : public VBoxWidget
{
    Q_OBJECT
public:
    /* constructor */
    ListEditor(QWidget *parent);

    /* Get the items back.
     * /return The (possible modified) Items
     */
    QList<QStringList> getRows();

    /* Add a new column to the ListEditor->list.
     * /param column The used columntitle
     * /param editable Is this column editable (did i have a ListEditorEdit)
     * /return the new ListEditorEdit added (if editable was false 0 will be returned)
     */
    ListEditorEdit* addColumn(const QString &column, const QString& tooltip = QString(), bool editable = true);
    /* Add a new row to the ListEditor->list.
     * /param items Each String in this StringList presents one col in the row
     * /return the new added ListEditorItem or 0 if the item couldn't be added
     */
    ListEditorItem* addRow(const QStringList& items);
    /* Add a bunch of items. Each QValueList-item presents a row
     * in the ListEditor->list.
     * /param items Each QValueList-item presents one row and the
     *              QStringList-items are the cols in this row.
     * /param clear Remove previous remembered items and set Modified to false
     */
    void addRows(QList<QStringList> items, bool clear = true);
    /* Remove all remembered items. The added columns are still avaible! */
    void removeRows();

    /* shows if the user did modify anything on the list. Usage of
     * the functions above arn't counted as modifiers! */
    bool Modified;

private:
    QTreeWidget *list;
    VBoxWidget *editBox;
    QList<ListEditorEdit*> edits;
    QPushButton *btnAdd;
    QPushButton *btnRem;
    QPushButton *btnUp;
    QPushButton *btnDown;

private slots:
    void slotListChanged();
    void slotEditChanged(int, const QString&);
    void slotAdd();
    void slotRemove();
    void slotUp();
    void slotDown();
};

class AutoRenamerListEditor : public ListEditor
{
public:
    /* constructor */
    AutoRenamerListEditor(QWidget *parent);
};

#endif // __kmldonkey_listeditor_h__
