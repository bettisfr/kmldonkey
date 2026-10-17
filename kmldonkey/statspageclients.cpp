/* -*- c++ -*-
 *
 * statspageclients.cpp
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
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPainter>
#include <QShowEvent>
#include <QPaintEvent>
#include <QList>
#include <QPixmap>
#include <QHideEvent>
#include <QKeyEvent>

#include <QDebug>
#include <QFontDatabase>
#include <QTextBrowser>
#include "autobox.h"

#include "kmldonkey.h"
#include "donkeyprotocol.h"
#include "prefs/prefs.h"

#include "statspageclients.h"

ClientsPieWidget::ClientsPieWidget(QWidget *parent) : QFrame(parent)
{
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //setBackgroundColor(Qt::white /*parent->paletteBackgroundColor()*/);
}

ClientsPieWidget::~ClientsPieWidget()
{
    qDeleteAll(values);
}

void ClientsPieWidget::addValue(const QString& label, const double value, QColor color)
{
    ClientsPieWidgetObject* obj = new ClientsPieWidgetObject();
    obj->label = label;
    obj->value = value;
    obj->color = color;
    values.append(obj);
}

void ClientsPieWidget::paintEvent(QPaintEvent*)
{
    QPixmap pixmap(width(), height());
    QPainter painterTmp;
    painterTmp.begin(&pixmap);
	painterTmp.setFont(font());

    painterTmp.setClipRect(2, 2, width() - 4, height() - 4);
    pixmap.fill( palette().window().color() );

    if (values.count() > 1) { // at least 2 values should be there to display a pie
        QFontMetrics fontmet( QFontDatabase::systemFont(QFontDatabase::GeneralFont) );

        int startpos = -1440; //-90*16
        int alen;
        int labelpos = fontmet.height() + 2;

        QList<ClientsPieWidgetObject*>::iterator it;
        for (it = values.begin(); it != values.end(); ++it) {
            alen = (int)( (*it)->value * 57.6 );

            painterTmp.setBrush( (*it)->color );
            painterTmp.setPen( (*it)->color );

            painterTmp.drawPie(2 + 130, 2, width() - 4 - 130, height() - 4, startpos, alen);
            startpos += alen;

            painterTmp.drawText(2, labelpos, (*it)->label);
            labelpos += fontmet.height() + 2;
        }
    }

    painterTmp.end();
	
	QPainter painter(this);
	painter.drawPixmap( QPoint(0, 0), pixmap );
}

StatsPageClientsCallback::StatsPageClientsCallback()
{
    colorsList.append( Qt::blue );
    colorsList.append( Qt::darkBlue );
    colorsList.append( Qt::darkMagenta );
    colorsList.append( Qt::darkGreen );
    colorsList.append( Qt::red );
    colorsList.append( Qt::darkCyan );
    colorsList.append( Qt::darkRed );
    colorsList.append( Qt::darkYellow );
    //colorsList.append( Qt::green );

    colorsListIterator = colorsList.begin();
}

StatsPageClientsCallback::~StatsPageClientsCallback()
{
}

void StatsPageClientsCallback::callback(const QString& /*cmd*/, const QString& res)
{
    if (! page) return;
    delete page->box;
    page->callback = 0;
    page->box = new VBoxWidget();

    QStringList groups = res.split("\n\n");
    QStringList list;
    int pos, count;
    QString label, value;
    QGroupBox * grpBox;
	QVBoxLayout * grpBoxLayout;
    HBoxWidget *hbox;
    VBoxWidget *vbox;
    ClientsPieWidget *pie = 0;

    for (QStringList::Iterator groupit = groups.begin(); groupit != groups.end(); ++groupit) {
        list = (*groupit).split("\n", Qt::SkipEmptyParts );
        
		grpBox = new QGroupBox(page->box);
		grpBoxLayout = new QVBoxLayout;
		grpBox->setLayout( grpBoxLayout );
        
		hbox = new HBoxWidget();
		grpBoxLayout->addWidget( hbox );
		
        vbox = new VBoxWidget(hbox);
        vbox->setSpacing(6);
        count = 0;

        for (QStringList::Iterator listit = list.begin(); listit != list.end(); ++listit) {
            pos = (*listit).indexOf(":");
            if (pos > 0) {
                label = (*listit).left(pos + 1).trimmed();
                value = (*listit).right((*listit).length() - pos - 1).trimmed();
                if (label.isEmpty())
                    continue;

                QString formattedValue = value.toHtmlEscaped();
                if (count == 0) {
                    grpBox->setTitle( label.left(label.length() - 1).trimmed() );
                }
                else {
                    int p1 = value.indexOf("(");
                    int p2 = value.indexOf("%", p1);
                    int p3 = value.indexOf(")", p2);
                    if (p3 > p2 && p2 > p1 && p1 > 0) {
                        bool ok;
                        double pv = value.mid(p1 + 1, p2 - p1 -1).trimmed().toDouble(&ok);
                        if (ok && pv >= 0.5) {
                            if (! pie)
                                pie = new ClientsPieWidget(hbox);
                            if (! colorsMap.contains(label)) {
                                if (colorsListIterator == colorsList.end())
                                    colorsListIterator = colorsList.begin();
                                colorsMap[label] = (*colorsListIterator);
                                colorsListIterator++;
                            }
                            pie->addValue(label.endsWith(":") ? label.left(label.length() - 1) : label, pv, colorsMap[label]);
                            formattedValue = value.left(p1+1).toHtmlEscaped() + "<b>" + value.mid(p1+1, p3-p1-1).toHtmlEscaped() + "</b>" + value.right(value.length()-p3).toHtmlEscaped();
                        }
                    }
                }

                HBoxWidget *hb = new HBoxWidget(vbox);
                auto *labelWidget = new QLabel(label, hb);
                labelWidget->setTextFormat(Qt::PlainText);
                new QLabel("<nobr>" + formattedValue + "</nobr>", hb);
                count++;
            }
        }
        if (pie) {
            pie->show();
            pie = 0;
        }
        grpBox->show();
    }

    page->setWidget(page->box);
}

StatsPageClients::StatsPageClients(QWidget* parent) : QScrollArea(parent)
{
    setWidgetResizable(true);
    box = 0;
    callback = 0;
}

#if 0
void StatsPageClients::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
        case Qt::Key_Up:
            scrollBy(0, -10);
            break;
        case Qt::Key_Down:
            scrollBy(0, 10);
            break;
        case Qt::Key_Left:
            scrollBy(-10, 0);
            break;
        case Qt::Key_Right:
            scrollBy(10, 0);
            break;
        case Qt::Key_Home:
            setContentsPos(0, 0);
            break;
        case Qt::Key_End:
            setContentsPos(0, contentsHeight() - visibleHeight());
            break;
    }
}
#endif

void StatsPageClients::slotUpdate()
{
    if (! KMLDonkey::App->donkey->isConnected()) return;
    if (! callback) {
        callback = new StatsPageClientsCallback;
        callback->page = this;
    }

    KMLDonkey::App->donkey->sendConsoleMessage("client_stats", callback);
}

void StatsPageClients::showEvent(QShowEvent* event)
{
    QScrollArea::showEvent(event);
    slotUpdate();
    setFocus();
}

void StatsPageClients::hideEvent(QHideEvent* event)
{
    QScrollArea::hideEvent(event);
    delete box;
    box = 0;
}

