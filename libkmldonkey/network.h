/* -*- c++ -*-
 *
 * network.h
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

#ifndef __libkmldonkey_network_h__
#define __libkmldonkey_network_h__

#include <QString>
#include <QStringList>
#include <kmldonkey_export.h>

#include "donkeytypes.h"

class DonkeyMessage;

//! Representation of a network.

class KMLDONKEY_EXPORT Network
{

public:

    enum NetworkFlags {
	NetworkHasServers    = 0x0001,
	NetworkHasRooms      = 0x0002,
	NetworkHasMultinet   = 0x0004,
	VirtualNetwork       = 0x0008,
	NetworkHasSearch     = 0x0010,
	NetworkHasChat       = 0x0020,
	NetworkHasSupernodes = 0x0040,
	NetworkHasUpload     = 0x0080
    };

    Network(int num, DonkeyMessage* msg, int proto);
    ~Network();

    void updateNetwork(DonkeyMessage *msg, int proto);

    //! The numerical ID of the network.
    const int& networkNo() const;
    //! The name of the network.
    const QString& networkName() const;
    //! Determine if the network is currently enabled.
    const bool& networkEnabled() const;
    //! The name of the network's configuration file.
    const QString& networkConfigFile() const;
    //! Number of bytes uploaded to the network.
    const int64& networkUploaded() const;
    //! Number of bytes downloaded from the network.
    const int64& networkDownloaded() const;
    //! Number of connected servers on the network.
    const int& networkConnected() const;
    //! The network flags.
    const NetworkFlags& networkFlags() const;

private:

    int num, connected;
    QString name, config_file;
    bool enabled;
    int64 uploaded, downloaded;
    NetworkFlags flags;

    Q_DISABLE_COPY(Network)

};

#endif
