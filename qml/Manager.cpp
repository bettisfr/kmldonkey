#include "Manager.h"
#include "HostModel.h"

#include "hostiface.h"
#include "donkeyhost.h"
#include "fileinfo.h"

//#define ENABLE_QNETWORKCONFIG

#ifdef ENABLE_QNETWORKCONFIG
#include <QNetworkConfigurationManager>
#endif
#include <QSettings>

Manager::Manager()
    : QObject()
{
    m_autoConnect = false;
    m_autoReconnect = false;
    m_autoOnline = true;
    m_savePassword = true;
    m_settingsModified = false;
    m_connectRequested = false;

    m_reconnectTimer.setInterval(5000); // re-try to connect every 5 seconds
    connect(&m_reconnectTimer, SIGNAL(timeout()), this, SLOT(slotReconnectTimeout()));

#ifdef ENABLE_QNETWORKCONFIG
    m_networkConfigManager = new QNetworkConfigurationManager(this);
    m_networkSession = 0;
#endif
    m_protocol = new DonkeyProtocol(false, this);

    m_hostModel = new HostModel(this);
    m_hostModel->setManager(this);

    QSettings settings("org.kde", "kmldonkey");
    qDebug() << "Reading settings from file=" << settings.fileName();

    settings.beginGroup("settings");
    QByteArray defaultHost = settings.value("host", QByteArray()).toByteArray();
    m_autoConnect = settings.value("autoconnect", m_autoConnect).toBool();
    m_autoReconnect = settings.value("autoreconnect", m_autoReconnect).toBool();
    m_autoOnline = settings.value("autoonline", m_autoOnline).toBool();
    m_savePassword = settings.value("savepassword", m_savePassword).toBool();
    settings.endGroup();

    settings.beginGroup("properties");
    Q_FOREACH(const QString &key, settings.childKeys())
        m_properties.insert(key, settings.value(key));
    settings.endGroup();

//    DonkeyHost *host = new DonkeyHost();
//    host->setName("Test");
//    host->setAddress("192.168.0.102");
//    host->setPort(4001);
//    host->setUsername("admin");
//    host->setPassword("");
//    m_hostModel->addHost(host);

    Q_FOREACH(const QString &group, settings.childGroups()) {
        if (!group.startsWith("connection_")) continue;
        settings.beginGroup(group);
        DonkeyHost *host = new DonkeyHost();
        host->setName(group.mid(11));
        host->setAddress(settings.value("address", "192.168.1.1").toString());
        host->setPort(settings.value("port", 4001).toInt());
        host->setUsername(settings.value("username", "admin").toString());
        host->setPassword(settings.value("userpass", "").toString());
        m_hostModel->addHost(host);
        settings.endGroup();
    }

    if (defaultHost.isEmpty() && m_hostModel->host("default"))
        defaultHost = "default";
    m_hostModel->setSelectedHost(defaultHost);
    m_hostModel->sort(0, Qt::AscendingOrder);

    connect(m_protocol, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(slotStateChanged(QAbstractSocket::SocketState)));
    connect(m_protocol, SIGNAL(signalConnected()), this, SLOT(slotConnectedWithCore()));
    connect(m_protocol, SIGNAL(signalDisconnected(int)), this, SLOT(slotDisconnectedFromCore(int)));
    connect(m_protocol, SIGNAL(clientStats(int64,int64,int64,int,int,int,int,int,int,int,QMap<int,int>*)), this, SLOT(slotClientStats(int64,int64,int64,int,int,int,int,int,int,int)));
    connect(m_protocol, SIGNAL(consoleMessage(QString)), this, SIGNAL(receivedConsoleMessage(QString)));

#ifdef ENABLE_QNETWORKCONFIG
    connect(m_networkConfigManager, SIGNAL(onlineStateChanged(bool)), this, SLOT(slotOnlineChanged()));
    connect(m_networkConfigManager, SIGNAL(updateCompleted()), this, SLOT(slotNetworkUpdated()));
#endif

    connect(m_hostModel, SIGNAL(selectedHostChanged()), this, SLOT(slotHostModified()));
    connect(m_hostModel, SIGNAL(hostAdded(QByteArray)), this, SLOT(slotHostModified()));
    connect(m_hostModel, SIGNAL(hostRemoved(QByteArray)), this, SLOT(slotHostModified()));
    connect(m_hostModel, SIGNAL(hostChanged(QByteArray)), this, SLOT(slotHostModified()));

    if (m_autoConnect && !m_protocol->isConnected()) {
        QTimer::singleShot(0, this, SLOT(connectToDefaultCore()));
    }
}

