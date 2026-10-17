/* -*- c++ -*-
 *
 * searchquery.cpp
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003-2004 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2010 Gioacchino Mazzurco <gmazzurco89@gmail.com>
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

#include "searchquery.h"
#include "donkeymessage.h"
#include "donkeytypes.h"

#include <QRegExp>
#include <QStringList>
#include <QDebug>

/*** SearchQuery ***/

SearchQuery::SearchQuery(Operation op)
    : QObject()
{
    setOperation(op);
}

SearchQuery::~SearchQuery()
{
}

SearchQuery* SearchQuery::cloneSearchQuery()
{
    SearchQuery *sq = new SearchQuery(Op);
    return sq;
}

SearchQuery::Operation SearchQuery::operation() const
{
    return Op;
}

void SearchQuery::setOperation(Operation op)
{
    Op = op;
}

QByteArray SearchQuery::queryOperation() const
{
    switch(Op) {
        case And: return "And";
        case Or: return "Or";
        case AndNot: return "AndNot";
        case Module: return "Module";
        case Keywords: return "Keywords";
        case MinSize: return "MinSize";
        case MaxSize: return "MaxSize";
        case Format: return "Format";
        case Media: return "Media";
        case Mp3Artist: return "Mp3Artist";
        case Mp3Title: return "Mp3Title";
        case Mp3Album: return "Mp3Album";
        case Mp3Bitrate: return "Mp3Bitrate";
        case Hidden: return "Hidden";
    }
    return QByteArray();
}

const QString SearchQuery::getQuerystring(bool simplify)
{
    qDebug() << "SearchQuery::getQuerystring() NOT IMPLEMENTED !!!";
    Q_UNUSED(simplify);
    return QString();
}

QString SearchQuery::queryString(bool simplify)
{
    return getQuerystring(simplify);
}

QObject* SearchQuery::clone()
{
    return cloneSearchQuery();
}

void SearchQuery::writeQuery(DonkeyMessage& msg)
{
    msg.writeInt8((int8)Op);
}

SearchQuery* SearchQuery::getQuery(DonkeyMessage* msg)
{
    int op = msg->readInt8();
    switch (op) {
        case And: {
            QueryAnd* q = new QueryAnd();
            int i, j = msg->readInt16();
            for (i=0; i<j; i++)
                if (SearchQuery *q2 = getQuery(msg))
                    q->append( q2 );
            return q;
        } break;
        case Or: {
            QueryOr* q = new QueryOr();
            int i, j = msg->readInt16();
            for (i=0; i<j; i++)
                if (SearchQuery *q2 = getQuery(msg))
                    q->append( q2 );
            return q;
        } break;
        case Hidden: {
            QueryHidden* q = new QueryHidden();
            int i, j = msg->readInt16();
            for (i=0; i<j; i++)
                if (SearchQuery *q2 = getQuery(msg))
                    q->append( q2 );
            return q;
        } break;
        case AndNot: {
            SearchQuery *q1 = getQuery(msg);
            SearchQuery *q2 = getQuery(msg);
            return q1 && q2 ? new QueryAndNot(q1, q2) : 0;
        } break;
        case Module: {
            QString s = msg->readString();
            SearchQuery* q = getQuery(msg);
            return q ? new QueryModule(s, q) : 0;
        } break;
        case Keywords: {
            const QString key = msg->readString();
            const QString value = msg->readString();
            return new QueryKeywords(key, value);
        } break;
        case MinSize: {
            const QString key = msg->readString();
            const QString value = msg->readString();
            return new QueryMinSize(key, value);
        } break;
        case MaxSize: {
            const QString key = msg->readString();
            const QString value = msg->readString();
            return new QueryMaxSize(key, value);
        } break;
        case Format: {
            const QString key = msg->readString();
            const QString value = msg->readString();
            return new QueryFormat(key, value);
        } break;
        case Media: {
            const QString key = msg->readString();
            const QString value = msg->readString();
            return new QueryMedia(key, value);
        } break;
        case Mp3Artist: {
            const QString key = msg->readString();
            const QString value = msg->readString();
            return new QueryMp3Artist(key, value);
        } break;
        case Mp3Title: {
            const QString key = msg->readString();
            const QString value = msg->readString();
            return new QueryMp3Title(key, value);
        } break;
        case Mp3Album: {
            const QString key = msg->readString();
            const QString value = msg->readString();
            return new QueryMp3Album(key, value);
        } break;
        case Mp3Bitrate: {
            const QString key = msg->readString();
            const QString value = msg->readString();
            return new QueryMp3Bitrate(key, value);
        } break;
        default: break;
    }
    return 0;
}

