/* -*- c++ -*-
 *
 * systemtray.h
 *
 * Copyright (C) 2004, 2007 Sebastian Sauer <mail@dipe.org>
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

#include "systemtray.h"
#include "kmldonkey.h"
#include <KXMLGUIFactory>
#include <QIcon>
#include <QMenu>
SystemTray::SystemTray(QWidget* parent)
    : QSystemTrayIcon(QIcon::fromTheme("kmldonkey"), parent)
{
    auto* menu = new QMenu(parent);
    if (auto* popup = qobject_cast<QMenu*>(KMLDonkey::App->factory()->container("systemtray_actions", KMLDonkey::App)))
        menu->addActions(popup->actions());
    setContextMenu(menu);
    connect(this, &QSystemTrayIcon::activated, parent, [parent](ActivationReason reason) {
        if (reason == Trigger) {
            parent->setVisible(!parent->isVisible());
            if (parent->isVisible()) {
                parent->raise();
                parent->activateWindow();
            }
        }
    });
}