Manager::~Manager()
{
    if (m_settingsModified) {
        QSettings settings("org.kde", "kmldonkey");
        qDebug() << "Writing settings to file=" << settings.fileName();
        settings.clear();

        settings.beginGroup("settings");
        settings.setValue("host", m_hostModel->selectedHost());
        settings.setValue("autoconnect", autoConnect());
        settings.setValue("autoreconnect", autoReconnect());
        settings.setValue("savepassword", savePassword());
        settings.endGroup();

        settings.beginGroup("properties");
        Q_FOREACH(const QString &key, m_properties.keys())
            settings.setValue(key, m_properties[key]);
        settings.endGroup();

        Q_FOREACH(const QByteArray &name, m_hostModel->hosts()) {
            DonkeyHost *h = m_hostModel->host(name);
            settings.beginGroup(QString("connection_%1").arg(h->name()));
            settings.setValue("address", h->address());
            settings.setValue("port", h->port());
            settings.setValue("username", h->username());
            if (savePassword())
                settings.setValue("userpass", h->password());
            else
                settings.remove("userpass");
            settings.endGroup();
        }

        settings.sync();
    }
}

DonkeyProtocol* Manager::protocol()
{
    return m_protocol;
}

QObject* Manager::hostModel()
{
    return m_hostModel;
}

bool Manager::autoConnect() const
{
    return m_autoConnect;
}

void Manager::setAutoConnect(bool enable)
{
    if (enable == m_autoConnect)
        return;
    m_settingsModified = true;
    m_autoConnect = enable;
}

bool Manager::autoReconnect() const
{
    return m_autoReconnect;
}

void Manager::setAutoReconnect(bool enable)
{
    if (enable == m_autoReconnect)
        return;
    m_settingsModified = true;
    m_reconnectTimer.stop();
    m_autoReconnect = enable;
    if (m_autoReconnect && !m_connectRequested) {
        m_reconnectTimer.start();
    }
}

bool Manager::autoOnline() const
{
    return m_autoOnline;
}

void Manager::setAutoOnline(bool enable)
{
    if (enable == m_autoOnline)
        return;
    m_settingsModified = true;
    m_autoOnline = enable;
}

bool Manager::savePassword() const
{
    return m_savePassword;
}

void Manager::setSavePassword(bool save)
{
    if (save == m_savePassword)
        return;
    m_settingsModified = true;
    m_savePassword = save;
}

QVariant Manager::setting(const QString &name, const QVariant &defaultValue) const
{
    QVariantMap::ConstIterator it = m_properties.constFind(name);
    return it != m_properties.constEnd() ? it.value() : defaultValue;
}

void Manager::setSetting(const QString &name, const QVariant &value)
{
    if (value == m_properties.value(name))
        return;
    m_settingsModified = true;
    if (value.isNull())
        m_properties.remove(name);
    else
        m_properties[name] = value;
}

bool Manager::isConnected() const
{
    return m_protocol->isConnected();
}

