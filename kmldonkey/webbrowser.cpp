/* -*- c++ -*-
 *
 * webbrowser.cpp
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

#include "webbrowser.h"
#include "kmldonkey.h"
#include <KLocalizedString>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressBar>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <donkeyprotocol.h>

WebBrowser::WebBrowser(QWidget* parent)
    : QWidget(parent)
    , browser(new QTextBrowser(this))
    , network(new QNetworkAccessManager(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(browser);
    browser->setOpenLinks(false);
    connect(browser, &QTextBrowser::anchorClicked, this, qOverload<const QUrl&>(&WebBrowser::openURL));

    auto* navigation = new QHBoxLayout;
    layout->addLayout(navigation);
    backButton = new QPushButton(i18n("Back"), this);
    backButton->setObjectName("webBack");
    forwardButton = new QPushButton(i18n("Forward"), this);
    forwardButton->setObjectName("webForward");
    homeButton = new QPushButton(i18n("Home"), this);
    for (auto* button : { backButton, forwardButton, homeButton }) {
        navigation->addWidget(button);
        button->setEnabled(false);
    }
    connect(backButton, &QPushButton::clicked, this, [this] { showPage(position - 1); });
    connect(forwardButton, &QPushButton::clicked, this, [this] { showPage(position + 1); });
    connect(homeButton, &QPushButton::clicked, this, [this] { showPage(0); });
    status = new QLabel(this);
    status->setTextFormat(Qt::PlainText);
    status->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    navigation->addWidget(status, 1);
    progress = new QProgressBar(this);
    progress->setRange(0, 0);
    progress->hide();
    navigation->addWidget(progress);
    auto* external = new QPushButton(i18n("Open in Browser"), this);
    navigation->addWidget(external);
    connect(
        external, &QPushButton::clicked, this, [this] { QDesktopServices::openUrl(browser->document()->baseUrl()); });
}

WebBrowser::~WebBrowser() = default;

void WebBrowser::showPage(int index)
{
    if (index < 0 || index >= history.size())
        return;
    ++requestId; // Ignore a pending response after navigating through cached pages.
    progress->hide();
    position = index;
    const auto& page = history.at(index);
    browser->document()->setBaseUrl(page.url);
    browser->setHtml(page.html);
    status->setText(page.url.toDisplayString());
    backButton->setEnabled(index > 0);
    forwardButton->setEnabled(index + 1 < history.size());
    homeButton->setEnabled(index > 0);
}

void WebBrowser::openURL(const QString& url)
{
    if (url.startsWith("ed2k:")) {
        handleED2K(ED2KURL(url));
        return;
    }
    openURL(QUrl(url));
}

void WebBrowser::openURL(const QUrl& target)
{
    const QUrl url = browser->document()->baseUrl().resolved(target);
    if (url.scheme() == "ed2k") {
        handleED2K(ED2KURL(url));
        return;
    }
    if (url.scheme() != "http" && url.scheme() != "https")
        return;
    progress->show();
    status->setText(url.toDisplayString());
    const quint64 currentRequest = ++requestId;
    auto* reply = network->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, currentRequest] {
        if (currentRequest == requestId) {
            QString html = reply->error() == QNetworkReply::NoError ? QString::fromUtf8(reply->readAll())
                                                                    : reply->errorString().toHtmlEscaped();
            // ED2K uses pipe characters in the authority, which QUrl rejects.
            // Its opaque form preserves the complete link for the click handler.
            html.replace("ed2k://|", "ed2k:|");
            while (history.size() > position + 1)
                history.removeLast();
            history.append({ reply->url(), html });
            showPage(history.size() - 1);
        }
        reply->deleteLater();
    });
}

void WebBrowser::handleED2K(const ED2KURL& url)
{
    if (!url.isInvalid())
        KMLDonkey::App->donkey->submitUrl(url.toString());
}
