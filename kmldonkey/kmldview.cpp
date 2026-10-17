/* -*- c++ -*-
 *
 * kmldview.cpp
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

#include "kmldview.h"

#include <QHeaderView>
#include <QTreeView>

#include <KConfig>
#include <KConfigGroup>
#include <QDebug>
#include <KLocalizedString>
#include <QMenu>

#include "abstractinfomodel.h"
#include "kmldonkey.h"

KmldView::KmldView(QObject *parent)
    : QObject(parent)
{
    ;
}

QSortFilterProxyModel *KmldView::createProxy(QAbstractItemModel *model, QObject *parent)
{
    QSortFilterProxyModel *proxy = qobject_cast<Q4::AbstractInfoModel*>(model)
        ? new Q4::InfoSortFilterProxyModel(parent)
        : new QSortFilterProxyModel(parent);
    proxy->setSourceModel(model);
    adjustProxy(proxy);
    return proxy;
}

void KmldView::adjustProxy(QSortFilterProxyModel *proxy)
{
    proxy->setSortLocaleAware(true);
    proxy->setDynamicSortFilter(true);
    proxy->setSortRole(Q4::InfoItem::SortRole);
}

QAbstractItemView *KmldView::createViewSorted(QAbstractItemModel *model, 
        const QString &name, QWidget *parent)
{
    return createView(createProxy(model, parent), name, parent);
}


QAbstractItemView *KmldView::createView(QAbstractItemModel *model, const QString &name, QWidget *parent)
{
    QTreeView *view = new QTreeView(parent);
    view->setObjectName(name);
    view->setModel(model);
    if (qobject_cast<QSortFilterProxyModel*>(model))
        view->setSortingEnabled(true);
    view->setWordWrap(false);
    view->setFrameShape(QFrame::NoFrame);
    view->setFont(KMLDonkey::App->listFont);
    view->setUniformRowHeights(true);
    view->setRootIsDecorated(false);
    view->setAlternatingRowColors(true);
    //
    QHeaderView *header = view->header();
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header, SIGNAL(customContextMenuRequested(const QPoint &)),
            SLOT(headerMenu(const QPoint &)));

    KSharedConfigPtr conf = KSharedConfig::openConfig();
    KConfigGroup viewState = conf->group("State").group("Views");
    const QByteArray state = QByteArray::fromHex( viewState.readEntry(name, QByteArray()) );
    bool ok = header->restoreState(state);
    if (!ok) qDebug() << "restore state failed for view: " << name;

    viewList << view;
    return view;
}

void KmldView::saveStates()
{
    foreach (const QPointer<QAbstractItemView> &pview, viewList) {
        if (pview) saveState(pview);
    }
}

void KmldView::saveState(QAbstractItemView *view)
{
    QTreeView *tview = qobject_cast<QTreeView*>(view);
    Q_ASSERT(tview); if (!tview) return;
    KSharedConfigPtr conf = KSharedConfig::openConfig();
    KConfigGroup viewState = conf->group("State").group("Views");
    //workaround for BUG 190464 fixed in SVN commit 962585
    //reproduced in 4.2.2
    //fixed in 4.3 ,not sure about 4.2.x
    const QByteArray state = tview->header()->saveState().toHex();
    qDebug() << "saving state of view: " << tview->objectName() << state;
    viewState.writeEntry(tview->objectName(), state);
    viewState.sync();
}


void KmldView::headerMenu(const QPoint &pos)
{
    QHeaderView *header = qobject_cast<QHeaderView*>(sender());
    QAbstractItemModel *model = header->model();
    Q_ASSERT(header);
    QMenu menu;
    menu.addSection(i18n("Show"));
    const int count = header->count();
    for (int i = 0; i < count; ++i) {
        QAction *action = menu.addAction(model->headerData(i, header->orientation()).toString());
        action->setData(i);
        action->setCheckable(true);
        action->setChecked(!header->isSectionHidden(i));
    }
    QAction *action = menu.exec(header->mapToGlobal(pos));
    if (!action) return;
    header->setSectionHidden(action->data().toInt(), !action->isChecked());
}

void KmldView::optionsChanged()
{
    //TODO:here we'll change fonts and so on
}
