#include "HostModel.h"

#include "Manager.h"

#include "donkeyhost.h"

class HostModel::Private : public BaseModel
{
    friend class HostModel;
public:
    explicit Private(HostModel *q) : BaseModel(q), q(q), m_currentHostState(Disconnected)
    {
        // The HostModel is special in that it keeps it's identifiers independend of the connection-state.
        BaseModel::m_clearIdentifiersOnDisconnect = false;

        QHash<int, QByteArray> roles;
        roles[HostModel::Identifier] = "identifier";
        roles[HostModel::NameRole] = "name";
        roles[HostModel::AddressRole] = "address";
        roles[HostModel::PortRole] = "port";
        roles[HostModel::UsernameRole] = "username";
        roles[HostModel::PasswordRole] = "userpass";
        roles[HostModel::DefaultRole] = "isDefault";
        roles[HostModel::SelectedRole] = "isSelected";
        roles[HostModel::ConnectedRole] = "isConnected";
        roles[HostModel::DisconnectedRole] = "isDisconnected";
        roles[HostModel::ConnectionStateRole] = "connectionState";
        setRoleNames(roles);
    }

    virtual int	columnCount(const QModelIndex & = QModelIndex()) const
    {
        return roleNames().count();
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        DonkeyHost *h = static_cast<DonkeyHost*>(index.internalPointer());
        if (!h)
            return QVariant();
        switch (role) {
            case Qt::DisplayRole:
            case HostModel::NameRole:
                return h->name();
            case HostModel::Identifier:
                return h->name().toUtf8();
            case HostModel::AddressRole:
                return h->address();
            case HostModel::PortRole:
                return h->port();
            case HostModel::UsernameRole:
                return h->username();
            case HostModel::PasswordRole:
                return h->password();
            case HostModel::DefaultRole: {
                Manager *m = manager();
                return m && q->host() == h;
            }
            case HostModel::SelectedRole: {
                Manager *m = manager();
                return m && m->protocol()->getHost() == h;
            }
            case HostModel::ConnectedRole: {
                Manager *m = manager();
                return m && m->protocol()->getHost() == h && m->protocol()->isConnected();
            }
            case HostModel::DisconnectedRole: {
                Manager *m = manager();
                if (!m || m->protocol()->getHost() != h)
                    return true;
                //return m->protocol()->connectionState() == QAbstractSocket::UnconnectedState || m->protocol()->connectionState() == QAbstractSocket::ClosingState;
                return m_currentHostState == Disconnected || m_currentHostState == Error;
            }
            case HostModel::ConnectionStateRole: {
                Manager *m = manager();
                if (m && m->protocol()->getHost() == h)
                    switch (m_currentHostState) {
                        case Connecting: return "connecting";
                        case Connected: return "connected";
                        case Disconnected: return "disconnected";
                        case Error: return "error";
                    }
                return "disconnected";
            }
            default:
                break;
        }
        return QVariant();
    }

    virtual QModelIndex	index(int row, int column, const QModelIndex & = QModelIndex()) const
    {
        QByteArray identifier = identifierFromRow(row);
        if (identifier.isEmpty()) {
            qWarning() << "HostModel::index Invalid row=" << row;
            return QModelIndex();
        }   
        DonkeyHost *h = q->host(identifier);
        return h ? createIndex(row, column, h) : QModelIndex();
    }

    virtual QModelIndex	parent(const QModelIndex &) const
    {
        return QModelIndex();
    }

    virtual int	rowCount(const QModelIndex & = QModelIndex()) const
    {
        return rowsToIdentifiers().count();
    }

    virtual void setManager(Manager *manager)
    {
        BaseModel::setManager(manager);
    }

protected:
    void virtual_connected()
    {
        q->updateHost(manager()->protocol()->getHost());
    }

    void virtual_disconnected(int)
    {
        q->updateHost(manager()->protocol()->getHost());
    }

    void virtual_setManager()
    {
        if (Manager *m = manager()) {
            connect(m, SIGNAL(connectingToCore()), q, SLOT(slotConnectingToCore()));
            connect(m, SIGNAL(connectedToCore()), q, SLOT(slotConnectedToCore()));
            connect(m, SIGNAL(disconnectedFromCore(int)), q, SLOT(slotDisconnectedFromCore(int)));
        }
    }

private:
    HostModel *q;
    QByteArray m_selectedHost;
    QMap<QByteArray, DonkeyHost*> m_hostMap;
    enum State { Connecting, Connected, Disconnected, Error };
    State m_currentHostState;
};

HostModel::HostModel(QObject *parent)
    : BaseProxyModel(parent)
    , d(new Private(this))
{
    setSourceModel(d);
}

HostModel::~HostModel()
{
    delete d;
}

DonkeyHost* HostModel::host() const
{
    QMap<QByteArray, DonkeyHost*>::ConstIterator it = d->m_hostMap.constFind(d->m_selectedHost);
    return it != d->m_hostMap.constEnd() ? it.value() : 0;
}

DonkeyHost* HostModel::host(const QByteArray &name) const
{
    return d->m_hostMap.value(name);
}

QVector<QByteArray> HostModel::hosts() const
{
    return d->rowsToIdentifiers();
}

QVariantList HostModel::hostList() const
{
    QVariantList result;
    Q_FOREACH(const QByteArray &name, d->rowsToIdentifiers())
        result.append(name);
    return result;
}

void HostModel::addHost(DonkeyHost *host)
{
    QByteArray name = host->name().toUtf8();
    DonkeyHost *oldHost = d->m_hostMap.take(name);
    if (oldHost) {
        if (oldHost != host)
            delete oldHost;
        d->removeIdentifier(name);
    }
    d->m_hostMap[name] = host;
    d->addIdentifier(name);
}

