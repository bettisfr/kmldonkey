/* -*- c++ -*-
 *
 * searchquery.h
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003-2004 Sebastian Sauer <mail@dipe.org>
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

#ifndef __libkmldonkey_searchquery_h__
#define __libkmldonkey_searchquery_h__

#include <QString>
#include <QList>
#include <QObject>
#include <kmldonkey_export.h>

class DonkeyMessage;

//! Base class for query operations.
class KMLDONKEY_EXPORT SearchQuery : public QObject
{
    Q_OBJECT
    Q_ENUMS(Operation)
public:
    enum Operation {
        And = 0,
        Or,
        AndNot,
        Module,
        Keywords,
        MinSize,
        MaxSize,
        Format,
        Media,
        Mp3Artist,
        Mp3Title, // 10
        Mp3Album,
        Mp3Bitrate,
        Hidden
    };

    SearchQuery(Operation op);
    virtual ~SearchQuery();

    //! Creates a clone of this SearchQuery instance.
    virtual SearchQuery* cloneSearchQuery();

    //! Returns the operation of the SearchQuery
    Operation operation() const;

    //! Set the operation of the SearchQuery.
    /*! \param op The operation of the query. */
    void setOperation(Operation op);

    //! Returns the SearchQuery as string
    virtual const QString getQuerystring(bool simplify);

    //! Write this query into a DonkeyMessage.
    /*! \param msg The message to write to. */
    virtual void writeQuery(DonkeyMessage& msg);

    //! Returns a SearchQuery matching to the given DonkeryMessage.
    /*! \param msg The DonkeyMessage.
        \return SearchQuery-instance. */
    static SearchQuery* getQuery(DonkeyMessage* msg);

    //! Returns a SearchQuery matching to the given QueryString.
    /*! \param querystring The Querystring (see getQuerystring()).
        \return SearchQuery-instance. */
    static SearchQuery* getQuery(const QString& querystring);

public slots:
    QObject* clone();

    //! Returns the operation of the SearchQuery
    QByteArray queryOperation() const;

    //! Returns the SearchQuery as string
    QString queryString(bool simplify = true);

protected:
    Operation Op;
};



//! A base class for queries containing a list of other queries.
class KMLDONKEY_EXPORT SearchQueryList : public SearchQuery
{
    Q_OBJECT
public:
    SearchQueryList(Operation op);
    ~SearchQueryList();

    template <typename T>
    T* cloneSearchQueryList(T *parent) {
        foreach(SearchQuery *sq, m_queryList)
            parent->append( sq->cloneSearchQuery() );
        return parent;
    }

    virtual SearchQueryList* cloneSearchQuery();

    //! Append a query to the list.
    /*! Queries appended to the query list are owned by the list, and will be freed along with it.
     *  \param q The query to append.
     *  \return A pointer to the list. Useful for code like <tt>query->append(foo)->append(bar)->append(baz)</tt>.
     */
    SearchQueryList* append(SearchQuery* q);

    //! Returns the size of the attached list of queries.
    /*! \return The number of queries in this query list. */
    uint count();
    //! Returns a pointer to the query at a given index in the list.
    /*! \param index The index of the query to retrieve.
     *  \return Pointer to the query at the given index.
     */
    SearchQuery* at(uint index);

    virtual const QString getQuerystring(bool simplify);

    //! Write this query (with its attached queries) into a DonkeyMessage.
    /*! \param msg The message to write to. */
    virtual void writeQuery(DonkeyMessage& msg);

public slots:
    QList<QObject*> queryList() const;

protected:
    const QString toQueryString(bool simplify, const QString& joinstr);

private:
    QList<SearchQuery *> m_queryList;
};



//! A query containing a list of subqueries that must all match.
class KMLDONKEY_EXPORT QueryAnd : public SearchQueryList
{
public:
    QueryAnd();
    virtual QueryAnd* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

//! A query containing a list of subqueries of which at least one must match.
class KMLDONKEY_EXPORT QueryOr : public SearchQueryList
{
public:
    QueryOr();
    virtual QueryOr* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

class KMLDONKEY_EXPORT QueryHidden : public SearchQueryList
{
public:
    QueryHidden();
    virtual QueryHidden* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};


class KMLDONKEY_EXPORT QueryAndNot : public SearchQuery
{
    Q_OBJECT
public:
    QueryAndNot(SearchQuery* q1, SearchQuery* q2);
    ~QueryAndNot();
    virtual QueryAndNot* cloneSearchQuery();

