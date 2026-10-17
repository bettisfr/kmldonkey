/* -*- c++ -*-
 *
 * webbrowser.h
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2007 Sebastian Sauer <mail@dipe.org>
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

#pragma once
#include <QUrl>
#include <QWidget>
#include <ed2kurl.h>
class QTextBrowser;
class QNetworkAccessManager;
class QPushButton;
class QProgressBar;
class QLabel;
class WebBrowser : public QWidget {
    Q_OBJECT
public:
    explicit WebBrowser(QWidget* parent);
    ~WebBrowser() override;
    virtual void handleED2K(const ED2KURL& url);
public slots:
    void openURL(const QString& url);
    void openURL(const QUrl& url);

private:
    QTextBrowser* browser;
    QNetworkAccessManager* network;
    struct Page {
        QUrl url;
        QString html;
    };
    QList<Page> history;
    int position = -1;
    quint64 requestId = 0;
    QPushButton *backButton, *forwardButton, *homeButton;
    QProgressBar* progress;
    QLabel* status;
    void showPage(int index);
};
