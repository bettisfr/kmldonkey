/* -*- c++ -*-
 *
 * availability.h
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

#ifndef __kmldonkey_availability_h__
#define __kmldonkey_availability_h__

#include <QFrame>
#include <QColor>
#include <QPixmap>
#include <QPaintEvent>
#include <QStyledItemDelegate>

#include <donkeyprotocol.h>

class AvailabilityRenderer
{
public:
    AvailabilityRenderer(int file);
    AvailabilityRenderer(int file, int source);
    FileInfo* fileInfo();
    int chunks() const;
    void loseCache();

    static QPixmap paint(const FileInfo *fi, const ClientInfo *ci = 0, QString *cacheKey = 0);
protected:
    void paintAvailability(QPainter& p, const QRect& r);
    int rfileno, clno, chunkNo;
    bool isClientRenderer;

private:
    void updateFileInfo();
    
    QPixmap m_cache;
};

class AvailabilityDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    AvailabilityDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, 
            const QModelIndex &index) const;
};

class AvailabilityWidget : public QFrame, public AvailabilityRenderer
{
    Q_OBJECT
public:
    AvailabilityWidget(int file, QWidget* parent = 0, const char* name = 0, bool frame = true);

    QSizePolicy sizePolicy() const;
    QSize minimumSizeHint() const;

protected:
    void paintEvent(QPaintEvent*);

};

#include <fileinfo.h>


#endif // __kmldonkey_availability_h__
