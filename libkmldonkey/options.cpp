/* -*- c++ -*-
 *
 * options.cpp
 *
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

#include "donkeymessage.h"
#include "options.h"

#include <QDebug>

inline DonkeyOption::OptionType typeFromName(const QString &typeName)
{
    static const char * const typeNames[] = { "String", "Integer", "Float",
        "Bool", "Ip", "Ip List",
        "Addr", "Md4", "Sha1" };

    if (typeName == "Int") {
        return DonkeyOption::Integer;
    }
    for (uint i = 0; i < sizeof(typeNames)/sizeof(typeNames[0]); ++i) {
        if (typeName == typeNames[i]) {
            return (DonkeyOption::OptionType)i;
        }
    }
    return DonkeyOption::Unknown;
}

DonkeyOption::DonkeyOption(DonkeyMessage* msg, int /*proto*/)
{
    bool ok = true;
    section = msg->readString(&ok);
    if (ok) {
        label = msg->readString();
        name = msg->readString();
        m_type = typeFromName( msg->readString() );
        help = msg->readString();
        value = msg->readString();
        defaultValue = msg->readString();
        advanced = msg->readBool();
    }
    else {
        qDebug()<<"DonkeyOption::DonkeyOption Failed to read string";
        advanced = false;
    }
}

DonkeyOption::DonkeyOption(const DonkeyOption& o)
{
    section = o.optionSection();
    label = o.optionLabel();
    name = o.optionName();
    m_type = o.type();
    help = o.optionHelp();
    value = o.optionValue();
    defaultValue = o.optionDefaultValue();
    advanced = o.optionAdvanced();
}

DonkeyOption::~DonkeyOption()
{
}

const QString& DonkeyOption::optionSection() const { return section; }
const QString& DonkeyOption::optionLabel() const { return label; }
const QString& DonkeyOption::optionName() const { return name; }
DonkeyOption::OptionType DonkeyOption::type() const { return m_type; }
const QString& DonkeyOption::optionHelp() const { return help; }
const QString& DonkeyOption::optionValue() const { return value; }
const QString& DonkeyOption::optionDefaultValue() const { return defaultValue; }
bool DonkeyOption::optionAdvanced() const { return advanced; }