void Manager::connectToCore(const QByteArray &identifier)
{
    m_reconnectTimer.stop();

    DonkeyHost *oldHost = dynamic_cast<DonkeyHost*>(m_protocol->getHost());
    DonkeyHost *newHost = identifier.isEmpty() ? oldHost : m_hostModel->host(identifier);
    if (!newHost) {
        qDebug() << "Manager::connectToCore No such host with identifier=" << identifier;
        return;
    }

    m_connectRequested = true;

    if (oldHost && oldHost != newHost) {
        disconnectFromCore();
    }

#ifdef ENABLE_QNETWORKCONFIG
    bool goOnline = m_autoOnline && !m_networkConfigManager->isOnline();
    if (goOnline && !m_networkSession) {
        QNetworkConfiguration networkConfig = m_networkConfigManager->defaultConfiguration();
        if (!networkConfig.isValid()) {
            qDebug() << "Manager::connectToCore: Default network configuration invalid. Abort connectiong.";
            disconnectFromCore();
            return;
        }

        // Note that we only create the QNetworkSession once and then later adapt if the configuration
        // changes. This is the prefered way and it seems to be a bad idea to delete and recreate the
        // QNetworkSession instance manually since that can have all kind of "funny" side-effects
        // including a stack-corruption on the N9 with Pr1.2 :-/
        m_networkSession = new QNetworkSession(networkConfig, m_networkConfigManager);
        connect(m_networkSession, SIGNAL(opened()), this, SLOT(slotSessionOpened()));
        connect(m_networkSession, SIGNAL(closed()), this, SLOT(slotSessionClosed()));
        connect(m_networkSession, SIGNAL(stateChanged(QNetworkSession::State)), this, SLOT(slotSessionStateChanged(QNetworkSession::State)));
        connect(m_networkSession, SIGNAL(error(QNetworkSession::SessionError)), this, SLOT(slotSessionError(QNetworkSession::SessionError)));
        connect(m_networkSession, SIGNAL(newConfigurationActivated()), this, SLOT(slotSessionNewActivated()));
        connect(m_networkSession, SIGNAL(preferredConfigurationChanged(QNetworkConfiguration,bool)), this, SLOT(slotSessionPreferredChanged(QNetworkConfiguration,bool)));
    }
#endif

    if (oldHost != newHost) {
        m_protocol->setHost(newHost);
        if (oldHost)
            m_hostModel->updateHost(oldHost);
        m_hostModel->updateHost(newHost);
    }

#ifdef ENABLE_QNETWORKCONFIG
    if (goOnline) {
        qDebug() << "Manager::connectToCore: Not online, trying to go online now - configIdentifier=" << m_networkSession->configuration().identifier() << "configName=" << m_networkSession->configuration().name() << "configState=" << m_networkSession->configuration().state() << "configType=" << m_networkSession->configuration().type() << "sessionState=" << m_networkSession->state();
        emit connectingToCore();

        //session.setSessionProperty("ConnectInBackground", true);
        //session.setSessionProperty("AutoCloseSessionTimeout", -1);
        m_networkSession->open();
    }
    else
#endif
    {
        qDebug() << "Manager::connectToCore: Connecting to core now...";
        m_protocol->connectToCore();
    }
}

void Manager::connectToDefaultCore()
{
    QByteArray defaultHost = m_hostModel->selectedHost();
    DonkeyHost *host = m_hostModel->host(defaultHost);
    if (!host) {
        qDebug() << "Manager::connectToDefaultCore No valid default host with identifier=" << defaultHost;
        return;
    }
    connectToCore(defaultHost);
}

void Manager::disconnectFromCore()
{
    qDebug() << "Manager::disconnectFromCore()";
    if (!m_protocol->disconnectFromCore())
        slotDisconnectedFromCore(DonkeyProtocol::NoError);
}

void Manager::submitUrl(const QString &url)
{
    qDebug() << "Manager::submitUrl url=" << url;
    m_protocol->submitUrl(url);
}

void Manager::sendConsoleMessage(const QString &message)
{
    qDebug() << "Manager::sendConsoleMessage message=" << message;
    m_protocol->sendConsoleMessage(message);
}

void Manager::slotOnlineChanged()
{
#ifdef ENABLE_QNETWORKCONFIG
    bool isOnline = m_networkConfigManager->isOnline();
    bool isConnected = m_protocol->isConnected();
    qDebug() << "Online state changed, isOnline=" << isOnline << "isConnected=" << isConnected << "connectRequested=" << m_connectRequested << "autoReconnect=" << m_autoReconnect << "autoOnline=" << m_autoOnline << "socketState=" << m_protocol->connectionState();
    if (isOnline) {
        if (m_connectRequested && m_protocol->connectionState() != QAbstractSocket::ConnectedState && m_protocol->connectionState() != QAbstractSocket::ConnectingState)
            m_protocol->connectToCore();
    } else {
        if (m_protocol->connectionState() != QAbstractSocket::UnconnectedState && m_protocol->connectionState() != QAbstractSocket::ClosingState)
            disconnectFromCore();
    }
#endif
}

