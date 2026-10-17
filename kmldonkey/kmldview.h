/* -*- c++ -*-
 *
 * kmldview.h
 *
 * Copyright (C) 2009       Aleksey Markelov <markelovai@gmail.com>
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

#ifndef KMLDONKEY_KMLDVIEW_H
#define KMLDONKEY_KMLDVIEW_H

#include <QAbstractItemView>
#include <QSortFilterProxyModel>
#include <QList>
#include <QPointer>

class QTreeView;

class KmldView : public QObject
{
    Q_OBJECT
public:
    KmldView(QObject *parent = 0);
    QAbstractItemView *createViewSorted(QAbstractItemModel *model,
            const QString &name, QWidget *parent = 0);
    QSortFilterProxyModel *createProxy(QAbstractItemModel *model, QObject *parent = 0);
    void adjustProxy(QSortFilterProxyModel *model);
    QAbstractItemView *createView(QAbstractItemModel *model, const QString &name, QWidget *parent = 0);
    void saveStates();
    void saveState(QAbstractItemView *view);
private slots:
    void optionsChanged();
    void headerMenu(const QPoint &pos);
private:
    QList<QPointer<QAbstractItemView> > viewList;
};

#endif //KMLDONKEY_KMLDVIEW_H
