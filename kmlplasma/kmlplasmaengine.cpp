/* -*- c++ -*-
 *
 * kmlplasmaengine.cpp
 *
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
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

#include "kmlplasmaengine.h"

#include <QTime>

#include <KDebug>
//#include <KIcon>

#include <plasma/datacontainer.h>

#include <hostmanager.h>
#include <hostdialog.h>
#include <hostselectaction.h>
#include <donkeyprotocol.h>
#include <donkeyhost.h>
#include <fileinfo.h>

class KmlPlasmaEngine::Private
{
    public:
        HostManager* hostManager;
        DonkeyProtocol* donkey;
        bool initDone;
        QString hostname;
};

KmlPlasmaEngine::KmlPlasmaEngine(QObject* parent, const QVariantList&) : Plasma::DataEngine(parent), d(new Private)
{
    d->hostManager = new HostManager(this);
    d->donkey = new DonkeyProtocol(true, this);
    d->initDone = false;
    connect(d->donkey, SIGNAL(clientStats(int64, int64, int64, int, int, int, int, int, int, int, QMap<int,int>*)),
            this, SLOT(updateStatus(int64, int64, int64, int, int, int, int, int, int, int, QMap<int,int>*)));
    connect(d->donkey, SIGNAL(signalConnected()), this, SLOT(donkeyConnected()));
    connect(d->donkey, SIGNAL(signalDisconnected(int)), this, SLOT(donkeyDisconnected(int)));

    //connect(d->donkey, SIGNAL(serverUpdated(int)), this, SLOT(serverUpdated(int)));
    //connect(d->donkey, SIGNAL(serverRemoved(int)), this, SLOT(serverRemoved(int)));
    //connect(d->donkey, SIGNAL(networkUpdated(int)), this, SLOT(networkUpdated(int)));
    //connect(d->donkey, SIGNAL(updatedDownloadFiles()), this, SLOT(updateDownloadFiles()));
    //connect(d->donkey, SIGNAL(updatedDownloadedFiles()), this, SLOT(updateDownloadedFiles()));
    //connect(d->donkey, SIGNAL(updatedConnectedServers()), this, SLOT(updateServers()));
    //connect(d->donkey, SIGNAL(consoleMessage(QString&)), this, SLOT(consoleMessage(QString&)));

    updateHosts();
    connect(d->hostManager, SIGNAL(hostListUpdated()), this, SLOT(updateHosts()));

    KAction* action = new KAction(KIcon("kmldonkey"), i18n("Connections..."), this);
    action->setObjectName("configure_connection");
    action->setIconText(i18n("Connections"));
    connect(action, SIGNAL(triggered()), this, SLOT(configureConnections()));

    HostSelectAction* hostaction = new HostSelectAction(i18n("Connect To..."), "network-connect", d->hostManager, this);
    connect(hostaction, SIGNAL(hostSelected(HostInterface*)), SLOT(connectToHost(HostInterface*)));
}

KmlPlasmaEngine::~KmlPlasmaEngine()
{
    delete d;
}

QStringList KmlPlasmaEngine::sources() const
{
    QStringList list = Plasma::DataEngine::sources();
    //kDebug() << "KmlPlasmaEngine::sources list=" << list;
    return list;
}

QString KmlPlasmaEngine::getHostName() const
{
    return d->hostname;
}

void KmlPlasmaEngine::setHostName(const QString& hostname)
{
    d->hostname = hostname;
    if( d->initDone )
        connectToHost(d->hostname);
}

void KmlPlasmaEngine::init()
{
    d->initDone = true;
    connectToHost(d->hostname);
}

void KmlPlasmaEngine::connectToHost(const QString& hostname)
{
    DonkeyHost* host = dynamic_cast<DonkeyHost*>( hostname.isEmpty() ? d->hostManager->defaultHost() : d->hostManager->hostProperties(hostname) );
    if( host )
        connectToHost(host);
}

void KmlPlasmaEngine::connectToHost(HostInterface* host)
{
    Q_ASSERT( host );
    kDebug() << "KmlPlasmaEngine::connectToHost host=" << host->name();
    d->hostname = host->name();
    d->donkey->setHost(host);
    d->donkey->connectToCore();
}

void KmlPlasmaEngine::updateHosts()
{
    kDebug() << "KmlPlasmaEngine::updateHosts";
    Plasma::DataEngine::Data data;
    data["names"] = d->hostManager->hostList();
    data["default"] = d->hostManager->defaultHostName();
    setData("hosts", data);
}

void KmlPlasmaEngine::updateStatus(int64 ul, int64 dl, int64 sh, int nsh, int tul, int tdl, int uul, int udl, int ndl, int ncp, QMap<int,int>*)
{
    //kDebug() << "KmlPlasmaEngine::updateStatus";
    Q_UNUSED(sh);
    Q_UNUSED(uul);
    Q_UNUSED(udl);

    Plasma::DataEngine::Data data;
    //data["servers_connected"] = d->donkey->connectedServerCount();
    //data["servers_total"] = d->donkey->totalServerCount();
    data["files_finished"] = ncp;
    data["files_downloading"] = ndl;
    data["shared_total"] = nsh;
    data["share_downloading"] = ndl;
    data["transfer_up"] = FileInfo::humanReadableSize(ul);
    data["transfer_down"] = FileInfo::humanReadableSize(dl);
    data["rate_up"] = tul / 1024.0;
    data["rate_down"] = tdl / 1024.0;
    setData("stats", data);
    //checkForUpdates();
}

void KmlPlasmaEngine::donkeyConnected()
{
    kDebug() << "KmlPlasmaEngine::donkeyConnected";
    HostInterface* host = d->donkey->getHost();
    Q_ASSERT(host);
    Plasma::DataEngine::Data data;
    data["name"] = host->name();
    data["address"] = host->address();
    data["port"] = host->port();
    data["username"] = d->donkey->username();
    data["password"] = d->donkey->password();
    setData("connection", data);
}

void KmlPlasmaEngine::donkeyDisconnected(int err)
{
    kDebug() << "KmlPlasmaEngine::donkeyDisconnected err=" << err;
    removeAllSources();
}

void KmlPlasmaEngine::configureConnections()
{
    HostDialog::showDialog(0, d->hostManager, d->donkey);
}

void KmlPlasmaEngine::submitUrl(const KUrl &url)
{
    kDebug() << "KmlPlasmaEngine::submitUrl url=" << url;
    d->donkey->submitUrl(url);
}

void KmlPlasmaEngine::submitUrls(const KUrl::List &urls)
{
    foreach(const KUrl &url, urls)
        submitUrl(url);
}

#include "kmlplasmaengine.moc"