SearchQuery* SearchQuery::getQuery(const QString& querystring)
{
    // First seek for SearchQueryList's
    QRegExp rxlist("(\\(.*\\))[\\s]*(AND|AND[\\s]*NOT|OR)[\\s]*(\\(.*\\))");
    int pos = rxlist.indexIn(querystring);
    if(pos >= 0) {
        QString op = rxlist.cap(2).trimmed().simplified();
        if(op == "AND") {
            QueryAnd* q = new QueryAnd();
            if (SearchQuery* q1 = getQuery(rxlist.cap(1)))
                q->append( q1 );
            if (SearchQuery* q2 = getQuery(rxlist.cap(3)))
                q->append( q2 );
            return q;
        }
        if(op == "OR") {
            QueryOr* q = new QueryOr();
            if (SearchQuery* q1 = getQuery(rxlist.cap(1)))
                q->append( q1 );
            if (SearchQuery* q2 = getQuery(rxlist.cap(3)))
                q->append( q2 );
            return q;
        }
        if(op == "AND NOT") {
            SearchQuery* q1 = getQuery(rxlist.cap(1));
            SearchQuery* q2 = getQuery(rxlist.cap(3));
            return q1 && q2 ? new QueryAndNot(q1, q2) : 0;
        }
        qDebug() << "SearchQuery::getQuery() Unknown list-operator " << op << " in " << querystring;
        return 0;
    }

    // Clean the querystring
    QRegExp rxq("^[\\s]*[\\(]+(.*)[\\)]+[\\s]*$");
    QString qs = (rxq.indexIn(querystring) >= 0) ? rxq.cap(1) : querystring;

    // Look for [...]>0 or [...]<0
    QRegExp rx1("\\[(.*)\\][\\s]*(>|<)[\\s]*\\[([0-9]+)\\]");
    if(rx1.indexIn(qs) >= 0) {
        QString op = rx1.cap(1).trimmed();
        if(op == "size") {
            QString sizeop = rx1.cap(2).trimmed();
            if(sizeop == ">")
                return new QueryMinSize(QString(), rx1.cap(3));
            else if(sizeop == "<")
                return new QueryMaxSize(QString(), rx1.cap(3));
            else
                qDebug() << "SearchQuery::getQuery() Unknown size-operator " << sizeop << " in " << qs;
        }
        if(op == "bitrate")
            return new QueryMp3Bitrate(QString(), rx1.cap(3));

        qDebug() << "SearchQuery::getQuery() Unknown integer-operator " << op << " in " << qs;
        return 0;
    }

    // look for [...]CONTAINS[...]
    QRegExp rx2("\\[(.*)\\][\\s]*CONTAINS[\\s]*\\[(.*)\\]");
    if(rx2.indexIn(qs) >= 0) {
        QString op = rx2.cap(1).trimmed();
        if(op == "format")
            return new QueryFormat(QString(), rx2.cap(2));
        if(op == "type")
            return new QueryMedia(QString(), rx2.cap(2));
        if(op == "artist")
            return new QueryMp3Artist(QString(), rx2.cap(2));
        if(op == "album")
            return new QueryMp3Album(QString(), rx2.cap(2));
        if(op == "title")
            return new QueryMp3Title(QString(), rx2.cap(2));

        qDebug() << "SearchQuery::getQuery() Unknown contains-operator " << op << " in " << qs;
        return 0;
    }

    // and finally look for CONTAINS[...]
    QRegExp rx3("CONTAINS[\\s]*\\[(.*)\\]");
    if(rx3.indexIn(qs) >= 0)
        return new QueryKeywords(QString(), rx3.cap(1));

    qDebug() << "SearchQuery::getQuery() Unknown querystring " << qs;
    return 0;
}

/*** SearchQueryList ***/

SearchQueryList::SearchQueryList(Operation op)
    : SearchQuery(op)
{
}


SearchQueryList::~SearchQueryList()
{
}

SearchQueryList* SearchQueryList::cloneSearchQuery()
{
    return cloneSearchQueryList( new SearchQueryList(operation()) );
}

SearchQueryList* SearchQueryList::append(SearchQuery* q)
{
    m_queryList.append(q);
    q->setParent(this);
    return this;
}

uint SearchQueryList::count()
{
    return m_queryList.count();
}

SearchQuery* SearchQueryList::at(uint index)
{
    return m_queryList.at(index);
}

const QString SearchQueryList::getQuerystring(bool simplify)
{
    qDebug() << "SearchQueryList::getQuerystring() NOT IMPLEMENTED !!!";
    Q_UNUSED(simplify);
    return QString();
}

