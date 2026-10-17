// SPDX-License-Identifier: GPL-2.0-or-later
#include "addserver.h"
#include "clipboard.h"
#include <QClipboard>
#include <QTextDocument>
#include <QLabel>
#include "donkeyhost.h"
#include "donkeymessage.h"
#include "donkeyprotocol.h"
#include "ed2kurl.h"
#include "fileinfo.h"
#include "hostdialog.h"
#include "hostmanager.h"
#include "infodialog.h"
#include "kmldonkey.h"
#include "mlconfig.h"
#include "prefs/prefs.h"
#include "renamedialog.h"
#include "searchcore.h"
#include "searchquery.h"
#include "shareddirs.h"
#include "submitdialog.h"
#include "webbrowser.h"
#include <KAboutData>
#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KLineEdit>
#include <KLocalizedString>
#include <KPageWidget>
#include <KPageWidgetModel>
#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QtTest>
#include <algorithm>

static QStringList connectionErrors;
static QtMessageHandler previousHandler;
static void recordMessages(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    if (message.contains("No such signal") || message.contains("No such slot")
        || message.contains("incompatible sender/receiver") || message.contains("already has a layout"))
        connectionErrors.append(message);
    if (previousHandler)
        previousHandler(type, context, message);
}
static QByteArray frame(const DonkeyMessage& message)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << qint32(message.size()) << qint16(message.opcode());
    result.append(message.data());
    return result;
}
class BrowserProbe : public WebBrowser {
public:
    BrowserProbe()
        : WebBrowser(nullptr)
    {
    }
    QString selected;
    void handleED2K(const ED2KURL& url) override { selected = url.toString(); }
};

