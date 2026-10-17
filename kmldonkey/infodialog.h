/* -*- c++ -*-
 *
 * infodialog.h
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

#ifndef __kmldonkey_infodialog_h__
#define __kmldonkey_infodialog_h__

#include "portedpagedialog.h"

class FileInfo;
class ClientList;
class PreviewWidget;
class AvailabilityWidget;
class KLineEdit;
class QListWidget;
class QFrame;

class FileInfoDialog : public PortedPageDialog
{
    Q_OBJECT
public:
    FileInfoDialog(int file);

protected slots:
    void saveSize();
    void updateFile(int file = -1);
    void updateCaption(FileInfo* fi);
    void namesViewContextMenu(const QPoint& pt);
    void namesViewRenameAction();

private:
    int fileno;

    KLineEdit *nameView, *sizeView, *hashView, *formatView,
	*totalSizeView, *etaView, *speedView, *networkView,
	*commentView;
    QListWidget *namesView;
    AvailabilityWidget* availView;
    ClientList* clientList;
    PreviewWidget* preview;
    QFrame* previewframe;

};

#endif // __kmldonkey_infodialog_h__
