/* -*- c++ -*-
 *
 * statspageovernet.cpp
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

#include <QLabel>
#include <QSplitter>
#include <QRegExp>
#include <QShowEvent>
#include <QHideEvent>
#include <QHBoxLayout>

#include <QDebug>
#include <KLocalizedString>
#include <QTreeWidget>
#include <QTextBrowser>

#include "kmldonkey.h"
#include "donkeyprotocol.h"
#include "prefs/prefs.h"

#include "statspageovernet.h"

StatsPageOvernetCallback::StatsPageOvernetCallback(StatsPageOvernet *p)
    : page(p)
{
}

StatsPageOvernetCallback::~StatsPageOvernetCallback()
{
}


void StatsPageOvernetCallback::callback(const QString& cmd, const QString& res)
{
    if ( !page )
        return;

    if (cmd.endsWith("md4"))
        page->parseOvmd4(res);
    if (cmd.endsWith("stats"))
        page->parseOvstat(res);
    else if (cmd.endsWith("store"))
        page->parseOvstore(res);
}

StatsPageOvernet::StatsPageOvernet(QWidget *parent)
    : QScrollArea(parent)
{
    box = new VBoxWidget( this );
        setViewport( box );
    box->show();
}


void StatsPageOvernet::parseOvmd4(const QString &res)
{
    addText(res);
}

void StatsPageOvernet::parseOvstat(const QString &res)
{
    QTreeWidget *view;
    QTreeWidget *viewFile = 0;
    QTreeWidget *viewKeyword = 0;
    QTextBrowser *browser = 0;

    int count, pos;
    QString s, t;
    QStringList values;
    QStringList list = res.split("\n");
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        s = *it;

        QRegExp rx("^Search (file|keyword) for[\\s]*([a-zA-Z0-9]+)[\\s]*\\((.*)\\).*");
        if (rx.indexIn(s) >= 0) {
            if(rx.cap(1) == "file") {
                if (! viewFile) {
                    viewFile = new QTreeWidget(box);
					viewFile->setColumnCount( 1 );
					viewFile->headerItem()->setText( 0 , i18n("Search filehash") ); 
                    addItem( viewFile );
                }
                view = viewFile;
            }
            else {
                if (! viewKeyword) {
                    viewKeyword = new QTreeWidget(box);
					viewKeyword->setColumnCount( 1 );
					viewKeyword->headerItem()->setText( 0 , i18n("Search keyword") ); 
                    addItem( viewKeyword );
                }
                view = viewKeyword;
            }
            QTreeWidgetItem *item = new QTreeWidgetItem(view);
            item->setText(0, rx.cap(2));
	    QString str = rx.cap(3);
            values = str.split(",");
            count = 1;
            for (QStringList::Iterator it = values.begin(); it != values.end(); ++it) {
                s = (*it).trimmed();
                t = QString("");
                pos = s.indexOf(" ");
                if (pos > 0) {
                    t = s.mid(pos + 1, s.length() - pos - 1).replace("_", " ").trimmed();
                    s = s.remove(pos, s.length() - pos).trimmed();
                }
                if (view->columnCount() <= count)
				{
					view->setColumnCount( view->columnCount() + 1 );
					view->headerItem()->setText( view->columnCount() - 1 , t ); 
				}
                item->setText(count, s);
                count++;
            }
            continue;
        }

        if (s.isEmpty() || s.startsWith("Overnet statistics")) continue;

        QRegExp rxHits("(Search|Source)[\\s]+hits[\\s]*:(.*)");
        if (rxHits.indexIn(s) >= 0) {
            if (rxHits.cap(1).startsWith("Se"))
                addText( i18n("Search hits: %1",rxHits.cap(2).trimmed()) );
            else
                addText( i18n("Source hits: %1",rxHits.cap(2).trimmed()) );
            continue;
        }

        if (! browser) {
            browser = new QTextBrowser(box);
            addItem( browser );
        }
        browser->append( s );
    }
}

void StatsPageOvernet::parseOvstore(const QString &res)
{
    QString s;
    QStringList list = res.split("\n");
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        s = (*it).trimmed();
        if (s.isEmpty() || s.startsWith("Overnet store")) continue;

        QRegExp rx("^size[\\s]*\\=[\\s]*([0-9]+)[\\s]*\\,[\\s]*max_size[\\s]*\\=[\\s]*([0-9]+).*");
        if(rx.indexIn(s) >= 0) {
            addText(i18n("Size: %1",rx.cap(1)));
            addText(i18n("Max size: %1",rx.cap(2)));
        }
        else {
            /*TODO
            md4=F665E93FA8785A7A698EC5E56C0164EF, files =
            r_md4=815DD38583ADE072D70C32A21DF96D32
            tags=loc = "bcp://813dd385a85df092d70c32ea25df86832:212.212.22.2:8080"
            */
        }
    }
}

void StatsPageOvernet::addText( const QString &text )
{
    auto *label = new QLabel(text.trimmed(), box);
    label->setTextFormat(Qt::PlainText);
    addItem(label);
}

void StatsPageOvernet::addItem( QWidget * item )
{
    box->layout()->addWidget( item );
    item->show();

    items.push_back( item );
}

void StatsPageOvernet::clearItems()
{
    foreach( QWidget * item, items )
    {
        box->layout()->removeWidget( item );
        delete item;
    }
    items.clear();
}

void StatsPageOvernet::slotUpdate()
{
    clearItems();

    if (! KMLDonkey::App->donkey->isConnected())
        return;

    StatsPageOvernetCallback * callback = new StatsPageOvernetCallback(this);

    //FIXME cause of bug http://bugs.kde.org/show_bug.cgi?id=87123
    //      we just send both kind of messages. That's not nice and
    //      3 of them will fail, but it's so far the easiest way
    //      to get right of those (let us name it stupid) change
    //      of the commandnames.
    //      We may like to remove the first 3 ones if older
    //      cores (< 2.5.23/2.6pre5, tag release-2-5-23) arn't
    //      supported any longer.
    KMLDonkey::App->donkey->sendConsoleMessage("ovmd4", callback);
    KMLDonkey::App->donkey->sendConsoleMessage("ovstats", callback);
    KMLDonkey::App->donkey->sendConsoleMessage("ovstore", callback);

    KMLDonkey::App->donkey->sendConsoleMessage("ov_md4", callback);
    KMLDonkey::App->donkey->sendConsoleMessage("ov_stats", callback);
    KMLDonkey::App->donkey->sendConsoleMessage("ov_store", callback);
}

void StatsPageOvernet::showEvent(QShowEvent* /*event*/)
{
    slotUpdate();
}

void StatsPageOvernet::hideEvent(QHideEvent* /*event*/)
{
    clearItems();
}
