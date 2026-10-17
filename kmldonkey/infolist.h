/* -*- c++ -*-
 *
 * infolist.h
 *
 * Copyright (C) 2003, 2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006       Christian Muehlhaeuser <chris@chris.de>
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

#ifndef __INFOLIST_H__
#define __INFOLIST_H__

#include "availability.h"

#include <time.h>

#include <QHash>
#include <QTimer>
#include <QFocusEvent>
#include <QList>
#include <QEvent>
#include <QTreeWidget>
#include <QHeaderView>
#include <QStyledItemDelegate>

class QMenu;
class QAction;
class KSharedConfig;

/**
 * Our base ListView-class which extends QTreeWidget a bit.
 */
class InfoList : public QTreeWidget
{
    Q_OBJECT

    public:
        /**
        * Constructor.
        * \param parent The parent QWidget-object
        * \param name Name of this InfoList.
        * \param headervisiblepopup Show/hide columns per headerpopupmenu?
        */
        InfoList( QWidget* parent = 0, const char* name = 0, bool headervisiblepopup = false );
        /**
        * Destructor.
        */
        ~InfoList();

        /**
        * Initialization.
        * This function should be called if all columns are avaible (the addColumn()-
        * function was called before at least one time to have a valid column) to do
        * some initial-work.
        */
        void initialise();

        /**
        * Save the layout.
        * This function saves all the listview-settings like columnposition and
        * which columns are shown/hidden to the KConfig.
        */
        void saveLayout();

        /**
        * Same as QTreeWidget::selectedItems() except that this
        * fixes http://bugs.kde.org/show_bug.cgi?id=86756.
        * TODO If the bug got fixed in future we may like to remove
        * this function again.
        */
        QList<QTreeWidgetItem *> selectedItems() const;

        /**
        * Overloaded function to add a column to the InfoList.
        * \param label The Column-label, diplsayed in the header()
        * \param width Columnwidth in pixel.
        * \param resizeMode The column resize mode
        * \param hideColumn By default hide this column.
        * \return Position the column was added to.
        */
        int addColumn( const QString& label, int width = 90, QHeaderView::ResizeMode resizeMode = QHeaderView::Interactive, bool hideColumn = false );

        /**
        * Returns seconds the list gots automatically resorted or <=0
        * if auto-resort is disabled (default).
        * \return Frequency the list should be resorted in seconds.
        */
        int sortInterval();

        /**
        * Set the automatic sorting-frequency (<=0 to disable).
        * \param sec Frequency the list should be resorted in seconds.
        */
        void setSortInterval( int sec );
		
    signals:
        /**
        * Emitted if the InfoList got the focus.
        */
        void gotFocus();
        /**
        * Emitted if the InfoList lost the focus.
        */
        void lostFocus();
        /**
        * Emitted if the InfoList got or lost the focus.
        */
        void focusChanged();
		
		/**
		 * Emitted whenever the user requests context menu over an item
		 */
		void contextMenu( QTreeWidgetItem * item, const QPoint &pt );

    protected slots:
        void toggleColumnVisible( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers );
        void sortTimer();

    protected:
        virtual bool eventFilter( QObject*, QEvent* );
        virtual void focusInEvent( QFocusEvent* );
        virtual void focusOutEvent( QFocusEvent* );
		virtual void contextMenuEvent( QContextMenuEvent * cm );

		void restoreLayout( const KSharedConfig * cfg, const QString &name );
		void saveLayout( KSharedConfig * cfg, const QString &name );

	
	public:
		/**
		 * Internal itrem delegate used for special painting
		 */
		class ItemDelegate: public QStyledItemDelegate
		{
			public:
				ItemDelegate( QObject * parent = NULL );
				
			public:
				/// Returns pointer to parent info list
				InfoList * parentList() const;
				
				/// Routes paint event through the InfoItem's paint() own function
				virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
			
			// item edition:
			public:
				/// Creates appropiate editor for column
				virtual QWidget * createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
				
