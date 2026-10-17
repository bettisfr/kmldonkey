/* -*- c++ -*-
*
* fileselector.h
*
* Copyright (C) 2004 Petter Stokke <ummo@hellokitty.com>
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

#ifndef __kmldonkey_fileselector_h__
#define __kmldonkey_fileselector_h__

#include <qwidget.h>

class KLineEdit;
class QPushButton;

class FileSelector : public QWidget
{
    Q_OBJECT
public:
    FileSelector( const QString& dialogCaption, bool selectDir, QWidget* parent = 0, const char* name = 0 );

    bool isExecutable();
    bool isDir();
    QString fileName();

protected slots:
    void slotButtonClicked();
    void slotTextChanged( const QString& t );

signals:
    void fileSelected( const QString& fileName );

private:
    KLineEdit* m_name;
    QPushButton* m_button;
    QString m_caption;
    bool m_selectDir;
};


#endif
