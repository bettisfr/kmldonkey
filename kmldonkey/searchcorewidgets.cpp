/* -*- c++ -*-
 *
 * searchcorewidgets.cpp
 *
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006 Christian Muehlhaeuser <chris@chris.de>
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

#include "searchcorewidgets.h"
#include "searchcore.h"
#include "search.h"

#include <QList>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>

#include <QDebug>
#include <KLocalizedString>
#include <klineedit.h>
#include <kcombobox.h>
#include "autobox.h"

SearchCoreWidget::SearchCoreWidget( SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container )
    : VBoxWidget( parent )
    , m_searchcore( c )
    , m_parent( parent )
    , m_hidden( false )
    , m_container( container )
{
    setMargin( 0 );
    setSpacing(8);
}


SearchQuery* SearchCoreWidget::getQuery()
{
    qDebug() << "SearchCoreWidget::getQuery() Unsupported!";
    return 0;
}


SearchCoreWidget* SearchCoreWidget::setQuery( SearchQuery *query )
{
    //qDebug() << "SearchCoreWidget::setQuery() query=" << query->getQuerystring();

    SearchCoreWidget* widget = 0;
    switch ( query->operation() )
    {
        case SearchQuery::And: widget = new SearchCoreWidgetAnd(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Or: widget = new SearchCoreWidgetOr(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Hidden: widget = new SearchCoreWidgetHidden(m_searchcore, m_parent, m_container); break;
        case SearchQuery::AndNot: widget = new SearchCoreWidgetAndNot(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Module: widget = new SearchCoreWidgetModule(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Keywords: widget = new SearchCoreWidgetKeywords(m_searchcore, m_parent, m_container); break;
        case SearchQuery::MinSize: widget = new SearchCoreWidgetMinSize(m_searchcore, m_parent, m_container); break;
        case SearchQuery::MaxSize: widget = new SearchCoreWidgetMaxSize(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Format: widget = new SearchCoreWidgetFormat(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Media: widget = new SearchCoreWidgetMedia(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Mp3Artist: widget = new SearchCoreWidgetMp3Artist(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Mp3Title: widget = new SearchCoreWidgetMp3Title(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Mp3Album: widget = new SearchCoreWidgetMp3Album(m_searchcore, m_parent, m_container); break;
        case SearchQuery::Mp3Bitrate: widget = new SearchCoreWidgetMp3Bitrate(m_searchcore, m_parent, m_container); break;

        default:
        {
            qDebug() << QString("SearchCoreWidget::setQuery() Unhandled operation %1").arg(query->operation());
            return 0;
        } break;
    }

    if ( !widget )
        qDebug() << QString( "SearchCoreWidget::setQuery() for operation %1 returned NULL" ).arg( query->operation() );
    else
    {
        SearchCoreWidgetContainer* container = widget->getContainer();
        if ( container )
            widget->setHidden( container->isHidden() );

        if ( !widget->setQuery( query ) )
        {
            qDebug() << QString( "SearchCoreWidget::setQuery() widget->setQuery() for operation %1 returned NULL" ).arg( query->operation() );
            delete widget;
            widget = 0;
        }
    }

    return widget;
}


const QString SearchCoreWidgetContainer::getDescription()
{
    QString s;
    for ( QList<SearchCoreWidget*>::Iterator it = m_children.begin(); it != m_children.end(); ++it )
    {
        QString t = (*it)->getDescription();
        if ( !t.isEmpty() ) s += t + QString( " " );
    }

    return s.trimmed();
}


SearchQuery* SearchCoreWidgetContainer::getQueryList( SearchQueryList* querylist )
{
    for ( QList<SearchCoreWidget*>::Iterator it = m_children.begin(); it != m_children.end(); ++it )
    {
        SearchQuery* q = (*it)->getQuery();
        if (q)
            querylist->append(q);
    }

    if ( querylist->count() < 1 )
    {
        delete querylist;
        querylist = 0;
    }
    /*
    else
        if ( querylist->count() == 1 ) // Simplify the query if possible
            return querylist->take( 0 );
    */

    return querylist;
}


