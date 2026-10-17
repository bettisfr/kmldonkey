/* -*- c++ -*-
 *
 * uploadersmodel.h
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

#ifndef KMLDONKEY_UPLOADERSMODEL_H
#define KMLDONKEY_UPLOADERSMODEL_H

#include "abstractinfomodel.h"

namespace Q4
{

class UploadersModel : public AbstractInfoModel
{
    Q_OBJECT
public:
    UploadersModel(QObject *parent = 0);

protected:
    virtual QStringList headerLabels() const;
    virtual InfoItem *prepareNewItem(int num);
};

} // namespace Q4

#endif //KMLDONKEY_UPLOADERSMODEL_H