const QString SearchQueryList::toQueryString(bool simplify, const QString& joinstr)
{
    QStringList qs;
    foreach (SearchQuery *q, m_queryList) {
        QString s = q->getQuerystring(simplify);
        if (!simplify || !s.isEmpty())
            qs << '(' + s + ')';
    }
    return qs.join(' ' + joinstr + ' ');
}

void SearchQueryList::writeQuery(DonkeyMessage& msg)
{
    SearchQuery::writeQuery(msg);
    msg.writeInt16((int16)m_queryList.count());
    foreach (SearchQuery *q, m_queryList) {
        q->writeQuery(msg);
    }
}

QList<QObject*> SearchQueryList::queryList() const
{
    QList<QObject*> result;
    foreach(SearchQuery  *sq, m_queryList)
        result.append(sq);
    return result;
}

/*** QueryAnd ***/

QueryAnd::QueryAnd()
    : SearchQueryList(SearchQuery::And)
{
}

QueryAnd* QueryAnd::cloneSearchQuery()
{
    return cloneSearchQueryList( new QueryAnd() );
}

const QString QueryAnd::getQuerystring(bool simplify)
{
    return toQueryString(simplify, "AND");
}

/*** QueryOr ***/

QueryOr::QueryOr()
    : SearchQueryList(SearchQuery::Or)
{
}

QueryOr* QueryOr::cloneSearchQuery()
{
    return cloneSearchQueryList( new QueryOr() );
}

const QString QueryOr::getQuerystring(bool simplify)
{
    return toQueryString(simplify, "OR");
}

/*** QueryHidden ***/

QueryHidden::QueryHidden()
    : SearchQueryList(SearchQuery::Hidden)
{
}

QueryHidden* QueryHidden::cloneSearchQuery()
{
    return cloneSearchQueryList( new QueryHidden() );
}

const QString QueryHidden::getQuerystring(bool simplify)
{
    return toQueryString(simplify, "HIDDEN");
}

/*** QueryAndNot ***/

QueryAndNot::QueryAndNot(SearchQuery* q1, SearchQuery* q2)
    : SearchQuery(SearchQuery::AndNot)
{
    query1 = q1;
    query2 = q2;
    query1->setParent(this);
    query2->setParent(this);
}

QueryAndNot::~QueryAndNot()
{
}

QueryAndNot* QueryAndNot::cloneSearchQuery()
{
    return new QueryAndNot(query1->cloneSearchQuery(), query2->cloneSearchQuery());
}

const QString QueryAndNot::getQuerystring(bool simplify)
{
    QString s1 = query1->getQuerystring(simplify);
    QString s2 = query2->getQuerystring(simplify);
    return simplify && s1.isEmpty() && s2.isEmpty() ? QString() : "(" + s1 + ") AND NOT (" + s2 + ")";
}

void QueryAndNot::writeQuery(DonkeyMessage& msg)
{
    SearchQuery::writeQuery(msg);
    query1->writeQuery(msg);
    query2->writeQuery(msg);
}

/*** QueryModule ***/

QueryModule::QueryModule(QString str, SearchQuery* q)
    : SearchQuery(SearchQuery::Module)
    , m_string(str)
    , m_query(q)
{
    m_query->setParent(this);
}

QueryModule::~QueryModule()
{
}

QueryModule* QueryModule::cloneSearchQuery()
{
    return new QueryModule(m_string, m_query->cloneSearchQuery());
}

const QString QueryModule::getQuerystring(bool simplify)
{
    return m_query->getQuerystring(simplify);
}

void QueryModule::writeQuery(DonkeyMessage& msg)
{
    SearchQuery::writeQuery(msg);
    msg.writeString(m_string);
    m_query->writeQuery(msg);
}

/*** SearchQueryTwoStrings ***/

SearchQueryTwoStrings::SearchQueryTwoStrings(Operation op, QString str1, QString str2)
    : SearchQuery(op)
    , s1(str1)
    , s2(str2)
{
}

SearchQueryTwoStrings* SearchQueryTwoStrings::cloneSearchQuery()
{
    return new SearchQueryTwoStrings( operation(), s1, s2 );
}

const QString SearchQueryTwoStrings::getQuerystring(bool simplify)
{
    qDebug() << "SearchQueryTwoStrings::getQuerystring() NOT IMPLEMENTED !!!";
    Q_UNUSED(simplify);
    return QString();
}

void SearchQueryTwoStrings::writeQuery(DonkeyMessage& msg)
{
    SearchQuery::writeQuery(msg);
    msg.writeString(s1);
    msg.writeString(s2);
}

/*** QueryKeywords ***/

