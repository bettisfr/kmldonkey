/* -*- c++ -*-
 *
 * imageloader.cpp
 *
 * Copyright (C) 2005 Petter Stokke <ummo@hellokitty.com>
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

#include "imageloader.h"

#include <QDebug>
#include <qpixmapcache.h>
//#include <QStandardPaths>
#include <kiconloader.h>

#include <network.h>

QPixmap ImageLoader::imageForNetwork(Network* network)
{
    QString iconname = network->networkName().toLower().replace(" ", "");

    QString cacheString("networkimage:" + iconname);
    QPixmap image;
    if (QPixmapCache::find(cacheString, &image))
        return image;

    image = KIconLoader::global()->loadIcon(iconname, KIconLoader::Small);
    if (image.isNull())
        image = KIconLoader::global()->loadIcon("unknown", KIconLoader::Small);
/*
    KStandardDirs dirs;
    QString path = dirs.findResource("data", "kmldonkey/icons/");
    QString filename(network->networkName());
    filename = filename.toLower().replace(" ", "").append(".png");
    qDebug() << "Generated path " << path << filename;
    if (!image.load(path + filename))
        image.load(path + "unknown.png");
*/
    QPixmapCache::insert(cacheString, image);
    return image;
}
