/* -*- c++ -*-
 *
 * searchinfo.cpp
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

#include <QRegularExpression>

#include "searchinfo.h"
#include "searchquery.h"
#include "donkeymessage.h"
#include "fileinfo.h"

/*** ResultInfo ***/

ResultInfo::ResultInfo(DonkeyMessage* msg, int proto)
{
    int i;
    bool ok = true;

    // set default values in the case we got one of those
    // buffer exceeds size"-error and have to break reading.
    size = 0;
    already_done = false;

    num = msg->readInt32();
    net = msg->readInt32();

    names = msg->readStringList();
    foreach (const QString &n, names) {
        if (name.length() < n.length()) name = n;
    }

    uids = proto > 26
        ? msg->readStringList()
        : QStringList( QString("urn:ed2k:") + FileInfo::md4ToString( msg->readMd4() ) );

    size = msg->readInt64();
    format = msg->readString(&ok);
    if(! ok) return;

    t = msg->readString(&ok);
    if(! ok) return;

    for (i = msg->readInt16(); i; i--) {
        if(! msg->readTag(tags)) return;
    }

    comment = msg->readString(&ok);
    if(! ok) return;

    already_done = msg->readInt8() > 0;

    if (proto > 26)
        time = msg->readInt32();
}

ResultInfo::ResultInfo(const ResultInfo& ri)
{
    num = ri.resultNo();
    name = ri.resultName();
    names = ri.resultNames();
    size = ri.resultSize();
    net = ri.resultNetwork();
    format = ri.resultFormat();
    tags = ri.resultTags();
    comment = ri.resultComment();
    already_done = ri.resultAlreadyDone();
    uids = ri.resultUids();
    time = ri.resultTime();
}

ResultInfo::~ResultInfo()
{
}

int ResultInfo::resultNo() const { return num; }
const QString& ResultInfo::resultName() const { return name; }
const QStringList& ResultInfo::resultNames() const { return names; }
int32 ResultInfo::resultSize() const { return size; }
int32 ResultInfo::resultNetwork() const { return net; }
const QString& ResultInfo::resultFormat() const { return format; }
const QMap<QString,QVariant>& ResultInfo::resultTags() const { return tags; }
const QString& ResultInfo::resultComment() const { return comment; }
bool ResultInfo::resultAlreadyDone() const { return already_done; }
int32 ResultInfo::resultTime() const { return time; }
const QStringList& ResultInfo::resultUids() const { return uids; }

QString ResultInfo::resultUid() const
{
    return uids.first();
}

QString ResultInfo::resultUid(const QString& type) const
{
    QRegularExpression match(QString("^urn:") + type + ":");
    QStringList results = uids.filter(match);
    if (!results.count()) return QString();

    QString result(results.first());
    result.remove(match);
    return result;
}

/*** SearchInfo ***/

SearchInfo::SearchInfo(int id)
    : QObject()
{
    num = id;
    fetched = false;
    query = 0;
    maxhits = -1;
    searchType = DonkeyProtocol::RemoteSearch;
    network = -1;
}

SearchInfo::SearchInfo(int id, SearchQuery* q, int mh, DonkeyProtocol::SearchType st, int nw)
    : QObject()
{
    num = id;
    fetched = false;
    query = q;
    maxhits = mh;
    searchType = st;
    network = nw;
}

SearchInfo::~SearchInfo()
{
}

int SearchInfo::searchNo() const
{
    return num;
}

SearchQuery* SearchInfo::getQuery()
{
    return query;
}

const QString SearchInfo::getQuerystring(bool simplified)
{
    if (simplified) {
        if(query && simpleQuerystring.isEmpty())
            simpleQuerystring = query->getQuerystring(true);
        return simpleQuerystring;
    } else {
        if(query && fullQuerystring.isEmpty())
            fullQuerystring = query->getQuerystring(false);
        return fullQuerystring;
    }
}

int SearchInfo::getMaxHits() const
{
    return maxhits;
}

DonkeyProtocol::SearchType SearchInfo::getSearchType() const
{
    return searchType;
}

int SearchInfo::getNetwork() const
{
    return network;
}

void SearchInfo::setQuery(DonkeyMessage* msg)
{
    delete query;
    query = SearchQuery::getQuery( msg->readString() );
    if (query)
        query->setParent(this);
    simpleQuerystring.clear(); fullQuerystring.clear(); // just set it dirty, on getQuerystring()-call it need to be re-readed.
    maxhits = msg->readInt32();
    searchType = (DonkeyProtocol::SearchType)msg->readInt8();
    network = msg->readInt32();
}

const ResultInfoHash& SearchInfo::searchResults() const
{
    return results;
}

void SearchInfo::addResult(ResultInfo* result)
{
    results.insert(result->resultNo(), result);
}

bool SearchInfo::isFetched() const
{
    return fetched;
}

void SearchInfo::setFetched(bool fetched_)
{
    fetched = fetched_;
}
