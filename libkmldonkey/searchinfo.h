/* -*- c++ -*-
 *
 * searchinfo.h
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

#ifndef __libkmldonkey_searchinfo_h__
#define __libkmldonkey_searchinfo_h__

#include <QString>
#include <QStringList>
#include <QVariant>
#include <kmldonkey_export.h>

#include "donkeytypes.h"
#include "donkeyprotocol.h"

class DonkeyMessage;
class SearchQuery;

//! Representation of an item in a search result.
class KMLDONKEY_EXPORT ResultInfo
{
public:
    ResultInfo(DonkeyMessage* msg, int proto);
    ResultInfo(const ResultInfo&);
    ~ResultInfo();

    //! The numerical ID of the result.
    int resultNo() const;
    //! The file name of the result.
    const QString& resultName() const;
    //! Alternative file names found on the network.
    const QStringList& resultNames() const;
    //! The size of the result in bytes.
    int32 resultSize() const;
    //! The numerical ID of the network the file was found on.
    int32 resultNetwork() const;
    //! The file format of the result.
    const QString& resultFormat() const;
    //! Result tags.
    const QMap<QString,QVariant>& resultTags() const;
    //! A comment found for the result.
    const QString& resultComment() const;
    //! Returns true if this file has already been downloaded.
    bool resultAlreadyDone() const;
    //! Return the list of uids relevant to this search (since protocol 27).
    const QStringList& resultUids() const;
    //! Return the first uid of the result, including the identifier string.
    QString resultUid() const;
    //! Return the uid for the given URN type, without the identifier string.
    QString resultUid(const QString& type) const;
    //! Return a timestamp of some sort. TODO: Figure out what this is.
    int32 resultTime() const;

private:
    int32 num;
    int32 net;
    QString name;
    QStringList names;
    int32 size;
    QString format;
    QString t;
    QMap<QString,QVariant> tags;
    QString comment;
    bool already_done;
    QStringList uids;
    int32 time;
};

typedef QHash<int, ResultInfo *> ResultInfoHash;

//! Representation of a search result.
class KMLDONKEY_EXPORT SearchInfo : public QObject
{
public:
    explicit SearchInfo(int id);
    explicit SearchInfo(int id, SearchQuery* query, int maxHits, DonkeyProtocol::SearchType searchType, int network);
    virtual ~SearchInfo();

    int searchNo() const;

    SearchQuery* getQuery();
    void setQuery(DonkeyMessage* msg);

    const QString getQuerystring(bool simplified);
    int getMaxHits() const;
    DonkeyProtocol::SearchType getSearchType() const;
    int getNetwork() const;

    const ResultInfoHash& searchResults() const;
    void addResult(ResultInfo* result);

    bool isFetched() const;
    void setFetched(bool fetched);

private:
    int32 num;
    ResultInfoHash results;
    bool fetched;

    SearchQuery* query;
    QString simpleQuerystring, fullQuerystring;
    int maxhits;
    DonkeyProtocol::SearchType searchType;
    int network;
};

#endif
