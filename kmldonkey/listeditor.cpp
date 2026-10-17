/*
 * listeditor.cpp
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

#include "listeditor.h"

#include <QLabel>
#include <QToolTip>
#include <QList>

#include <KSharedConfig>
#include <kconfig.h>
#include <KLocalizedString>
#include <QDebug>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <klineedit.h>

/*** ListEditorEdit ***/

ListEditorEdit::ListEditorEdit(QWidget *parent, int columnpos) : HBoxWidget(parent)
{
    this->columnpos = columnpos;
    edit = new KLineEdit(this);
    connect(edit, SIGNAL(textChanged(const QString &)), this, SLOT(slotEditChanged()));
}

inline int ListEditorEdit::getColumnPos() { return columnpos; }
inline void ListEditorEdit::setText(const QString& text) { edit->setText(text); }

void ListEditorEdit::slotEditChanged()
{
    emit editChanged(columnpos, edit->text());
}

/*** ListEditorItem ***/

ListEditorItem::ListEditorItem(QTreeWidget* parent, int pos) : QTreeWidgetItem(parent)
{
    setPosition(pos);
}

inline int ListEditorItem::getPosition() { return position; }
inline void ListEditorItem::setPosition(int pos) { position = pos; }

bool ListEditorItem::operator < ( const QTreeWidgetItem &other ) const
{
    int pos = ((ListEditorItem*) &other)->getPosition();
	return position < pos;
}

void ListEditorItem::moveItem(QTreeWidgetItem *after)
{
	QTreeWidget * list = treeWidget();
	list->takeTopLevelItem( list->indexOfTopLevelItem( this ) );
	list->insertTopLevelItem( list->indexOfTopLevelItem( after ) + 1 , this );
	
    int pos = position;
    setPosition( ((ListEditorItem*)after)->getPosition() );
    ((ListEditorItem*)after)->setPosition(pos);
}

/*** ListEditor ***/

ListEditor::ListEditor(QWidget *parent)
    : VBoxWidget(parent)
{
    Modified = false;
    setSpacing(6);

    HBoxWidget *box = new HBoxWidget(this);
    box->setSpacing(6);
    setStretchFactor(box, 1);

    list = new QTreeWidget(box);
    list->setSelectionMode( QTreeWidget::SingleSelection );
    list->setAllColumnsShowFocus(true);
    connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(slotListChanged()));

    VBoxWidget *btnBox = new VBoxWidget(box);

    btnAdd = new QPushButton(i18n("&Add"), btnBox);
    btnAdd->setToolTip(i18n("Add a new item to the list"));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(slotAdd()));

    btnRem = new QPushButton(i18n("&Remove"), btnBox);
    btnRem->setToolTip(i18n("Remove selected item from list"));
    connect(btnRem, SIGNAL(clicked()), this, SLOT(slotRemove()));

    btnUp = new QPushButton(i18n("&Up"), btnBox);
    btnUp->setToolTip(i18n("Move selected item up"));
    connect(btnUp, SIGNAL(clicked()), this, SLOT(slotUp()));

    btnDown = new QPushButton(i18n("&Down"), btnBox);
    btnDown->setToolTip(i18n("Move selected item down"));
    connect(btnDown, SIGNAL(clicked()), this, SLOT(slotDown()));

    btnBox->setStretchFactor(new QWidget(btnBox), 1);

    editBox = new VBoxWidget(this);
    editBox->setSpacing(6);

    slotListChanged();
}

QList<QStringList> ListEditor::getRows()
{
    QList<QStringList> items;
    int i;
    QStringList sl;
    QTreeWidgetItemIterator it( list );
    while ( (*it) ) {
        sl.clear();
        for (i = 0; i <= list->columnCount(); i++)
            sl << (*it)->text(i);
        items.append(sl);
        ++it;
    }
    return items;
}