void HostModel::removeHost(DonkeyHost *host)
{
    QByteArray name = host->name().toUtf8();
    DonkeyHost *oldHost = d->m_hostMap.take(name);
    if (oldHost) {
        if (d->m_hostMap.keys(oldHost).isEmpty())
            delete oldHost;
        d->removeIdentifier(name);
    }
}

void HostModel::updateHost(HostInterface *host)
{
    QModelIndex index = d->indexFromIdentifier(host->name().toUtf8());
    if (index.isValid())
        emit d->dataChanged(index, index);
}

void HostModel::updateCurrentHost()
{
    HostInterface *h = manager()->protocol()->getHost();
    if (h)
        updateHost(h);
}

void HostModel::slotConnectingToCore()
{
    d->m_currentHostState = Private::Connecting;
    updateCurrentHost();
}

void HostModel::slotConnectedToCore()
{
    d->m_currentHostState = Private::Connected;
    updateCurrentHost();
}

void HostModel::slotDisconnectedFromCore(int err)
{
    d->m_currentHostState = err == ProtocolInterface::NoError ? Private::Disconnected : Private::Error;
    updateCurrentHost();
}

QByteArray HostModel::selectedHost() const
{
    return d->m_selectedHost;
}

void HostModel::setSelectedHost(const QByteArray &hostName)
{
    if (d->m_selectedHost == hostName)
        return;
    QModelIndex oldIndex = d->indexFromIdentifier(d->m_selectedHost);
    d->m_selectedHost = hostName;
    QModelIndex newIndex = d->indexFromIdentifier(d->m_selectedHost);
    if (oldIndex.isValid())
        emit d->dataChanged(oldIndex, oldIndex);
    if (newIndex.isValid())
        emit d->dataChanged(newIndex, newIndex);
    emit selectedHostChanged();
}

bool HostModel::addHost(const QByteArray &identifier)
{
    Manager *m = manager();
    if (!m)
        return false;
    d->beginResetModel();
    bool updated = host(identifier) != 0;
    DonkeyHost *host = new DonkeyHost();
    host->setName(identifier);
    addHost(host);
    d->endResetModel();
    if (updated)
        emit hostChanged(identifier);
    else
        emit hostAdded(identifier);
    return true;
}

bool HostModel::removeHost(const QByteArray &identifier)
{
    Manager *m = manager();
    if (!m)
        return false;
    DonkeyHost *h = host(identifier);
    if (!h)
        return false;
    d->beginResetModel();
    bool isDefaultHost = (identifier == d->m_selectedHost);
    bool isCurrentHost = (h == m->protocol()->getHost());
    if (isDefaultHost) {
        d->m_selectedHost.clear();
    }
    if (isCurrentHost) {
        //d->m_currentHostState = Private::Disconnected;
        m->protocol()->disconnectFromCore();
        m->protocol()->setHost(0);
    }
    //if (isDefaultHost || isCurrentHost) updateHost(h);
    removeHost(h);
    d->endResetModel();
    if (isDefaultHost)
        emit selectedHostChanged();
    emit hostRemoved(identifier);
    return true;
}

bool HostModel::setData(const QByteArray &identifier, const QByteArray &_role, const QVariant &value)
{
    Manager *m = manager();
    if (!m) {
        qDebug() << "HostModel::setData No Manager";
        return false;
    }
    QModelIndex index = d->indexFromIdentifier(identifier);
    if (!index.isValid()) {
        qDebug() << "HostModel::setData Invalid identifier=" << identifier;
        return false;
    }
    DonkeyHost *h = static_cast<DonkeyHost*>(index.internalPointer());
    if (!h) {
        qDebug() << "HostModel::setData Invalid host for identifier=" << identifier;
        return false;
    }
    int r = role(_role);
    if (r < 0) {
        qDebug() << "HostModel::setData Invalid role=" << _role;
        return false;
    }
    switch ((Roles)r) {
        case HostModel::DefaultRole:
        case HostModel::SelectedRole:
        case HostModel::ConnectedRole:
        case HostModel::DisconnectedRole:
        case HostModel::ConnectionStateRole: {
            qDebug() << "HostModel::setData role=" << _role << "is read-only";
            return false;
        }
        case HostModel::Identifier:
        case HostModel::NameRole: {
            QString name = value.toString();
            if (name == h->name())
                return true;
            QByteArray oldIdentifier = h->name().toUtf8();
            QByteArray newIdentifier = name.toUtf8();
            if (!d->renameIdentifier(oldIdentifier, newIdentifier))
                return false;
            h->setName(name);
            d->m_hostMap.insert(newIdentifier, d->m_hostMap.take(oldIdentifier));
            if (d->m_selectedHost == oldIdentifier)
                d->m_selectedHost = newIdentifier;
            break;
        }
        case HostModel::AddressRole: {
            QString address = value.toString();
            if (address == h->address())
                return true;
            h->setAddress(address);
            break;
        }
        case HostModel::PortRole: {
            bool ok;
            int port = value.toInt(&ok);
            if (!ok)
                return false;
            if (port == h->port())
                return true;
            h->setPort(port);
            break;
        }
        case HostModel::UsernameRole: {
            QString username = value.toString();
            if (username == h->username())
                return true;
            h->setUsername(username);
            break;
        }
        case HostModel::PasswordRole: {
            QString password = value.toString();
            if (password == h->password())
                return true;
            h->setPassword(password);
            break;
        }
    }
    emit d->dataChanged(index, index);
    emit hostChanged(identifier);
    return true;
}
