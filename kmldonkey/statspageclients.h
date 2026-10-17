/* -*- c++ -*-
 *
 * statspageclients.h
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

#ifndef __kmldonkey_statspageclients_h__
#define __kmldonkey_statspageclients_h__

#include <QFrame>
#include <QScrollArea>
#include <QPaintEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QHideEvent>
#include <QList>

class VBoxWidget;

#include "statspage.h"
#include "donkeyprotocol.h"

class ClientsPieWidgetObject
{
public:
    QString label;
    double value;
    QColor color;
};

class ClientsPieWidget : public QFrame
{
public:
    ClientsPieWidget(QWidget *parent);
    ~ClientsPieWidget();
    void addValue(const QString& label, const double value, QColor color);

protected:
    virtual void paintEvent(QPaintEvent*);

private:
    QList<ClientsPieWidgetObject*> values;
};

class StatsPageClientsCallback : public ConsoleCallbackInterface
{
public:
    StatsPageClientsCallback();
    virtual ~StatsPageClientsCallback();
    void callback(const QString& cmd, const QString& res);
    StatsPageClients *page;
private:
    QList<QColor> colorsList;
    QList<QColor>::iterator colorsListIterator;
    QMap<QString, QColor> colorsMap;
};

class StatsPageClients : public QScrollArea
{
    Q_OBJECT
public:
    StatsPageClients(QWidget* parent);
    VBoxWidget *box;
public slots:
    void slotUpdate();
protected:
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);
private:
    StatsPageClientsCallback *callback;
    friend class StatsPageClientsCallback;
};

#endif
