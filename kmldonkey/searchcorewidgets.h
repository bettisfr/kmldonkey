/* -*- c++ -*-
 *
 * searchcorewidgets.h
 *
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

#ifndef __kmldonkey_searchcorewidgets_h__
#define __kmldonkey_searchcorewidgets_h__

#include <QWidget>
#include <QList>
#include "autobox.h"

#include <searchquery.h>

class KLineEdit;
class KComboBox;
class SearchCore;
class SearchCoreWidgetContainer;

//! All Formwidgets are inherited from this base class.
class SearchCoreWidget : public VBoxWidget
{
public:
    /**
     * Constructor.
     * \param searchcore The SearchCore this SearchCoreWidget belongs to
     * \param parent The parent widget
     * \param container The parent-SearchCoreWidgetContainer of this
     *                  SearchCoreWidget or NULL if there is no parent
     */
    SearchCoreWidget(SearchCore*, QWidget *parent, SearchCoreWidgetContainer* container);

    /**
     * Returns the SearchCoreWidgetContainer this SearchCoreWidget is child of
     * or NULL if this SearchCoreWidget doesn't have a SearchCoreWidgetContainer.
     * \return The parent-SearchCoreWidgetContainer or NULL
     */
    SearchCoreWidgetContainer* getContainer() { return m_container; }

    /**
     * Returns true if this SearchCoreWidget is hidden else false. This
     * property got set to true if one of the SearchCoreWidgetContainer's
     * above this SearchCoreWidget is a SearchCoreWidgetHidden.
     * \return Is SearchCoreWidget hidden?
     */
    bool isHidden() { return m_hidden; }

    /**
     * Sets if this SearchCoreWidget is hidden. Normaly this function gots
     * only called if the SearchCoreWidgetContainer this SearchCoreWidget
     * is child of is hidden as well. So, the hidden-property is recursive.
     * \param hidden Is this SearchCoreWidget hidden?
     */
    void setHidden(bool hidden) { m_hidden = hidden; }

    /**
     * Returns the titletext for the widget.
     */
    virtual const QString getTitle() { return QString(); }

    /**
     * Returns the text the user puts into this widget.
     * \return The widget's text.
     */
    virtual const QString getValue() { return QString(); }

    /**
     * Returns a description of the SearchQuery. This function joins
     * recursive all SearchCoreWidget::getValue()'s containing
     * searchterms (like SearchCoreWidgetKeywords, but not
     * SearchCoreWidgetMinSize) together to a description.
     * \return A describing text of the search.
     */
    virtual const QString getDescription() { return QString(); }

    /**
     * Builds a SearchQuery and returns it. The SearchQuery gots recursive
     * constructed what means if the SearchCoreWidget is a
     * SearchCoreWidgetContainer also it's childrens are included.
     * Note that the return value could be NULL if getValue() of the
     * SearchCoreWidget returns an empty string. That way we got a minimal
     * sized SearchQuery back and empty SearchQueries arn't included.
     * \return A SearchQuery or NULL.
     */
    virtual SearchQuery* getQuery();

    /**
     * Set the query and build the Formwidget (and it's maybe avaible
     * child-Formwidgets as well).
     * \param query The SearchQuery
     * \param op The SearchCoreWidget::Operation
     * \return The SearchCoreWidget or NULL if the defined SearchQuery
     *         was invalid/empty.
     */
    virtual SearchCoreWidget* setQuery(SearchQuery *query);

protected:
    SearchCore* m_searchcore;
    QWidget* m_parent;
    bool m_hidden;
    SearchCoreWidgetContainer* m_container;
};

