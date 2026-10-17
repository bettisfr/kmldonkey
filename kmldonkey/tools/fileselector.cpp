#include <QFileInfo>
#include <QIcon>
/* -*- c++ -*-
*
* fileselector.cpp
*
* Copyright (C) 2004 Petter Stokke <ummo@hellokitty.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
*_ of the License, or (at your option) any later version.
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

#include "fileselector.h"

#include <QDebug>
#include <KLocalizedString>
#include <QPushButton>
#include <klineedit.h>
#include <kguiitem.h>
#include <QFileDialog>

#include <QHBoxLayout>

FileSelector::FileSelector( const QString& dialogCaption, bool selectDir, QWidget* parent, const char* name )
        : QWidget( parent )
        , m_caption( dialogCaption )
        , m_selectDir( selectDir )
{
    setObjectName(name);
    QHBoxLayout * l = new QHBoxLayout( this );
    m_name = new KLineEdit( this );
    m_name->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    l->addWidget( m_name );
    m_button = new QPushButton( QIcon::fromTheme("document-open"), QString(), this );
    m_button->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    l->addWidget( m_button );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

    connect( m_button, SIGNAL( clicked() ), this, SLOT( slotButtonClicked() ) );
    connect( m_name, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotTextChanged( const QString& ) ) );
}

void FileSelector::slotTextChanged( const QString& t )
{
    QFileInfo info( t );
    if ( ( m_selectDir && info.isDir() ) || info.exists() )
        emit fileSelected( t );
    else
        emit fileSelected( QString() );
}

void FileSelector::slotButtonClicked()
{
    QString path;
    if ( m_selectDir ) {
        path = QFileDialog::getExistingDirectory(this, m_caption, m_name->text());
    } else {
        path = QFileDialog::getOpenFileName(this, m_caption, m_name->text());
    }
    if ( !path.isEmpty() ) {
        m_name->setText( path );
    }
}

bool FileSelector::isExecutable()
{
    QFileInfo info( m_name->text() );
    return info.isExecutable();
}

bool FileSelector::isDir()
{
    QFileInfo info( m_name->text() );
    return info.isDir();
}

QString FileSelector::fileName()
{
    return m_name->text();
}