SearchCoreWidget* SearchCoreWidgetContainer::setQuery( SearchQuery *query )
{
    //qDebug() << "SearchCoreWidgetContainer::setQuery() query=" << query->getQuerystring();

    if ( query->operation() == SearchQuery::Hidden )
        m_hidden = true;

    SearchQueryList* q = dynamic_cast<SearchQueryList*>( query );
    if ( !q )
    {
        qDebug() << "SearchCoreWidgetContainer::setQuery() Invalid SearchQueryList";
        return 0;
    }

    m_container = this;
    int i, j = q->count();

    for ( i = 0; i < j; i++ )
    {
        SearchCoreWidget* w = SearchCoreWidget::setQuery( q->at( i ) );
        if ( w ) {
            m_children.append( w );
        }
    }

    return ( m_children.count() > 0 ) ? this : 0;
}


SearchQuery* SearchCoreWidgetAnd::getQuery()
{
    //qDebug() << "SearchCoreWidgetAnd::getQuery()";

    return getQueryList( new QueryAnd() );
}


SearchQuery* SearchCoreWidgetOr::getQuery()
{
    //qDebug() << "SearchCoreWidgetOr::getQuery()";

    return getQueryList( new QueryOr() );
}


SearchCoreWidgetAndNot::SearchCoreWidgetAndNot( SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container )
    : SearchCoreWidgetContainer( c, parent, container )
    , m_andmodule( 0 )
    , m_notmodule( 0 )
{
}


SearchCoreWidgetAndNot::~SearchCoreWidgetAndNot()
{
    delete m_andmodule; m_andmodule = 0;
    delete m_notmodule; m_notmodule = 0;
}


const QString SearchCoreWidgetAndNot::getDescription()
{
    if ( !( m_andmodule && m_notmodule ) )
        return QString();

    QString s = m_andmodule->getDescription();
    QString t = m_notmodule->getDescription();
    if ( !t.isEmpty() )
        s += ( s.isEmpty() ) ? "-" + t : " -" + t;

    return s;
}


SearchQuery* SearchCoreWidgetAndNot::getQuery()
{
    //qDebug() << "SearchCoreWidgetAndNot::getQuery()";

    if ( !( m_andmodule && m_notmodule ) )
        return 0;

    SearchQuery* qand = m_andmodule->getQuery();
    SearchQuery* qnot = m_notmodule->getQuery();
    if ( qand && qnot )
        return new QueryAndNot( qand, qnot );

    delete qand;
    qand = 0;
    delete qnot;
    qnot = 0;

    return 0;
}


SearchCoreWidget* SearchCoreWidgetAndNot::setQuery( SearchQuery *query )
{
    //qDebug() << "SearchCoreWidgetAndNot::setQuery() query=" << query->getQuerystring();

    QueryAndNot* q = dynamic_cast<QueryAndNot*>( query );
    if ( !q )
        return 0;

    m_container = this;
    m_andmodule = SearchCoreWidget::setQuery( q->getQueryAnd() );
    m_notmodule = SearchCoreWidget::setQuery( q->getQueryNot() );
    if ( !( m_andmodule && m_notmodule ) )
    {
        delete m_andmodule;
        m_andmodule = 0;
        delete m_notmodule;
        m_notmodule = 0;

        return 0;
    }

    return this;
}


SearchQuery* SearchCoreWidgetHidden::getQuery()
{
    //qDebug() << "SearchCoreWidgetHidden::getQuery()";

    return getQueryList( new QueryHidden() );
}


SearchCoreWidgetModule::SearchCoreWidgetModule( SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container )
    : SearchCoreWidgetContainer( c, parent, container )
    , m_module( 0 )
{
}


SearchCoreWidgetModule::~SearchCoreWidgetModule()
{
    delete m_module;
    m_module = 0;
}


const QString SearchCoreWidgetModule::getDescription()
{
    return m_module ? m_module->getDescription() : QString();
}


SearchQuery* SearchCoreWidgetModule::getQuery()
{
    //qDebug() << "SearchCoreWidgetModule::getQuery()";
    if ( !m_module )
        return 0;

    /*
    SearchQuery *q = m_module->getQuery();
    if(! q) return 0;
    return new QueryModule(m_name, q);
    */

    return m_module->getQuery();
}


