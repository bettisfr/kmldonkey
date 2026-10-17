#ifndef MANAGER_H
#define MANAGER_H

#include "donkeyprotocol.h"

#include <QObject>
#include <QTimer>
#include <QByteArray>
#include <QVector>
#include <QVariant>
#include <QNetworkSession>

class HostModel;
class QNetworkConfigurationManager;

class Manager : public QObject
{
    Q_OBJECT
public:
    Manager();
    ~Manager();

    DonkeyProtocol* protocol();

public slots:
    QObject* hostModel();

    bool autoConnect() const;
    void setAutoConnect(bool enable);

    bool autoReconnect() const;
    void setAutoReconnect(bool enable);

    bool autoOnline() const;
    void setAutoOnline(bool enable);

    bool savePassword() const;
    void setSavePassword(bool save);

    QVariant setting(const QString &name, const QVariant &defaultValue = QVariant()) const;
    void setSetting(const QString &name, const QVariant &value);

    bool isConnected() const;
    void connectToCore(const QByteArray &identifier = QByteArray());
    void connectToDefaultCore();
    void disconnectFromCore();

    QByteArray errorName(int err) const;
    QString errorMessage(int err, bool summary = true) const;

    void submitUrl(const QString &url);
    void sendConsoleMessage(const QString &message);

signals:
    void connectingToCore();
    void connectedToCore();
    void disconnectedFromCore(int err);
    void clientStats(QString files, QString transfer, QString rate);
    void definedSearchesUpdated();
    void receivedConsoleMessage(QString);

private slots:
    void slotStateChanged(QAbstractSocket::SocketState);
    void slotConnectedWithCore();
    void slotDisconnectedFromCore(int);
    void slotReconnectTimeout();

    void slotOnlineChanged();
    void slotNetworkUpdated();
    void slotSessionOpened();
    void slotSessionClosed();
    void slotSessionStateChanged(QNetworkSession::State);
    void slotSessionError(QNetworkSession::SessionError error);
    void slotSessionNewActivated();
    void slotSessionPreferredChanged(const QNetworkConfiguration &config, bool isSeamless);

    void slotClientStats(int64 ul, int64 dl, int64 sh, int nsh, int tul, int tdl, int uul, int udl, int ndl, int ncp);
    void slotHostModified();

private:
    QNetworkConfigurationManager *m_networkConfigManager;
    QNetworkSession *m_networkSession;

    mutable DonkeyProtocol *m_protocol;
    HostModel *m_hostModel;

    QTimer m_reconnectTimer;
    bool m_autoConnect;
    bool m_autoReconnect;
    bool m_autoOnline;
    bool m_savePassword;
    bool m_settingsModified;
    bool m_connectRequested;
    QVariantMap m_properties;
};

#endif // MANAGER_H