QueryKeywords::QueryKeywords(QString s1, QString s2)
    : SearchQueryTwoStrings(SearchQuery::Keywords, s1, s2)
{
}

QueryKeywords* QueryKeywords::cloneSearchQuery()
{
    return new QueryKeywords( s1, s2 );
}

const QString QueryKeywords::getQuerystring(bool simplify)
{
    return simplify && s2.isEmpty() ? QString() : "CONTAINS[" + s2 + "]";
}

/*** QueryMinSize ***/

QueryMinSize::QueryMinSize(QString s1, QString s2)
    : SearchQueryTwoStrings(SearchQuery::MinSize, s1, s2)
{
}

QueryMinSize* QueryMinSize::cloneSearchQuery()
{
    return new QueryMinSize( s1, s2 );
}

const QString QueryMinSize::getQuerystring(bool simplify)
{
    return simplify && s2.isEmpty() ? QString() : "[size]>[" + s2 + "]";
}

/*** QueryMaxSize ***/

QueryMaxSize::QueryMaxSize(QString s1, QString s2)
    : SearchQueryTwoStrings(SearchQuery::MaxSize, s1, s2)
{
}

QueryMaxSize* QueryMaxSize::cloneSearchQuery()
{
    return new QueryMaxSize( s1, s2 );
}

const QString QueryMaxSize::getQuerystring(bool simplify)
{
    return simplify && s2.isEmpty() ? QString() : "[size]<[" + s2 + "]";
}

/*** QueryFormat ***/

QueryFormat::QueryFormat(QString s1, QString s2)
    : SearchQueryTwoStrings(SearchQuery::Format, s1, s2)
{
}

QueryFormat* QueryFormat::cloneSearchQuery()
{
    return new QueryFormat( s1, s2 );
}

const QString QueryFormat::getQuerystring(bool simplify)
{
    return simplify && s2.isEmpty() ? QString() : "[format]CONTAINS[" + s2 + "]";
}

/*** QueryMedia ***/

QueryMedia::QueryMedia(QString s1, QString s2)
    : SearchQueryTwoStrings(SearchQuery::Media, s1, s2)
{
}

QueryMedia* QueryMedia::cloneSearchQuery()
{
    return new QueryMedia( s1, s2 );
}

const QString QueryMedia::getQuerystring(bool simplify)
{
    return simplify && s2.isEmpty() ? QString() : "[type]CONTAINS[" + s2 + "]";
}

/*** QueryMp3Artist ***/

QueryMp3Artist::QueryMp3Artist(QString s1, QString s2)
    : SearchQueryTwoStrings(SearchQuery::Mp3Artist, s1, s2)
{
}

QueryMp3Artist* QueryMp3Artist::cloneSearchQuery()
{
    return new QueryMp3Artist( s1, s2 );
}

const QString QueryMp3Artist::getQuerystring(bool simplify)
{
    return simplify && s2.isEmpty() ? QString() : "[artist]CONTAINS[" + s2 + "]";
}

/*** QueryMp3Title ***/

QueryMp3Title::QueryMp3Title(QString s1, QString s2)
    : SearchQueryTwoStrings(SearchQuery::Mp3Title, s1, s2)
{
}

QueryMp3Title* QueryMp3Title::cloneSearchQuery()
{
    return new QueryMp3Title( s1, s2 );
}

const QString QueryMp3Title::getQuerystring(bool simplify)
{
    return simplify && s2.isEmpty() ? QString() : "[title]CONTAINS[" + s2 + "]";
}

/*** QueryMp3Album ***/

QueryMp3Album::QueryMp3Album(QString s1, QString s2)
    : SearchQueryTwoStrings(SearchQuery::Mp3Album, s1, s2)
{
}

QueryMp3Album* QueryMp3Album::cloneSearchQuery()
{
    return new QueryMp3Album( s1, s2 );
}

const QString QueryMp3Album::getQuerystring(bool simplify)
{
    return simplify && s2.isEmpty() ? QString() : "[album]CONTAINS[" + s2 + "]";
}

/*** QueryMp3Bitrate ***/

QueryMp3Bitrate::QueryMp3Bitrate(QString s1, QString s2)
    : SearchQueryTwoStrings(SearchQuery::Mp3Bitrate, s1, s2)
{
}

QueryMp3Bitrate* QueryMp3Bitrate::cloneSearchQuery()
{
    return new QueryMp3Bitrate( s1, s2 );
}

const QString QueryMp3Bitrate::getQuerystring(bool simplify)
{
    return simplify && s2.isEmpty() ? QString() : "[bitrate]>[" + s2 + "]";
}


