/* -*- c++ -*-
 *
 * infoiface.h
 *
 * Copyright (C) 2003 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
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

#ifndef __libkmldonkey_infoiface_h__
#define __libkmldonkey_infoiface_h__

#include <QObject>
#include <QVariant>

class InfoInterface
{
public:
    InfoInterface() {}
    ~InfoInterface() {}

/*TODO Did we need that?
    QVariant getProperty(const QString& key) {
        return properties[key];
    }
    QVariant getProperty(const QString& key, const QVariant defaultproperty) {
        return hasProperty(key) ? properties[key] : defaultproperty;
    }
    bool hasProperty(const QString& key) {
        return properties.contains(key);
    }
    void setProperty(const QString& key, const QVariant property) {
        properties.replace(key, property);
    }
    void removeProperty(const QString& key) {
        properties.remove(key);
    }

    const QMap<QString, QVariant> getProperties() {
        return properties;
    }
    void setProperties(const QMap<QString, QVariant> p) {
        properties = p;
    }
    void clearProperties() {
        properties.clear();
    }

protected:
    QMap<QString, QVariant> properties;
*/
};

#endif