SearchCoreWidget* SearchCoreWidgetModule::setQuery( SearchQuery *query )
{
    //qDebug() << "SearchCoreWidgetModule::setQuery() query=" << query->getQuerystring();

    if ( query->operation() != SearchQuery::Module )
    {
        qDebug() << "SearchCoreWidgetModule::setQuery() query->operation()!=SearchQuery::Module This should never be the case!";
        SearchCoreWidget::setQuery( query );

        return 0;
    }

    QueryModule* q = dynamic_cast<QueryModule*>( query );
    if ( !q )
        return 0;

    m_name = q->getName();
    QGroupBox *grp = new QGroupBox( m_name, m_parent );
    grp->setAlignment(Qt::AlignLeft);
    QVBoxLayout* layout = new QVBoxLayout(grp);
    layout->setSpacing(8);
    grp->setLayout(layout);
    m_parent = grp;

    m_module = SearchCoreWidget::setQuery( q->getQuery() );

    return m_module ? this : 0;
}


SearchCoreWidgetEdit::SearchCoreWidgetEdit( SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container )
    : SearchCoreWidget( c, parent, container )
    , m_edit( 0 )
    , m_combo( 0 )
{
}


const QString SearchCoreWidgetEdit::getValue()
{
    return m_combo ? m_combo->currentText() : m_edit ? m_edit->text() : QString();
}


SearchCoreWidget* SearchCoreWidgetEdit::setQuery( SearchQuery *query )
{
    return SearchCoreWidgetEdit::setQuery( query, QStringList() );
}


SearchCoreWidget* SearchCoreWidgetEdit::setQuery( SearchQuery *query, const QStringList& items )
{
    //qDebug() << "SearchCoreWidgetEdit::setQuery() query=" << query->getQuerystring();
    SearchQueryTwoStrings* q = dynamic_cast<SearchQueryTwoStrings*>( query );
    if ( !q )
        return 0;

    auto* box = new QWidget(m_parent);
    box->setEnabled(!m_hidden);
    auto* form = new QFormLayout(box);
    form->setContentsMargins(0, 0, 0, 0);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setRowWrapPolicy(QFormLayout::WrapLongRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);

    if ( m_parent->layout() )
        m_parent->layout()->addWidget(box);

    m_key = q->key();

    //TODO The label should reflect the parent SearchQueryList if there is one!
    auto* label = new QLabel(getTitle(), box);
    label->setMinimumWidth(fontMetrics().horizontalAdvance(i18n("Keywords:")));
    if ( items.isEmpty() )
    {
        m_edit = new KLineEdit( box );
        form->addRow(label, m_edit);
        label->setBuddy(m_edit);
        m_edit->setClearButtonEnabled(true);
        if (query->operation() == SearchQuery::Keywords)
            m_edit->setPlaceholderText(i18n("Search for files…"));
        if ( isHidden() )
            m_edit->setText( m_key );

        connect( m_edit, SIGNAL( returnPressed() ), m_searchcore, SLOT( startSearch() ) );
    }
    else
    {
        m_combo = new KComboBox( box );
        form->addRow(label, m_combo);
        label->setBuddy(m_combo);
        m_combo->setEditable( true );
        m_combo->insertItem( 0, isHidden() ? m_key : QString( "" ) );
        m_combo->insertItems( 1, items );
        connect(m_combo->lineEdit(), SIGNAL(returnPressed()), m_searchcore, SLOT(startSearch()));
    }

    return this;
}


const QString SearchCoreWidgetKeywords::getTitle()
{
    return i18n( "Keywords:" );
}


const QString SearchCoreWidgetKeywords::getDescription()
{
    return getValue();
}


SearchQuery* SearchCoreWidgetKeywords::getQuery()
{
    //qDebug() << "SearchCoreWidgetKeywords::getQuery()";
    QString s = getValue();

    if ( s.isEmpty() )
    {
        qDebug() << "SearchCoreWidgetKeywords::getQuery() getValue().isEmpty()";
        return 0;
    }
    return new QueryKeywords( m_key, s );
}


const QString SearchCoreWidgetMinSize::getTitle()
{
    return i18n( "Min size:" );
}


SearchQuery* SearchCoreWidgetMinSize::getQuery()
{
    //qDebug() << "SearchCoreWidgetMinSize::getQuery()";
    QString s = getValue();
    if ( s.isEmpty() )
        return 0;

    return new QueryMinSize( m_key, QString::number( filesizeStr2Int( getValue() ) ) );
}


SearchCoreWidget* SearchCoreWidgetMinSize::setQuery( SearchQuery *query )
{
    return SearchCoreWidgetEdit::setQuery( query, QStringList() << "500 MB" << "100 MB" << "50 MB" << "3 MB" << "500 KB" << "500 B" );
}


