/* -*- c++ -*-
 *
 * kmlplasmaapplet.h
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

#ifndef KMLPLASMAAPPLET_H
#define KMLPLASMAAPPLET_H

#include <plasma/applet.h>
#include <plasma/containment.h>
#include <plasma/dataengine.h>
#include <plasma/theme.h>

#include <QAction>
#include <QPainter>
#include <QRect>
#include <QPointer>
#include <QVBoxLayout>
#include <QStyleOptionGraphicsItem>
#include <QCheckBox>
#include <QGroupBox>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

#include <KConfigDialog>
#include <KIcon>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KToolInvocation>
#include <KDebug>

class KmlPlasmaApplet : public Plasma::Applet
{
    Q_OBJECT
    public:
        KmlPlasmaApplet(QObject *parent, const QVariantList &args);
        virtual ~KmlPlasmaApplet();
        virtual void init();
        virtual QList<QAction*> contextualActions();
        virtual void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);

    public Q_SLOTS:
        void onSourceAdded(const QString&);
        void onSourceRemoved(const QString&);
        void dataUpdated(QString name, Plasma::DataEngine::Data data);

        void createConfigurationInterface(KConfigDialog *parent);
        void configAccepted();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        void dropEvent(QGraphicsSceneDragDropEvent *event);

    private:
        QPixmap m_pixmap;
        Plasma::DataEngine* m_engine;
        bool m_runOnClick, m_showLabels, m_connectionEnabled, m_rateEnabled, m_filesEnabled, m_transferEnabled, m_sharedEnabled;
        QString m_connection, m_rate, m_files, m_transfer, m_shared;
        QPointer<QWidget> m_widget;
        QCheckBox *m_runOnClickCheckBox, *m_showLabelsCheckBox, *m_connectionCheckBox, *m_rateCheckBox, *m_filesCheckBox, *m_transferCheckBox, *m_sharedCheckBox;

        void run();
};

K_EXPORT_PLASMA_APPLET(kmldonkey, KmlPlasmaApplet)

#endif
