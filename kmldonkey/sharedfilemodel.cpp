#include <KIconLoader>
#include <QMimeData>
/* -*- c++ -*-
 *
 * sharedfilemodel.cpp
 *
 * Copyright (C) 2009       Aleksey Markelov <markelovai@gmail.com>
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

#include "sharedfilemodel.h"

#include <shareinfo.h>
#include <network.h>
#include <donkeyprotocol.h>
#include <KLocalizedString>
#include <QMimeDatabase>
#include <fileinfo.h>

#include "kmldonkey.h"

namespace Q4
{

class ShareItem : public InfoItem
{
public:
    enum ColumnName
    {
        FileName = 0,
        NetworkName,
        Requests,
        Uploaded,
        FileType
    };
    enum { ColumnCount = FileType + 1};
    ShareItem(int num) : InfoItem(num) { update(); }
    void update();
};

void ShareItem::update()
{
    ShareInfo *si = KMLDonkey::App->donkey->findShareNo(id); Q_ASSERT(si);
    setData(FileName, si->shareName());
    setData(NetworkName, KMLDonkey::App->donkey->findNetworkNo(si->shareNetwork())->networkName());
    setData(Requests, si->shareRequests());
    setData(Uploaded, FileInfo::humanReadableSize(si->shareUploaded()));
    setData(Uploaded, si->shareUploaded(), SortRole);

    QMimeType mime = QMimeDatabase().mimeTypeForFile(si->shareName(), QMimeDatabase::MatchExtension);
    setData(FileType, mime.comment());
    setData(FileName, KIconLoader::global()->loadIcon(mime.iconName(), KIconLoader::Small),
            Qt::DecorationRole);
}

SharedFileModel::SharedFileModel(QObject *parent) 
    : AbstractInfoModel(ShareItem::ColumnCount, parent)
{
    connect(KMLDonkey::App->donkey, SIGNAL(shareRemoved(int)), SLOT(removeItem(int)));
    connect(KMLDonkey::App->donkey, SIGNAL(shareUpdated(int)), SLOT(updateItem(int)));
    init();
}

Qt::ItemFlags SharedFileModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
}

QStringList SharedFileModel::mimeTypes() const
{
    return QStringList() << "text/plain" << "text/uri-list" << "text/html";
}

QMimeData *SharedFileModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) return 0;

    QStringList hashes;
    QStringList html;
    QList<QUrl> urls;
    foreach (const QModelIndex &index, indexes) {
        if (index.column() != 0) continue;
        InfoItem *item = itemAt(index);
        ShareInfo *si = KMLDonkey::App->donkey->findShareNo(item->id);
        if (!si) continue;
        QString name = si->shareName();
        QString hash = si->shareUid("ed2k");
        //what about root hash?
        QString url = QString("ed2k://|file|%1|%3|%2|/").arg(name.toUtf8().toPercentEncoding(),hash)
            .arg(si->shareSize());
        hashes << hash;
        html << QString("<a href=\"%1\">%2</a>").arg(url.toHtmlEscaped(),name.toHtmlEscaped());
        urls << url;
    }
    Q_ASSERT(!hashes.isEmpty());//otherwise view has disabled row selection
    QMimeData *data = new QMimeData();
    data->setText(hashes.join("\n"));
    data->setHtml(html.join("\n"));
    data->setUrls(urls);
    return data;
}

QStringList SharedFileModel::headerLabels() const
{
    return QStringList()
        << i18n("File name")
        << i18n("Network")
        << i18n("Requests")
        << i18n("Uploaded")
        << i18n("File type");
}

InfoItem *SharedFileModel::prepareNewItem(int num)
{
    return new ShareItem(num);
}

} // namespace Q4