    SearchQuery* getQueryAnd() { return query1; }
    SearchQuery* getQueryNot() { return query2; }

    virtual const QString getQuerystring(bool simplify);
    virtual void writeQuery(DonkeyMessage& msg);

public slots:
    QObject* queryAnd() const { return query1; }
    QObject* queryNot() const { return query2; }

private:
    SearchQuery *query1, *query2;
};



class KMLDONKEY_EXPORT QueryModule : public SearchQuery
{
    Q_OBJECT
public:
    QueryModule(QString str, SearchQuery* q);
    ~QueryModule();
    virtual QueryModule* cloneSearchQuery();

    const QString& getName() { return m_string; }
    SearchQuery* getQuery() { return m_query; }
    virtual const QString getQuerystring(bool simplify);
    virtual void writeQuery(DonkeyMessage& msg);

public slots:
    QString name() const { return m_string; }
    QObject* query() const { return m_query; }

private:
    QString m_string;
    SearchQuery* m_query;
};


class KMLDONKEY_EXPORT SearchQueryTwoStrings : public SearchQuery
{
    Q_OBJECT
public:
    SearchQueryTwoStrings(Operation op, QString str1, QString str2);
    virtual SearchQueryTwoStrings* cloneSearchQuery();

    virtual const QString getQuerystring(bool simplify);
    virtual void writeQuery(DonkeyMessage& msg);

public slots:
    QString key() { return s1; }
    void setKey(const QString& key) { s1 = key; }

    QString value() { return s2; }
    void setValue(const QString& value) { s2 = value; }

protected:
    QString s1, s2;
};

//! A query that specifies keywords to match.
class KMLDONKEY_EXPORT QueryKeywords : public SearchQueryTwoStrings
{
public:
    QueryKeywords(QString str1, QString str2);
    virtual QueryKeywords* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

//! A query that defines a minimum file size that must be matched.
class KMLDONKEY_EXPORT QueryMinSize : public SearchQueryTwoStrings
{
public:
    QueryMinSize(QString str1, QString str2);
    virtual QueryMinSize* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

//! A query that defines a maximum file size that must be matched.
class KMLDONKEY_EXPORT QueryMaxSize : public SearchQueryTwoStrings
{
public:
    QueryMaxSize(QString str1, QString str2);
    virtual QueryMaxSize* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

//! A query that specifies a file format to match.
class KMLDONKEY_EXPORT QueryFormat : public SearchQueryTwoStrings
{
public:
    QueryFormat(QString str1, QString str2);
    virtual QueryFormat* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

//! A query that specifies a media type to match.
class KMLDONKEY_EXPORT QueryMedia : public SearchQueryTwoStrings
{
public:
    QueryMedia(QString str1, QString str2);
    virtual QueryMedia* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

//! A query that specifies an ID3 artist tag for MP3 files.
class KMLDONKEY_EXPORT QueryMp3Artist : public SearchQueryTwoStrings
{
public:
    QueryMp3Artist(QString str1, QString str2);
    virtual QueryMp3Artist* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

//! A query that specifies an ID3 title tag for MP3 files.
class KMLDONKEY_EXPORT QueryMp3Title : public SearchQueryTwoStrings
{
public:
    QueryMp3Title(QString str1, QString str2);
    virtual QueryMp3Title* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

//! A query that specifies an ID3 album tag for MP3 files.
class KMLDONKEY_EXPORT QueryMp3Album : public SearchQueryTwoStrings
{
public:
    QueryMp3Album(QString str1, QString str2);
    virtual QueryMp3Album* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

//! A query that specifies a given MP3 bitrate to match.
class KMLDONKEY_EXPORT QueryMp3Bitrate : public SearchQueryTwoStrings
{
public:
    QueryMp3Bitrate(QString str1, QString str2);
    virtual QueryMp3Bitrate* cloneSearchQuery();
    virtual const QString getQuerystring(bool simplify);
};

#endif
