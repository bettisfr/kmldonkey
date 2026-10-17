/* -*- c++ -*-
 *
 * main.cpp
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

#include "kmldonkey.h"
#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>
#include <QApplication>
#include <QPointer>
#include <QCommandLineParser>
#include <KIconTheme>
#include <KStyleManager>
int main(int argc, char** argv)
{
    KIconTheme::initTheme();
    QApplication app(argc, argv);
    KStyleManager::initStyle();
    KLocalizedString::setApplicationDomain("kmldonkey");
    KAboutData about("kmldonkey", i18n("KMLDonkey"), "2.1.5", i18n("An MLDonkey frontend for the KDE desktop."),
        KAboutLicense::GPL_V2, i18n("Copyright © 2003–2018 The KMLDonkey Authors"));
    about.setHomepage(QStringLiteral("https://github.com/bettisfr/kmldonkey"));
    about.addAuthor("Francesco Betti Sorbelli", i18n("Qt6 / KDE Frameworks 6 port"),
        QStringLiteral("francesco.bettisorbelli@gmail.com"));
    about.addAuthor("Sebastian Sauer", i18n("Maintainer & Developer"), "mail@dipe.org");
    about.addAuthor("Gioacchino Mazzurco", i18n("Developer"), "gmazzurco89@gmail.com");
    about.addAuthor("Christian Muehlhaeuser", i18n("Developer"), "chris@chris.de");
    about.addAuthor("Eduard Huguet", i18n("Developer"), "eduardhc@gmail.com");
    about.addAuthor("Petter Stokke", i18n("Developer & Project Founder"), "ummo@hellokitty.com");
    KAboutData::setApplicationData(about);
    QGuiApplication::setDesktopFileName(QStringLiteral("kmldonkey"));
    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
    KDBusService service(KDBusService::Unique);
    // KMainWindow can delete itself on close, including deferred deletion.
    // Keep its lifetime independent of main's stack frame.
    QPointer<KMLDonkey> window = new KMLDonkey;
    QObject::connect(&service, &KDBusService::activateRequested, window.data(), [window] {
        if (!window) return;
        window->show();
        window->raise();
        window->activateWindow();
    });
    window->show();
    const int result = app.exec();
    delete window.data();
    return result;
}
