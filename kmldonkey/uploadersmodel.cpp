#include <QLocale>
/* -*- c++ -*-
 *
 * uploadersmodel.cpp
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

#include "uploadersmodel.h"

#include <QMimeDatabase>
#include <KIconLoader>
#include <KLocalizedString>

#include <donkeyprotocol.h>
#include <clientinfo.h>
#include <fileinfo.h>
#include <network.h>

#include "kmldonkey.h"
#include "tools/imageloader.h"

namespace Q4
{

class UploaderItem : public InfoItem
{
public:
    enum ColumnName {
        FileName = 0,
        ClientName,
        Downloaded,
        Uploaded,
        State,
        NetworkName,
        ClientRating,
        ConnectionTime,
        ClientSoftware,
        FileType
    };
    enum { ColumnCount = FileType + 1};
    UploaderItem(int num) : InfoItem(num) { update(); }
    void update();
};

QVariant client_state(const ClientInfo *ci)
{
    switch (ci->clientState()) {
    case ClientInfo::NotConnected:
    case ClientInfo::NotConnected2:
        return i18nc("not connected to host", "Not connected");
        break;
    case ClientInfo::Connecting:
        return i18nc("connecting to host", "Connecting");
        break;
    case ClientInfo::Initiating:
         return i18nc("initiating connection to host", "Initiating");
         break;
    case ClientInfo::Downloading: 
         return i18nc("downloading from host", "Downloading");
         break;
    case ClientInfo::Connected:
    case ClientInfo::Connected3: 
         return i18nc("connected to host", "Connected");
         break;
    case ClientInfo::Connected2: 
         return i18nc("we're at this position in client's queue", "Queued: %1", 
                 QLocale().toString(double(ci->clientQueuePosition()), 'f', 0));
         break;
    case ClientInfo::NewHost: 
         return i18nc("new host added to list", "New host");
         break;
    case ClientInfo::Removed: 
         return i18nc("host has been removed", "Removed");
         break;
    case ClientInfo::Blacklisted: 
         return i18nc("host is blacklisted", "Blacklisted");
         break;
    default: 
         return i18nc("unknown host state (unknown ID code)", "Unknown (%1)", ci->clientState());
    }
}


void UploaderItem::update()
{
    const ClientInfo *ci = KMLDonkey::App->donkey->findClientNo(id); Q_ASSERT(ci);
    setData(FileName, ci->clientUpload());
    setData(ClientName, ci->clientName());

    setData(Downloaded, FileInfo::humanReadableSize(ci->clientDownloaded()));
    setData(Downloaded, ci->clientDownloaded(), SortRole);

    setData(Uploaded, FileInfo::humanReadableSize(ci->clientUploaded()));
    setData(Uploaded, ci->clientUploaded(), SortRole);

    setData(State, client_state(ci));

    Network *net = KMLDonkey::App->donkey->findNetworkNo(ci->clientNetwork());
    setData(NetworkName, net->networkName());
    setData(NetworkName, ImageLoader::imageForNetwork(net), Qt::DecorationRole);

    setData(ClientRating, ci->clientRating());

    setData(ConnectionTime, FileInfo::humanReadableTime(ci->clientConnectTime(), true));
    setData(ConnectionTime, ci->clientConnectTime(), SortRole);

    setData(ClientSoftware, ci->clientSoftware() + ' ' + ci->clientEmuleMod());
    //TODO: get nice icons for decoration role
    //setData(ClientSoftware, soft_icon(ci->clientSoftware()), Qt::DecorationRole);

    QMimeType mime = QMimeDatabase().mimeTypeForFile(ci->clientUpload(), QMimeDatabase::MatchExtension);
    setData(FileType, mime.comment());
    setData(FileName,
            KIconLoader::global()->loadIcon(mime.iconName(), KIconLoader::Small),
            Qt::DecorationRole);
}

QStringList UploadersModel::headerLabels() const
{
    return QStringList() << i18n( "File name" )
        << i18n( "Name" )
        << i18n( "Downloaded" )
        << i18n( "Uploaded" )
        << i18n( "Status" )
        << i18n( "Network" )
        << i18n( "Rating" )
        << i18n( "Time" )
        << i18n( "Software" )
        << i18n( "File type" );
}

UploadersModel::UploadersModel(QObject *parent)
    : AbstractInfoModel(UploaderItem::ColumnCount, parent)
{
    connect(KMLDonkey::App->donkey, SIGNAL(uploadRemoved(int)), SLOT(removeItem(int)));
    connect(KMLDonkey::App->donkey, SIGNAL(uploadUpdated(int)), SLOT(updateItem(int)));
    init();
}

InfoItem *UploadersModel::prepareNewItem(int num)
{
    return new UploaderItem(num);
}

} // namespace Q4
