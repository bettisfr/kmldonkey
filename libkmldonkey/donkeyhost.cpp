/* -*- c++ -*-
 *
 * donkeyhost.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003 Sebastian Sauer <mail@dipe.org>
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

#include "donkeyhost.h"

DonkeyHost::DonkeyHost(const QString& h_name, const QString& h_address, int h_port, int h_httpPort,
	       const QString& h_username, const QString& h_password,
	       HostInterface::HostType h_type,
	       const QUrl& h_binaryPath, const QUrl& h_rootPath,
	       HostInterface::StartupMode h_startupMode)
    : HostInterface(h_name, h_address, h_port, h_type, h_binaryPath, h_rootPath, h_startupMode)
    , m_username(h_username)
    , m_password(h_password)
    , m_httpPort(h_httpPort)
{
}

DonkeyHost::DonkeyHost(DonkeyHost& host)
    : HostInterface(host)
{
    m_username = host.username();
    m_password = host.password();
    m_httpPort = host.httpPort();
}

DonkeyHost::DonkeyHost()
    : HostInterface()
{
}

const QString& DonkeyHost::username() const { return m_username; }
const QString& DonkeyHost::password() const { return m_password; }
int DonkeyHost::httpPort() const { return m_httpPort; }

void DonkeyHost::setUsername(const QString& username) { m_username = username; }
void DonkeyHost::setPassword(const QString& password) { m_password = password; }
void DonkeyHost::setHttpPort(int httpPort) { m_httpPort = httpPort; }