class PortingTest : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() { previousHandler = qInstallMessageHandler(recordMessages); }
    void cleanupTestCase() { qInstallMessageHandler(previousHandler); }
    void clipboardHtmlPreservesFilename()
    {
        const QString filename = "<b>archive</b> & \"copy\".iso";
        QStringList values{filename, "0123456789abcdef0123456789abcdef", "42"};
        ClipboardHelper clipboard;
        clipboard.copyToClipboard(values, ClipboardHelper::HTML);
        QTextDocument document;
        document.setHtml(QApplication::clipboard()->text());
        QCOMPARE(document.toPlainText(), filename);
        QVERIFY(QApplication::clipboard()->text().contains("%3Cb%3E"));
    }
    void ed2kLinks()
    {
        const QString raw = "ed2k://|file|example|5368709120|0123456789abcdef0123456789abcdef|/";
        ED2KURL link(raw);
        QVERIFY(!link.isInvalid());
        QCOMPARE(link.fileSize(), quint64(5368709120));
        QCOMPARE(link.toString().toLower(), raw);
        ED2KURL opaque(QUrl("ed2k:|file|example|5368709120|0123456789abcdef0123456789abcdef|/"));
        QVERIFY(link.isSameFile(opaque));
    }
    void searchQueryFieldOrder()
    {
        DonkeyMessage message(0);
        QueryKeywords original("Keywords", "distribution");
        original.writeQuery(message);
        message.resetPosition();
        QScopedPointer<SearchQuery> decoded(SearchQuery::getQuery(&message));
        auto* keywords = dynamic_cast<QueryKeywords*>(decoded.data());
        QVERIFY(keywords);
        QCOMPARE(keywords->key(), QString("Keywords"));
        QCOMPARE(keywords->value(), QString("distribution"));
    }
    void webNavigation()
    {
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        connect(&server, &QTcpServer::newConnection, &server, [&server] {
            auto* peer = server.nextPendingConnection();
            connect(peer, &QTcpSocket::readyRead, peer, [peer] {
                QByteArray request = peer->property("request").toByteArray() + peer->readAll();
                peer->setProperty("request", request);
                if (!request.contains("\r\n\r\n"))
                    return;
                QByteArray body = request.startsWith("GET /second ") ? "<p>second page</p>" : "<p>first page</p>";
                peer->write("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: "
                    + QByteArray::number(body.size()) + "\r\nConnection: close\r\n\r\n" + body);
                peer->disconnectFromHost();
            });
            connect(peer, &QTcpSocket::disconnected, peer, &QObject::deleteLater);
        });
        BrowserProbe browser;
        auto* document = browser.findChild<QTextBrowser*>();
        QVERIFY(document);
        const QString base = QString("http://127.0.0.1:%1").arg(server.serverPort());
        browser.openURL(QUrl(base + "/first"));
        QTRY_COMPARE(document->toPlainText(), QString("first page"));
        browser.openURL(QUrl(base + "/second"));
        QTRY_COMPARE(document->toPlainText(), QString("second page"));
        auto* back = browser.findChild<QPushButton*>("webBack");
        QVERIFY(back && back->isEnabled());
        QTest::mouseClick(back, Qt::LeftButton);
        QCOMPARE(document->toPlainText(), QString("first page"));
        const QUrl link("ed2k:|file|example|42|0123456789abcdef0123456789abcdef|/");
        QVERIFY(QMetaObject::invokeMethod(document, "anchorClicked", Q_ARG(QUrl, link)));
        QVERIFY(browser.selected.startsWith("ed2k://|file|example|42|"));
    }
    void desktopAndProtocol()
    {
        QTcpServer core;
        QVERIFY(core.listen(QHostAddress::LocalHost));
        KConfig config("mldonkeyrc");
        auto group = config.group("Test <b>core</b>");
        group.writeEntry("DonkeyHost", "127.0.0.1");
        group.writeEntry("DonkeyGuiPort", int(core.serverPort()));
        group.writeEntry("Default", true);
        group.writeEntry("DonkeyUsername", "user<&>");
        config.sync();
        KConfig notifications("kmldonkey.notifyrc", KConfig::NoGlobals);
        for (const auto& event :
            { "KMLDonkeyConnected", "KMLDonkeyDisconnected", "KMLDonkeyDownloadFinished", "KMLDonkeyDownloadAdded" })
            notifications.group(QString("Event/") + event).writeEntry("Action", "None");
        notifications.sync();
        KMLDonkey window;
        window.show();
        QVERIFY(window.factory());
        auto* pages = window.findChild<KPageWidget*>("pages");
        QVERIFY(pages);
        QCOMPARE(pages->model()->rowCount(), 7);
        for (int i = 0; i < 7; ++i) {
            window.activatePage(i);
            QCoreApplication::processEvents();
            QCOMPARE(static_cast<KPageWidgetModel*>(pages->model())->index(pages->currentPage()).row(), i);
        }
        QTRY_VERIFY(core.hasPendingConnections());
        QScopedPointer<QTcpSocket> peer(core.nextPendingConnection());
        QSignalSpy connected(window.donkey, &DonkeyProtocol::signalConnected);
        DonkeyMessage hello(DonkeyProtocol::CoreProtocol);
        hello.writeInt32(33);
        auto packet = frame(hello);
        peer->write(packet.left(3));
        peer->flush();
        QTest::qWait(20);
        QCOMPARE(connected.count(), 0);
        peer->write(packet.mid(3));
        peer->flush();
        QTRY_COMPARE(connected.count(), 1);
        auto *status = window.findChild<QLabel *>("connectionStatus");
        QVERIFY(status);
        QCOMPARE(status->textFormat(), Qt::PlainText);
        QVERIFY(status->text().contains("Test <b>core</b>"));
        QTextDocument tooltip;
        tooltip.setHtml(status->toolTip());
        QVERIFY(tooltip.toPlainText().contains("Test <b>core</b>"));
        QVERIFY(tooltip.toPlainText().contains("user<&>"));
        QTRY_VERIFY(peer->bytesAvailable() > 0);
        DonkeyMessage console(DonkeyProtocol::Console);
        console.writeString("Qt6 protocol smoke test");
        peer->write(frame(console));
        peer->flush();
        auto browsers = window.findChildren<QTextBrowser*>();
        QVERIFY(!browsers.isEmpty());
        QTRY_VERIFY(std::any_of(browsers.cbegin(), browsers.cend(),
            [](QTextBrowser* browser) { return browser->toPlainText().contains("Qt6 protocol smoke test"); }));
        DonkeyMessage network(DonkeyProtocol::Network_info);
        network.writeInt32(1);
        network.writeString("BitTorrent");
        network.writeInt8(1);
        network.writeString("");
        network.writeInt64(0);
        network.writeInt64(0);
        network.writeInt32(0);
        network.writeInt16(0);
        peer->write(frame(network));
        peer->flush();
        QTRY_VERIFY(window.donkey->findNetworkNo(1));
        DonkeyMessage file(DonkeyProtocol::File_info);
        file.writeInt32(42);
        file.writeInt32(1);
        file.writeInt16(1);
        file.writeString("example.iso");
        for (int i = 0; i < 16; ++i)
            file.writeInt8(0);
        file.writeInt64(5368709120);
        file.writeInt64(1024);
        file.writeInt32(1);
        file.writeInt32(1);
        const int statePosition = file.data().size();
        file.writeInt8(FileInfo::Downloading);
        file.writeByteArray("1");
        file.writeInt16(0);
        file.writeFloat(1024.0);
        file.writeInt16(0);
        file.writeInt32(0);
        file.writeInt8(0);
        file.writeString("example.iso");
        file.writeInt32(0);
        file.writeInt32(0);
        file.writeString("");
        file.writeInt16(1);
        file.writeString("urn:ed2k:00000000000000000000000000000000");
        peer->write(frame(file));
        peer->flush();
        QTRY_VERIFY(window.donkey->findDownloadFileNo(42));
        QCOMPARE(window.donkey->findDownloadFileNo(42)->fileSize(), quint64(5368709120));
        auto* downloads = window.findChild<QTreeWidget*>("downloadView");
        QVERIFY(downloads);
        QTRY_COMPARE(downloads->topLevelItemCount(), 1);
        QCOMPARE(downloads->topLevelItem(0)->text(0), QString("example.iso"));
        auto* info = new FileInfoDialog(42);
        info->show();
        QCoreApplication::processEvents();
        info->reject();
        RenameDialog rename(window.donkey, 42);
        rename.show();
        QSignalSpy preview(&rename, &PortedDialog::tryClicked);
        QTest::mouseClick(rename.button(PortedDialog::Try), Qt::LeftButton);
        QCOMPARE(preview.count(), 1);
        QVERIFY(rename.isVisible());
        rename.reject();
        QByteArray completedData = file.data();
        completedData[statePosition] = char(FileInfo::Complete);
        peer->write(frame(DonkeyMessage(DonkeyProtocol::File_info, completedData)));
        peer->flush();
        auto* completed = window.findChild<QTreeWidget*>("downloadedView");
        QVERIFY(completed);
        QTRY_COMPARE(downloads->topLevelItemCount(), 0);
        QTRY_COMPARE(completed->topLevelItemCount(), 1);
        QCOMPARE(completed->topLevelItem(0)->text(0), QString("example.iso"));
        // Enter in the console must still send the command to the core.
        peer->readAll();
        auto* consolePage = window.findChild<QWidget*>("consolePage");
        auto* consoleEntry = consolePage->findChild<KLineEdit*>();
        QVERIFY(consoleEntry);
        consoleEntry->setText("porting-command <b>&</b>");
        QTest::keyClick(consoleEntry, Qt::Key_Return);
        QTRY_VERIFY(peer->bytesAvailable() > 0);
        QByteArray consoleCommands;
        QTRY_VERIFY((consoleCommands += peer->readAll()).contains("porting-command <b>&</b>"));
        QVERIFY(std::any_of(browsers.cbegin(), browsers.cend(), [](QTextBrowser *browser) {
            return browser->toPlainText().contains("porting-command <b>&</b>");
        }));
        DonkeyMessage forms(DonkeyProtocol::DefineSearches);
        forms.writeInt16(1);
        forms.writeString("Simple search");
        QueryAnd form;
        form.append(new QueryKeywords("Keywords", ""));
        form.writeQuery(forms);
        peer->write(frame(forms));
        peer->flush();
        auto* searchCore = window.findChild<SearchCore*>("searchCore");
        QVERIFY(searchCore);
        QTRY_VERIFY(!searchCore->findChildren<KLineEdit*>().isEmpty());
        auto* keywords = searchCore->findChild<KLineEdit*>();
        keywords->setText("distribution");
        window.activatePage(1);
        QVERIFY(QMetaObject::invokeMethod(searchCore, "startSearch"));
        auto* tabs = window.findChild<QWidget*>("searchPage")->findChild<QTabWidget*>();
        QVERIFY(tabs);
        QCOMPARE(tabs->count(), 1);
        DonkeyMessage result(DonkeyProtocol::Result_info);
        result.writeInt32(9);
        result.writeInt32(1);
        result.writeInt16(1);
        result.writeString("distribution.iso");
        result.writeInt16(1);
        result.writeString("urn:ed2k:00000000000000000000000000000000");
        result.writeInt64(1048576);
        result.writeString("iso");
        result.writeString("");
        result.writeInt16(0);
        result.writeString("");
        result.writeInt8(0);
        result.writeInt32(0);
        DonkeyMessage mapping(DonkeyProtocol::Search_result);
        mapping.writeInt32(1);
        mapping.writeInt32(9);
        peer->write(frame(result) + frame(mapping));
        peer->flush();
        auto* results = tabs->widget(0)->findChild<QTreeWidget*>();
        QVERIFY(results);
        QTRY_COMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), QString("distribution.iso"));
        QVERIFY(QMetaObject::invokeMethod(tabs, "tabCloseRequested", Q_ARG(int, 0)));
        QCOMPARE(tabs->count(), 0);
        // Exercise the widgets that were removed from KDE4.
        KMLDonkeyPreferences preferences(&window);
        preferences.show();
        QCoreApplication::processEvents();
        QVERIFY(preferences.findChildren<QSpinBox*>().size() > 0);
        QVERIFY(preferences.findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Apply));
        preferences.reject();
        HostDialog hosts(&window, window.hostManager, window.donkey);
        QVERIFY(hosts.hostList);
        SubmitDialog* submit = new SubmitDialog(&window);
        submit->show();
        submit->setText("ed2k://|file|example|42|0123456789abcdef0123456789abcdef|/");
        QVERIFY(submit->button(PortedDialog::Ok)->isEnabled());
        submit->reject();
        auto* shares = new SharedDirsDialog(&window);
        shares->show();
        QCoreApplication::processEvents();
        shares->reject();
        auto* addServer = new AddServerDialog(&window);
        addServer->show();
        QCoreApplication::processEvents();
        addServer->reject();
        MLDonkeyConfigDialog coreOptions(&window);
        coreOptions.show();
        QCoreApplication::processEvents();
        coreOptions.reject();
        // Remote disconnect must clear the GUI without stale signal connections.
        peer->disconnectFromHost();
        QTRY_VERIFY(!window.donkey->isConnected());
        QVERIFY(status->text().startsWith("Disconnected."));
        QVERIFY(!status->text().contains("<b>"));
        QVERIFY2(connectionErrors.isEmpty(), qPrintable(connectionErrors.join('\n')));
        window.activatePage(1);
        window.resize(1100, 700);
        QCoreApplication::processEvents();
        window.finalise();
    }
};
int main(int argc, char** argv)
{
    QTemporaryDir settings;
    qputenv("XDG_CONFIG_HOME", (settings.path() + "/config").toUtf8());
    qputenv("XDG_DATA_HOME", (settings.path() + "/data").toUtf8());
    qputenv("XDG_CACHE_HOME", (settings.path() + "/cache").toUtf8());
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kmldonkey");
    KAboutData::setApplicationData(KAboutData("kmldonkey", "KMLDonkey", "2.1.5"));
    PortingTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "portingtest.moc"