void Manager::slotNetworkUpdated()
{
    qDebug() << "Manager::slotNetworkUpdated";
}

void Manager::slotSessionOpened()
{
    qDebug() << "Manager::slotSessionOpened";
}

void Manager::slotSessionClosed()
{
    qDebug() << "Manager::slotSessionClosed";
}

void Manager::slotSessionStateChanged(QNetworkSession::State state)
{
    qDebug() << "Manager::slotSessionStateChanged state=" << state;
}

void Manager::slotSessionError(QNetworkSession::SessionError error)
{
    qDebug() << "Manager::slotSessionError error=" << error;
    disconnectFromCore();
}

void Manager::slotSessionNewActivated()
{
    qDebug() << "Manager::slotSessionNewActivated";
#ifdef ENABLE_QNETWORKCONFIG
    m_networkSession->accept();
#endif
}

void Manager::slotSessionPreferredChanged(const QNetworkConfiguration &config, bool isSeamless)
{
#ifdef ENABLE_QNETWORKCONFIG
    if (config.isValid())
        qDebug() << "Manager::slotSessionPreferredChanged configBearer=" << config.bearerName() << "configIdentifier=" << config.identifier() << "configName=" << config.name() << "isSeamless=" << isSeamless;
    else
        qDebug() << "Manager::slotSessionPreferredChanged Invalid network configuration - isSeamless=" << isSeamless;
    disconnectFromCore();
    m_networkSession->migrate();
#else
    Q_UNUSED(config)
    Q_UNUSED(isSeamless)
#endif
}

void Manager::slotStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "Manager::slotStateChanged() socketState=" << state;
    if (state == QAbstractSocket::ConnectingState)
        emit connectingToCore();

#ifdef ENABLE_QNETWORKCONFIG
    if (state == QAbstractSocket::UnconnectedState)
        if (m_networkSession && m_networkSession->isOpen())
            m_networkSession->close();
#endif
}

void Manager::slotConnectedWithCore()
{
    qDebug() << "Manager::slotConnectedWithCore()";
    emit connectedToCore();
}

void Manager::slotDisconnectedFromCore(int err)
{
    qDebug() << "Manager::slotDisconnectedFromCore()" << err;
    m_connectRequested = false;
    m_reconnectTimer.stop();
    emit disconnectedFromCore(err);
    if (m_autoReconnect && err != ProtocolInterface::NoError)
        m_reconnectTimer.start();
}

QByteArray Manager::errorName(int err) const
{
    ProtocolInterface::enumDisconnectError e = (ProtocolInterface::enumDisconnectError) err;
    return m_protocol->errorName(e);
}

QString Manager::errorMessage(int err, bool summary) const
{
    ProtocolInterface::enumDisconnectError e = (ProtocolInterface::enumDisconnectError) err;
    return summary ? m_protocol->errorTextShort(e) : m_protocol->errorTextLong(e);
}

void Manager::slotReconnectTimeout()
{
    qDebug() << "Manager::slotReconnectTimeout connectRequested=" << m_connectRequested << "autoReconnect=" << m_autoReconnect << "autoOnline=" << m_autoOnline << "socketState=" << m_protocol->connectionState();
    if (m_connectRequested)
        return;
    connectToCore();
}

void Manager::slotClientStats(int64 ul, int64 dl, int64 sh, int nsh, int tul, int tdl, int uul, int udl, int ndl, int ncp)
{
    Q_UNUSED(sh);
    Q_UNUSED(nsh);
    Q_UNUSED(uul);
    Q_UNUSED(udl);
    QString files = QString("%1/%2").arg(ndl).arg(ncp);
    QString transfer = QString("%1/%2").arg(FileInfo::humanReadableSize(dl)).arg(FileInfo::humanReadableSize(ul));
    QString rate = QString("%1/%2").arg(QString::number((double)tdl / 1024.0,'f',1)).arg(QString::number((double)tul / 1024.0,'f',1));
    emit clientStats(files, transfer, rate);
}

void Manager::slotHostModified()
{
    m_settingsModified = true;
}