const QString SearchCoreWidgetMaxSize::getTitle()
{
    return i18n( "Max size:" );
}


SearchQuery* SearchCoreWidgetMaxSize::getQuery()
{
    //qDebug() << "SearchCoreWidgetMaxSize::getQuery()";

    QString s = getValue();
    if ( s.isEmpty() )
        return 0;

    return new QueryMaxSize( m_key, QString::number( filesizeStr2Int( s ) ) );
}


SearchCoreWidget* SearchCoreWidgetMaxSize::setQuery( SearchQuery *query )
{
    return SearchCoreWidgetEdit::setQuery( query, QStringList() << "500 MB" << "100 MB" << "50 MB" << "3 MB" << "500 KB" << "500 B" );
}


const QString SearchCoreWidgetFormat::getTitle()
{
    return i18n( "Format:" );
}


SearchQuery* SearchCoreWidgetFormat::getQuery()
{
    //qDebug() << "SearchCoreWidgetFormat::getQuery()";

    QString s = getValue();
    if ( s.isEmpty() )
        return 0;

    return new QueryFormat( m_key, s );
}


SearchCoreWidget* SearchCoreWidgetFormat::setQuery( SearchQuery *query )
{
    //return SearchCoreWidgetEdit::setQuery( query, QStringList() << "avi" << "mp3" );
    return SearchCoreWidgetEdit::setQuery( query, QStringList() << "avi ogv ogm ogg mov mpg mpeg rmvb flv" << "mp3 ogg oga ogm flac wav" << "zip bzip gz rar ace tar 7z" );
}


const QString SearchCoreWidgetMedia::getTitle()
{
    return i18n( "Media:" );
}


SearchQuery* SearchCoreWidgetMedia::getQuery()
{
    //qDebug() << "SearchCoreWidgetMedia::getQuery()";

    QString s = getValue();
    if ( s.isEmpty() )
        return 0;

    return new QueryMedia( m_key, s );
}


SearchCoreWidget* SearchCoreWidgetMedia::setQuery( SearchQuery *query )
{
    return SearchCoreWidgetEdit::setQuery( query, QStringList() << "Audio" << "Video" << "Program" << "Image" << "Documentation" << "Collection" );
}


const QString SearchCoreWidgetMp3Artist::getTitle()
{
    return i18n( "Artist:" );
}


const QString SearchCoreWidgetMp3Artist::getDescription()
{
    return getValue();
}


SearchQuery* SearchCoreWidgetMp3Artist::getQuery()
{
    //qDebug() << "SearchCoreWidgetMp3Artist::getQuery()";

    QString s = getValue();
    if ( s.isEmpty() )
        return 0;

    return new QueryMp3Artist( m_key, s );
}


const QString SearchCoreWidgetMp3Title::getTitle()
{
    return i18n( "Title:" );
}


const QString SearchCoreWidgetMp3Title::getDescription()
{
    return getValue();
}


SearchQuery* SearchCoreWidgetMp3Title::getQuery()
{
    //qDebug() << "SearchCoreWidgetMp3Title::getQuery()";

    QString s = getValue();
    if ( s.isEmpty() )
        return 0;

    return new QueryMp3Title( m_key, s );
}


const QString SearchCoreWidgetMp3Album::getTitle()
{
    return i18n( "Album:" );
}


const QString SearchCoreWidgetMp3Album::getDescription()
{
    return getValue();
}


SearchQuery* SearchCoreWidgetMp3Album::getQuery()
{
    //qDebug() << "SearchCoreWidgetMp3Album::getQuery()";

    QString s = getValue();
    if ( s.isEmpty() )
        return 0;

    return new QueryMp3Album( m_key, s );
}


const QString SearchCoreWidgetMp3Bitrate::getTitle()
{
    return i18n( "Bitrate:" );
}


SearchQuery* SearchCoreWidgetMp3Bitrate::getQuery()
{
    //qDebug() << "SearchCoreWidgetMp3Bitrate::getQuery()";

    QString s = getValue();
    if (s.isEmpty() )
        return 0;

    return new QueryMp3Bitrate( m_key, s );
}


SearchCoreWidget* SearchCoreWidgetMp3Bitrate::setQuery( SearchQuery *query )
{
    return SearchCoreWidgetEdit::setQuery( query, QStringList() << "64" << "96" << "128" << "160" << "192" );
}