ListEditorEdit* ListEditor::addColumn(const QString &column, const QString& tooltip, bool editable)
{
	list->setColumnCount( list->columnCount() + 1 );
	QTreeWidgetItem * hi = list->headerItem();
	if( hi )
		hi->setText( list->columnCount() - 1, column );
	QHeaderView * h = list->header();
	if( h )
		h->setSectionResizeMode( QHeaderView::Fixed );
	
    ListEditorEdit *edit = 0;
    if (editable) {
        HBoxWidget *box = new HBoxWidget(editBox);
        if (! tooltip.isEmpty()) box->setToolTip(tooltip);
        new QLabel(column, box);
        edit = new ListEditorEdit(box, list->columnCount() - 1);
        edits.append(edit);
        connect(edit, SIGNAL( editChanged(int, const QString&) ), this, SLOT( slotEditChanged(int, const QString&) ));
    }
    return edit;
}

ListEditorItem* ListEditor::addRow(const QStringList& items)
{
    if (items.count() < 1) 
		return 0;
    ListEditorItem *item = new ListEditorItem(list, list->topLevelItemCount() );
    for (int i = 0; i <= items.count() && i <= list->columnCount(); i++)
        item->setText(i, items[i]);
    return item;
}

void ListEditor::addRows(QList<QStringList> items, bool clear)
{
    if (clear) {
        removeRows();
        Modified = false;
    }
    for (QList<QStringList>::Iterator lit = items.begin(); lit != items.end(); ++lit)
        addRow(*lit);
}

void ListEditor::removeRows()
{
    list->clear();
    slotListChanged();
}

void ListEditor::slotListChanged()
{
    editBox->setEnabled( list->currentItem() );

    for (QList<ListEditorEdit*>::Iterator it = edits.begin(); it != edits.end(); ++it) {
        if (! (*it)) continue;
        (*it)->setText( list->currentItem()
                        ? list->currentItem()->text( (*it)->getColumnPos() )
                        : QString("") );
    }

    btnRem->setEnabled( list->currentItem() );
    btnUp->setEnabled( list->currentItem() && list->itemAbove( list->currentItem() ) );
    btnDown->setEnabled( list->currentItem() && list->itemBelow( list->currentItem()) );
}

void ListEditor::slotEditChanged(int columnpos, const QString& text)
{
    if (list->currentItem() && list->currentItem()->text(columnpos) != text) {
        list->currentItem()->setText(columnpos, text);
        Modified = true;
    }
}

void ListEditor::slotAdd()
{
    ListEditorItem *item = new ListEditorItem(list, list->topLevelItemCount() );
    list->sortByColumn(0, Qt::AscendingOrder);
    list->setCurrentItem( item );
    Modified = true;
}

void ListEditor::slotRemove()
{
    ListEditorItem *item = (ListEditorItem*)list->currentItem();
    if (! item) return;

    if ( list->itemAbove( item ))
        list->setCurrentItem( list->itemAbove( item ), true);
    else if (list->itemBelow( item ))
        list->setCurrentItem(list->itemBelow( item ), true);

    ListEditorItem *i = item;
    while(list->itemBelow( i )) {
        i = (ListEditorItem*) list->itemBelow( i );
        i->setPosition(i->getPosition() - 1);
    }

    delete item;
    slotListChanged();
    Modified = true;
}

void ListEditor::slotUp()
{
    if ( (! list->currentItem()) || (! list->itemAbove( list->currentItem() )) ) 
		return;
    ((ListEditorItem*) list->itemAbove( list->currentItem() ))->moveItem( list->currentItem() );
    slotListChanged();
    Modified = true;
}

void ListEditor::slotDown()
{
    if ( (! list->currentItem()) || (! list->itemBelow( list->currentItem() )) ) 
		return;
    ((ListEditorItem*)list->currentItem())->moveItem( list->itemBelow( list->currentItem() ) );
    slotListChanged();
    Modified = true;
}

/*** AutoRenamerListEditor ***/

AutoRenamerListEditor::AutoRenamerListEditor(QWidget *parent) : ListEditor(parent)
{
    addColumn(i18n("Search for"), i18n("Search for regular expression..."));
    addColumn(i18n("Replace with"), i18n("Replace searched regular expression with..."));
}

