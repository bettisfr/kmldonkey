/* -*- c++ -*-
 *
 * options.h
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

#ifndef __libkmldonkey_options_h__
#define __libkmldonkey_options_h__

#include <QString>

#include <kmldonkey_export.h>

class DonkeyMessage;

//! Representation of a file.

class KMLDONKEY_EXPORT DonkeyOption
{

public:
    /// Possible type values:
    enum OptionType {
        String = 0, ///< a text string
        Integer, ///< an integer value ("Integer" or "Int" type tag)
        Float, ///< a floating point value
        Boolean, ///< a boolean value, either \c "true" or \c "false"
        Ip, ///< an IP address
        IpList, ///< a list of IP addresses separated by spaces ("Ip List" type tag)
        Address, ///< an IP address and port number, separated by a color ("Addr" type tag)
        Md4, ///< an MD4 hash in hexadecimal form
        Sha1, ///< an SHA1 hash in hexadecimal form
        Unknown ///< value reserved for non-described types, should be treated as string?
    };
    DonkeyOption(DonkeyMessage* msg, int proto);
    DonkeyOption(const DonkeyOption&);
    DonkeyOption() { advanced = false; }
    ~DonkeyOption();

    //! The section the option should appear in.
    const QString& optionSection() const;
    //! The option label.
    const QString& optionLabel() const;
    //! The option name.
    const QString& optionName() const;
    //! The option type.
    OptionType type() const;
    //! The option's help string.
    const QString& optionHelp() const;
    //! The option's current value.
    const QString& optionValue() const;
    //! The option's default value.
    const QString& optionDefaultValue() const;
    //! The option's advanced flag.
    bool optionAdvanced() const;

private:

    QString section, label, name, help, value, defaultValue;
    OptionType m_type;
    bool advanced;

};

#endif
