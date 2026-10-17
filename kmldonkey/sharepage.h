/* -*- c++ -*-
 *
 * sharepage.h
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2009 Aleksey Markelov <markelovai@gmail.com>
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

#ifndef __kmldonkey_sharepage_h__
#define __kmldonkey_sharepage_h__

#include <QObject>
#include <QSplitter>
#include <QShowEvent>

#include "clipboard.h"
#include "page.h"

class QTimer;
class KActionCollection;

class KMLDonkeyPreferences;

class SharePage : public QWidget, public KMLDonkeyPage
{
    Q_OBJECT
public:
    SharePage(QWidget* parent);

    virtual void setupActions(KActionCollection*);
    virtual QStringList supportedGenericActions();
    virtual void handleGenericAction(const QString&);
    virtual void plugGenericActions(QObject*, const char*);
    virtual void deactivatePageActions();

    virtual void configurePrefsDialog(KMLDonkeyPreferences* prefs);
    virtual void applyPreferences(KMLDonkeyPreferences* prefs = 0);
    virtual void saveState(KSharedConfigPtr);
    virtual void restoreState(KSharedConfigPtr);

    virtual void clear();

    //void clearUploadersView();
    //void addUploadersViewItem(QMap<QString, QString> items);

signals:
    void genericActionsChanged(KMLDonkeyPage*);

protected slots:
    void uploadContextMenu(const QPoint&);
    void uploadersContextMenu(const QPoint&);
    void actionRefreshShared();
    void actionSharedDirs();

    void actionRefreshUploaders();

    void actionActivatePage();

    void clearUploadSelection();
    void clearUploadersSelection();

    void refreshLists();
    void updateTimer();

protected:
    void showEvent(QShowEvent* event);

private:
    QSplitter *uploadSplitter;
    QAbstractItemView *shareView, *uploadersView;

    QAction * refreshShared, * refreshUploaders;

    QTimer* refreshTimer;
    bool autoUpdate;
    int updateFrequency, activeFrequency;
};

#endif
