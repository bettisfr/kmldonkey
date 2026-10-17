/* -*- c++ -*-
 *
 * kmlplasmaengine.h
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

#ifndef KMLPLASMAENGINE_H
#define KMLPLASMAENGINE_H

#include <QVariant>

#include <plasma/dataengine.h>

#include <donkeytypes.h>
#include <hostmanager.h>

class KmlPlasmaEngine : public Plasma::DataEngine
{
        Q_OBJECT
        Q_PROPERTY(QString hostName READ getHostName WRITE setHostName)

    public:
        KmlPlasmaEngine(QObject* parent, const QVariantList& args);
        virtual ~KmlPlasmaEngine();
        virtual QStringList sources() const;

        QString getHostName() const;
        void setHostName(const QString& hostname);

    protected:
        virtual void init();

    public Q_SLOTS:
        void configureConnections();
        void connectToHost(const QString& hostname = QString());

        void submitUrl(const KUrl& url);
        void submitUrls(const KUrl::List& urls);

    protected Q_SLOTS:
        void connectToHost(HostInterface* host);

    private Q_SLOTS:
        void updateHosts();
        void updateStatus(int64 ul, int64 dl, int64 sh, int nsh, int tul, int tdl, int uul, int udl, int ndl, int ncp, QMap<int,int>*);
        void donkeyConnected();
        void donkeyDisconnected(int err);

    private:
        class Private;
        Private *const d;
};

K_EXPORT_PLASMA_DATAENGINE(kmldonkey, KmlPlasmaEngine)

#endif
