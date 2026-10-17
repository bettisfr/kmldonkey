/* -*- c++ -*-
 *
 * abstractinfomodel.cpp
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

#include "abstractinfomodel.h"

#include <QStringList>

#include "kmldonkey.h"
#include "donkeyprotocol.h"

namespace Q4
{

QVariant InfoItem::data(int column, int role) const
{
    Q_ASSERT(column < m_data.size());
    if (role == Qt::EditRole) role = Qt::DisplayRole;

    const CellData &cellData = m_data.at(column);
    for (int i = 0; i < cellData.count(); ++i) {
        const RoleData &roleData = cellData.at(i);
        if (roleData.first == role) return roleData.second;
    }
    if (role == IdRole) return id;
    return QVariant();
}

void InfoItem::setData(int column, const QVariant &data, int role)
{
    if (m_data.size() <= column) m_data.resize(column + 1);

    CellData &cellData = m_data[column];
    bool done = false;
    for  (int i = 0; i < cellData.count(); ++i) {
        RoleData &roleData = cellData[i];
        if (roleData.first != role) continue;
        if (data.isNull()) {
            cellData.remove(i);
            updatedColumns |= 0x01<<column;
        } else if (roleData.second.metaType() != data.metaType() || roleData.second != data) {
            roleData.second = data;
            updatedColumns |= 0x01<<column;
        }
        done = true;
        break;
    }
    if (!done) {
        cellData << qMakePair(role, data);
        updatedColumns |= 0x01<<column;
    }
    if (role == Qt::DisplayRole) {
        setData(column, data, SortRole);
    }
}

int AbstractInfoModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : headers.size();
}

int AbstractInfoModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : itemsList.size();
}

void AbstractInfoModel::dropCache()
{
    //TODO:call this on language change
    headers.clear();
    foreach (const QString label, headerLabels()) {
        headers << label;
    }
    dropCacheImp();
}

QVariant AbstractInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return QVariant();
    return headers.at(section);
}


AbstractInfoModel::AbstractInfoModel(int columnCount, QObject *parent)
    : QAbstractTableModel(parent)
{
    headers.resize(columnCount);
    connect(KMLDonkey::App->donkey, SIGNAL(signalDisconnected(int)), SLOT(clear()));
}

AbstractInfoModel::~AbstractInfoModel()
{
    qDeleteAll(itemsList);
}

void AbstractInfoModel::clear()
{
    beginResetModel();
    qDeleteAll(itemsList);
    itemsList.clear();
    itemsHash.clear();

    endResetModel();
}

void AbstractInfoModel::removeItem(int num)
{
    InfoItem *item = itemsHash.take(num);
    Q_ASSERT(item);

    int row = item->row;

    beginRemoveRows(QModelIndex(), row, row);
    itemsList.removeAt(row);
    for (int i = row; i < itemsList.count(); ++i) {
        itemsList.at(i)->row--;
    }
    delete item;
    endRemoveRows();
}

void AbstractInfoModel::updateItem(int num)
{
    InfoItem *item = itemsHash.value(num);
    if (item) {
        item->updatedColumns = 0;
        item->update();
        //XXX:this hack is ugly and i'd be glad to get rid of it but QSortFilterProxyModel doesn't
        //preserve stable order otherwise
        int col = 0;
        while (item->updatedColumns) {
            bool upd = item->updatedColumns & 0x01;
            if (upd) emit dataChanged(index(item->row, col), index(item->row, col));
            ++col;
            item->updatedColumns >>= 1;
        }
    } else {
        item = prepareNewItem(num);
        if (!item) {
            Q_ASSERT(false);
            return;
        }
        item->row = rowCount();
        item->id = num;
        beginInsertRows(QModelIndex(), item->row, item->row);
        itemsHash.insert(num, item);
        itemsList << item;
        endInsertRows();
    }
}

QVariant AbstractInfoModel::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid()) return QVariant();
    InfoItem *item = itemsList.at(index.row());
    return item->data(index.column(), role);
}

InfoItem *AbstractInfoModel::itemAt(int row) const
{
    Q_ASSERT(row < rowCount());
    return itemsList.at(row);
}

bool InfoSortFilterProxyModel::lessThan(const QModelIndex &left,const QModelIndex &right) const
{
    bool less = QSortFilterProxyModel::lessThan(left, right);
    if (less) return true;
    bool more = QSortFilterProxyModel::lessThan(right,left);
    if (more) return false;
    //this way there will be no equal items and sorting will be stable
    return left.data(InfoItem::IdRole).toInt() < right.data(InfoItem::IdRole).toInt();
}

} // namespace Q4
