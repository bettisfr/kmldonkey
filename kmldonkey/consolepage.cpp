#include <QRegularExpression>
/* -*- c++ -*-
 *
 * consolepage.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006 Christian Muehlhaeuser <chris@chris.de>
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

#include <QRegExp>

#include <KLocalizedString>
#include <QTextBrowser>
#include <klineedit.h>
#include <QAction>
#include <kactioncollection.h>
#include <QDebug>

#include <donkeyprotocol.h>

#include "kmldonkey.h"
#include "consolepage.h"

QString disclaimerText()
{
    return i18n( "<h2>WARNING</h2>\n"
                 "<p>Peer-to-peer networks, particularly those accessible via KMLDonkey, "
                 "may contain material which you may be legally prohibited from downloading or "
                 "redistributing. It is your responsibility to verify that any material you "
                 "download from any peer-to-peer network is being made available to you lawfully. "
                 "It is also your responsibility to verify that you have the legal right to "
                 "redistribute any material which you make available to others via any "
                 "peer-to-peer networks. In short, it is your responsibility to use this software "
                 "in accordance with applicable laws where you live, including copyright law.</p>\n"
                 "<h3>RESTRICTIONS</h3>\n"
                 "<p>The authors of this software intend it to be used for the "
                 "lawful exchange of data, art, software and other documents or works which "
                 "may be lawfully distributed to third parties. The authors will not "
                 "support, and strongly discourage, any use of this software to violate "
                 "copyright or other intellectual property laws. In particular, the authors "
                 "discourage any transmission of any material in violation of any applicable "
                 "intellectual property law or regulation. Each user is assumed to comply "
                 "with such rules and restrictions as well as the rules and restrictions "
                 "applicable to the P2P networks or services connected directly or "
                 "indirectly to this software.</p>\n"
                 "<h3>DISCLAIMER OF LIABILITY</h3>\n"
                 "<p>The authors of this software do not and cannot "
                 "exercise any control whatsoever over the content of the information "
                 "exchanged using this software. The authors make no warranties of any "
                 "kind, whether expressed or implied, for the service this software is "
                 "providing or for the data exchanged with the assistance of this software. "
                 "The authors cannot be held responsible for any claims resulting from the "
                 "user's conduct and/or use of the software which is in any manner unlawful "
                 "or which damages such user or any other party.</p>" );
}

ConsolePage::ConsolePage(QWidget* parent)
    : VBoxWidget(parent)
    , KMLDonkeyPage()
{
    setObjectName("consolePage");
    alreadyClear = true;

    consoleView = new QTextBrowser(this);
    //consoleView->setFocusPolicy(QTextBrowser::ClickFocus);
    //consoleView->setTextFormat(QTextBrowser::RichText);
    //consoleView->setLinkUnderline(true);

    consoleView->append(i18n("<p><b>KMLDonkey</b>, Copyright 2003-2009 Petter Stokke, Sebastian Sauer, Gioacchino Mazzurco and Christian Muehlhaeuser</p>"));
    consoleView->append(i18n("<p>This program is free software; you can redistribute it and/or modify "
			     "it under the terms of the GNU General Public License as published by "
			     "the Free Software Foundation; either version 2 of the License, or "
			     "(at your option) any later version.</p>"
			     "<p>This program is distributed in the hope that it will be useful, "
			     "but WITHOUT ANY WARRANTY; without even the implied warranty of "
			     "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
			     "GNU General Public License for more details.</p>"
			     "<p>You should have received a copy of the GNU General Public License "
			     "along with this program; if not, write to the Free Software "
			     "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA</p>"));
    consoleView->append("<hr>");
    consoleView->append(disclaimerText());
    consoleView->append("<hr>");

    consoleEntry = new KLineEdit(this);
    connect(consoleEntry, &QLineEdit::returnPressed, this, [this]{
        const QString text=consoleEntry->text();
        consoleEntry->completionObject()->addItem(text);
        sendConsoleMessage(text);
    });

    connect(KMLDonkey::App->donkey, SIGNAL(consoleMessage(const QString&)), this, SLOT(consoleMessage(const QString&)));
}

void ConsolePage::clear()
{
    if (!alreadyClear) {
        consoleView->append("<hr>");
        alreadyClear = true;
    }
}

void ConsolePage::applyPreferences(KMLDonkeyPreferences*)
{
    consoleView->setFont(KMLDonkey::App->consoleFont);
}

void ConsolePage::setupActions(KActionCollection* actionCollection)
{
    QAction* action = new QAction(QIcon::fromTheme("utilities-terminal"), i18n("Activate Console Page"), this);
    action->setIconText(i18n("Console"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(actionActivatePage()));
    actionCollection->addAction("activate_page_console", action);
}

void ConsolePage::consoleMessage(const QString& msg)
{
    alreadyClear = false;

    QString txt = msg.toHtmlEscaped();

    if (txt.startsWith("\n+--"))
        txt = "<b>" + txt + "</b>";
    else if (txt.startsWith("CORRUPTION DETECTED"))
        txt = "<p><b>" + txt + "</b></p>";
    else {
        QRegExp rxurl("[^\">](((https|http|ftp|news)://|(?!://)www\\.)([\\w\\.\\-\\_/~?=&%;:@+$,!*'\\(\\)\\|\\^\\[\\]\\`\\{\\}]+))");
        rxurl.setCaseSensitivity(Qt::CaseInsensitive);
        int pos = 0;
        QString url, replaceurl;
        while ((pos = rxurl.indexIn(txt, pos)) >= 0) {
            QString s = ( rxurl.cap(2).startsWith("www") ) ? QString("http://") : QString();
            url = s + rxurl.cap(2) + rxurl.cap(4);
            replaceurl = "<a href=\"" + url + "\">" + rxurl.cap(1) + "</a>";
            txt.replace(pos + 1, rxurl.cap(1).length(), replaceurl);
            pos += replaceurl.length();
        }
    }

    txt.replace(QRegularExpression("\n(?!$)"), "<br>");

    if(txtBefore != txt) {
        consoleView->append(txt);
        txtBefore = txt;
    }
}

void ConsolePage::sendConsoleMessage(const QString& txt)
{
    alreadyClear = false;

    QString foo = "<p><b>" + txt.toHtmlEscaped() + "</b></p>";
    consoleView->append(foo);
    consoleEntry->clear();
    foo = txt;
    KMLDonkey::App->donkey->sendConsoleMessage(foo);
}

void ConsolePage::actionActivatePage()
{
    KMLDonkey::App->activatePage(this);
}