				/// Initializes editor data
				virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const;
				
				/// Routes editor change though the InfoItem's setText own function
				virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const;
		};
		
		ItemDelegate * itemDelegate() const;
		
    private:
        void showColumn( int );
        void hideColumn( int );

	private:
        QString m_name;
        QMap<int,QString> m_columns;
        QMap<int,int> m_columnWidth;
        QMap<int, QHeaderView::ResizeMode> m_columnResizeMode;
		QMap<QAction *, int> m_columnActions;
        int m_columnCount;

        int m_sortInterval;
        QTimer *m_sortTimer;

        QMenu *m_headerMenu;
        QStringList hideHeader;
		
		ItemDelegate * m_pDelegate;
};


/**
 * Used to cache InfoItem's.
 */
class InfoItemCacheEntry
{
    public:
        InfoItemCacheEntry() {}

        void update( const QString& str );
        void update( const QString& str, double num );
        int compare( const InfoItemCacheEntry* ) const;

        const QString& text() const { return m_str; }
        double numeric() const { return m_isNum ? m_num : 0.0; }
        bool isNumeric() const { return m_isNum; }

    private:
        QString m_str;
        double m_num;
        bool m_isNum;
};


/**
 * Base ListViewItem for InfoList's.
 */
class InfoItem : public QTreeWidgetItem
{
    public:
        InfoItem( QTreeWidget *parent, int file );
        ~InfoItem();

        QString text( int column ) const;
        virtual QString xtext( int column ) const = 0;
        void setFileNo( int file );
        int fileNo();

		int width() const;
		int height() const;
		
		virtual bool operator < (const QTreeWidgetItem &other ) const;
		
        virtual double numeric( int ) const;
        virtual bool isNumeric( int ) const;

        virtual void refresh();
		virtual void refreshColumn( int c, bool notify = true );
        const InfoItemCacheEntry* cacheEntry( int column ) const;

        virtual bool paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index );
		virtual QVariant data( int column, int role ) const;
		
		virtual void setData( int column, int role, const QVariant &value );
		
    protected:
        int fileno;
        QHash<int, InfoItemCacheEntry *> cache;
};


class DownloadFile : public InfoItem, public AvailabilityRenderer
{
    public:
        DownloadFile( QTreeWidget *parent, int file );
        ~DownloadFile() {}

		virtual void setData( int column, int role, const QVariant &value );
        virtual QString xtext( int column ) const;
		
		virtual double numeric( int col ) const;
        virtual bool isNumeric( int col ) const;

		virtual bool operator < (const QTreeWidgetItem &other ) const;
        virtual bool paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index );
        virtual void refresh();
        virtual void flush();

    private:
        time_t m_lastFlush;
};


class DownloadedFile : public InfoItem
{
    public:
        DownloadedFile( QTreeWidget *parent, int file );
        ~DownloadedFile() {}

        QString xtext( int column ) const;
        double numeric( int col ) const;
        bool isNumeric( int col ) const;
};


class ServerInfoItem : public InfoItem
{
    public:
        ServerInfoItem( QTreeWidget *parent, int server );
        ~ServerInfoItem() {}

		virtual void setData( int column, int role, const QVariant &value );
        virtual QString xtext( int column ) const;
        virtual double numeric( int col ) const;
        virtual bool isNumeric( int col ) const;

        virtual void refresh();
};


class ClientItem : public InfoItem
{
    public:
        ClientItem( QTreeWidget *parent, int client );
        ~ClientItem() {}

        virtual QString xtext( int column ) const;
        virtual void refresh();

        void setFilesListed( bool listed );

    private:
        bool filesListed;
};


class ResultInfo;

class ClientFile : public InfoItem
{
    public:
        ClientFile( QTreeWidget* parent, int file );
        ~ClientFile() {}

        virtual QString xtext( int column ) const;
        virtual double numeric( int col ) const;
        virtual bool isNumeric( int col ) const;
};

#endif /* __INFOLIST_H__ */
