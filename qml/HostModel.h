#ifndef HOSTMODEL_H
#define HOSTMODEL_H

#include <QObject>

#include "BaseModel.h"

class HostInterface;
class DonkeyHost;

class HostModel : public BaseProxyModel
{
    Q_OBJECT
public:
    explicit HostModel(QObject *parent = 0);
    virtual ~HostModel();

    enum Roles {
        Identifier = Qt::UserRole,
        NameRole,
        AddressRole,
        PortRole,
        UsernameRole,
        PasswordRole,
        DefaultRole,
        SelectedRole,
        ConnectedRole,
        DisconnectedRole,
        ConnectionStateRole
    };

    DonkeyHost* host() const;
    DonkeyHost* host(const QByteArray &name) const;
    QVector<QByteArray> hosts() const;

    void addHost(DonkeyHost *host);
    void removeHost(DonkeyHost *host);

    void updateHost(HostInterface *host);

public slots:

    QByteArray selectedHost() const;
    void setSelectedHost(const QByteArray &hostName);

    QVariantList hostList() const;

    bool addHost(const QByteArray &identifier);
    bool removeHost(const QByteArray &identifier);
    bool setData(const QByteArray &identifier, const QByteArray &role, const QVariant &value);

signals:
    void selectedHostChanged();
    void hostAdded(const QByteArray &identifier);
    void hostRemoved(const QByteArray &identifier);
    void hostChanged(const QByteArray &identifier);

private slots:
    void updateCurrentHost();
    void slotConnectingToCore();
    void slotConnectedToCore();
    void slotDisconnectedFromCore(int err);

private:
    class Private;
    friend class Private;
    Private *d;
};

Q_DECLARE_METATYPE(HostModel*)

#endif // HOSTMODEL_H
