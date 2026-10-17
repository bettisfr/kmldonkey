/* -*- c++ -*-
 *
 * abstractinfomodel.h
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

#ifndef KMLDONKEY_ABSTRACTINFOMODEL_H
#define KMLDONKEY_ABSTRACTINFOMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QVector>
#include <QPair>

class QItemSelectionModel;

namespace Q4
{

class InfoItem
{
public:
    enum {
        IdRole = Qt::UserRole,
        SortRole = Qt::UserRole + 1
    };

    InfoItem(int newid) : id(newid) {}
    virtual ~InfoItem() {}
    QVariant data(int column, int role) const;
    void setData(int column, const QVariant &data, int role = Qt::DisplayRole);
    virtual void update() = 0;
    //use with care
    int id;
    int row;
    int updatedColumns;
protected:
    typedef QPair<int, QVariant> RoleData;
    typedef QVector<RoleData> CellData;
    typedef QVector<CellData> RowData;
    RowData m_data;
};

class AbstractInfoModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    static const int SortRole = Qt::UserRole + 20;
    AbstractInfoModel(int columnCount, QObject *parent = 0);
    ~AbstractInfoModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void init() { dropCache(); }
    void dropCache();

    void clipboardAction(const QString &action, QItemSelectionModel *smodel);
    InfoItem *itemAt(const QModelIndex &index) const { return itemAt(index.row()); }
    InfoItem *itemAt(int row) const;
    InfoItem *findItemById(int id) const { return itemsHash.value(id); }
protected slots:
    void removeItem(int i);
    void updateItem(int i);
    virtual void clear();//used to clear model on disconnect
protected:
    virtual void dropCacheImp() {}
    virtual QStringList headerLabels() const = 0;
    virtual InfoItem *prepareNewItem(int num) = 0;
private:
    QVector<QVariant> headers;
    typedef QHash<int, InfoItem *> InfoItemHash;
    typedef QList<InfoItem *> InfoItemList;
    InfoItemHash itemsHash;
    InfoItemList itemsList;
};

/** 
 * This class should solve sorting stability problem with QSortFilterProxyModel
 * and dynamicSortFilter enabled by comparing equal items by id's of InfoItem
 */
class InfoSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    InfoSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}
    bool lessThan(const QModelIndex &left,const QModelIndex &right) const;
};

} // namespace Q4


#endif //KMLDONKEY_ABSTRACTINFOMODEL_H
