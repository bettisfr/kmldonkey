/* -*- c++ -*-
 *
 * clipboard.cpp
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

#include <QClipboard>
#include <QUrl>
#include <QApplication>

#include "clipboard.h"

void ClipboardHelper::copyToClipboard(QStringList& values, ClipFormat format)
{
    QClipboard* clip = QApplication::clipboard();
    QStringList out;
    for ( QStringList::Iterator it = values.begin(); it != values.end(); ++it ) {
        QString name = *it;
        ++it;
        QString hash = *it;
        ++it;
        QString size = *it;
        const QString link = "ed2k://|file|" + QString::fromLatin1(QUrl::toPercentEncoding(name)) + "|" + size + "|" + hash + "|/";
        switch (format) {
        case Hash:
            out.append(hash);
            break;
        case URL:
            out.append(link);
            break;
        case HTML:
            out.append(QString("<a href=\"%1\">%2</a>").arg(link.toHtmlEscaped(), name.toHtmlEscaped()));
            break;
        default:
            break;
        }
    }
    if (clip->supportsSelection()) clip->setText(out.join("\n"), QClipboard::Selection);
    clip->setText(out.join("\n"), QClipboard::Clipboard);
}

