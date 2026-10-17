/* -*- c++ -*-
 *
 * searchtabresult.h
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006      Christian Muehlhaeuser <chris@chris.de>
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

#ifndef __KMLDONKEY_SEARCHTABRESULT_H__
#define __KMLDONKEY_SEARCHTABRESULT_H__

#include "infolist.h"
#include "searchtabiface.h"

#include <KCoreAddons>
#include <ksqueezedtextlabel.h>

#include <QStringList>
#include <QLabel>

class QLabel;
class QTreeWidget;
class KComboBox;
class KLineEdit;
class HBoxWidget;

class SearchResultItem : public InfoItem
{
    public:
        SearchResultItem( QTreeWidget *parent, int num, const ResultInfo *searchinfo );
        ~SearchResultItem();

        virtual double numeric( int col ) const;
        virtual bool isNumeric( int col ) const;
		virtual void refresh();

        QString xtext( int column ) const;

        int getNum() const { return number; }
        const QString& getName() const { return name; }
        const QStringList& getNames() const { return names; }
        int64 getSize() const { return filesize; }
        const QString& getEd2kHash() const { return md4; }
        const QStringList& getUids() const { return uids; }

    private:
        int number;
        QString name, nwName, format, comment, md4;
        QStringList names, uids;
        int64 filesize;
        int32 availability;
        bool alreadydone;
};


class SearchResultView : public InfoList //, QToolTip
{
    Q_OBJECT

    public:
        SearchResultView( QWidget *parent = 0, const char *name = 0 );
};


class SearchResult : public SearchTabInterface
{
    Q_OBJECT

    public:
        SearchResult( QWidget *parent, const QString &tablabel, int sno );
        ~SearchResult();

        void addItem( const ResultInfo* searchinfo );
        void DownloadSelectedItems( bool force, bool askForFilename );

        int searchNo() { return id; }
        const QString& getTabLabel() { return tabLabel; }
        void setSearchLabel( const QString &text ) { searchLabel->setText( text ); }
        void setListFont( const QFont& font ) { resultView->QTreeWidget::setFont( font ); }

        SearchResultView *resultView;

    private:
        int visibleCount;

        HBoxWidget *statusBox;
        QStringList filters;
        KLineEdit *filterEdit;
        QTimer *filterTimer;
        KSqueezedTextLabel *searchLabel;
        QLabel *statusLabel;
        QStringList m_hashes;

        void setStatusLabel();
        bool filterItem( SearchResultItem *item );

    protected slots:
        void filterChanged();
        void filterTimerDone();
        void contextSearchResult( QTreeWidgetItem*,const QPoint& );
        void doubleClick( QTreeWidgetItem* );
};

#endif /* __KMLDONKEY_SEARCHTABRESULT_H__ */