//! Generic Formwidget used as container for other Formwidgets.
class SearchCoreWidgetContainer : public SearchCoreWidget
{
public:
    SearchCoreWidgetContainer(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidget(c, parent, container) {}
    virtual const QString getDescription();
    virtual SearchCoreWidget* setQuery(SearchQuery *query);
protected:
    QList<SearchCoreWidget*> m_children;
    SearchQuery* getQueryList(SearchQueryList* querylist);
};

class SearchCoreWidgetAnd : public SearchCoreWidgetContainer
{
public:
    SearchCoreWidgetAnd(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetContainer(c, parent, container) {}
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetOr : public SearchCoreWidgetContainer
{
public:
    SearchCoreWidgetOr(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetContainer(c, parent, container) {}
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetAndNot : public SearchCoreWidgetContainer
{
public:
    SearchCoreWidgetAndNot(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container);
    ~SearchCoreWidgetAndNot();
    virtual const QString getDescription();
    virtual SearchQuery* getQuery();
    virtual SearchCoreWidget* setQuery(SearchQuery *query);
private:
    SearchCoreWidget *m_andmodule, *m_notmodule;
};

class SearchCoreWidgetHidden : public SearchCoreWidgetContainer
{
public:
    SearchCoreWidgetHidden(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetContainer(c, parent, container) {}
    virtual const QString getDescription() { return QString(); }
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetModule : public SearchCoreWidgetContainer
{
public:
    SearchCoreWidgetModule(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container);
    ~SearchCoreWidgetModule();
    virtual const QString getDescription();
    virtual SearchQuery* getQuery();
    virtual SearchCoreWidget* setQuery(SearchQuery *query);
private:
    QString m_name;
    SearchCoreWidget* m_module;
};

//! Generic Formwidget with QLabel and KLineEdit/KComboBox.
class SearchCoreWidgetEdit : public SearchCoreWidget
{
public:
    SearchCoreWidgetEdit(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container);
    virtual const QString getValue();
    virtual SearchCoreWidget* setQuery(SearchQuery *query);
protected:
    QString m_key;
    KLineEdit* m_edit;
    KComboBox* m_combo;
    SearchCoreWidget* setQuery(SearchQuery *query, const QStringList& items);
};

class SearchCoreWidgetKeywords : public SearchCoreWidgetEdit
{
public:
    SearchCoreWidgetKeywords(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetEdit(c, parent, container) {}
    virtual const QString getTitle();
    virtual const QString getDescription();
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetMinSize : public SearchCoreWidgetEdit
{
public:
    SearchCoreWidgetMinSize(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetEdit(c, parent, container) {}
    virtual const QString getTitle();
    virtual SearchCoreWidget* setQuery(SearchQuery *query);
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetMaxSize : public SearchCoreWidgetEdit
{
public:
    SearchCoreWidgetMaxSize(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetEdit(c, parent, container) {}
    virtual const QString getTitle();
    virtual SearchCoreWidget* setQuery(SearchQuery *query);
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetFormat : public SearchCoreWidgetEdit
{
public:
    SearchCoreWidgetFormat(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetEdit(c, parent, container) {}
    virtual const QString getTitle();
    virtual SearchCoreWidget* setQuery(SearchQuery *query);
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetMedia : public SearchCoreWidgetEdit
{
public:
    SearchCoreWidgetMedia(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetEdit(c, parent, container) {}
    virtual const QString getTitle();
    virtual SearchCoreWidget* setQuery(SearchQuery *query);
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetMp3Artist : public SearchCoreWidgetEdit
{
public:
    SearchCoreWidgetMp3Artist(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetEdit(c, parent, container) {}
    virtual const QString getTitle();
    virtual const QString getDescription();
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetMp3Title : public SearchCoreWidgetEdit
{
public:
    SearchCoreWidgetMp3Title(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetEdit(c, parent, container) {}
    virtual const QString getTitle();
    virtual const QString getDescription();
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetMp3Album : public SearchCoreWidgetEdit
{
public:
    SearchCoreWidgetMp3Album(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetEdit(c, parent, container) {}
    virtual const QString getTitle();
    virtual const QString getDescription();
    virtual SearchQuery* getQuery();
};

class SearchCoreWidgetMp3Bitrate : public SearchCoreWidgetEdit
{
public:
    SearchCoreWidgetMp3Bitrate(SearchCore* c, QWidget *parent, SearchCoreWidgetContainer* container) : SearchCoreWidgetEdit(c, parent, container) {}
    virtual const QString getTitle();
    virtual SearchCoreWidget* setQuery(SearchQuery *query);
    virtual SearchQuery* getQuery();
};

#endif // __kmldonkey_searchcore_h__
