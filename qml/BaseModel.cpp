#include "BaseModel.h"
#include "Manager.h"

BaseModel::BaseModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_clearIdentifiersOnDisconnect(true)
    , m_manager(0)
{
    connect(this, SIGNAL(modelAboutToBeReset()), this, SLOT(slotModelAboutToBeReset()));
    connect(this, SIGNAL(layoutAboutToBeChanged()), this, SLOT(slotLayoutAboutToBeChanged()));
}

BaseModel::~BaseModel()
{
}

Manager *BaseModel::manager() const
{
    return m_manager;
}

void BaseModel::setManager(Manager *manager)
{
    if (m_manager) {
        disconnect(m_manager->protocol(), 0, this, 0);
    }
    m_manager = manager;
    if (m_manager) {
        connect(m_manager->protocol(), SIGNAL(signalConnected()), this, SLOT(slotConnected()));
        connect(m_manager->protocol(), SIGNAL(signalDisconnected(int)), this, SLOT(slotDisconnected(int)));
    }
    virtual_setManager();
}

int BaseModel::role(const QByteArray& roleName) const
{
    if (m_roles.isEmpty()) {
        QHash<int, QByteArray> roles = roleNames();
        for(QHash<int, QByteArray>::ConstIterator it = roles.constBegin(); it != roles.constEnd(); ++it)
            m_roles[it.value()] = it.key();
    }
    if (!m_roles.contains(roleName)) {
        qWarning() << "BaseModel::role No such role with roleName=" << roleName;
        return -1;
    }
    return m_roles[roleName];
}

const QMap<QByteArray, int>& BaseModel::identifiersToRows() const
{
    return m_id2row;
}

const QVector<QByteArray>& BaseModel::rowsToIdentifiers() const
{
    return m_row2id;
}

int BaseModel::rowFromIdentifier(const QByteArray &identifier) const
{
    QMap<QByteArray, int>::ConstIterator it = m_id2row.constFind(identifier);
    if (it == m_id2row.constEnd())
        qWarning() << "BaseModel::rowFromIdentifier No such row for identifier=" << identifier;
    return it != m_id2row.constEnd() ? it.value() : -1;
}

QModelIndex BaseModel::indexFromIdentifier(const QByteArray &identifier) const
{
    int row = rowFromIdentifier(identifier);
    return row >= 0 ? index(row, 0) : QModelIndex();
}

QByteArray BaseModel::identifierFromRow(int row) const
{
    bool validRow = row >= 0 && row < m_row2id.count();
    if (!validRow)
        qWarning() << "BaseModel::identifierFromRow Invalid row=" << row;
    return validRow ? m_row2id[row] : QByteArray();
}

QByteArray BaseModel::identifierFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        qWarning() << "BaseModel::identifierFromIndex Invalid index";
    return index.isValid() ? identifierFromRow(index.row()) : QByteArray();
}

void BaseModel::clearIdentifiers()
{
    m_id2row.clear();
    m_row2id.clear();
}

bool BaseModel::addIdentifier(const QByteArray &identifier)
{
    if (m_id2row.contains(identifier))
        return false;
    m_id2row[identifier] = m_row2id.count();
    m_row2id.append(identifier);
    return true;
}

bool BaseModel::removeIdentifier(const QByteArray &identifier)
{
    if (!m_id2row.contains(identifier))
        return false;
    int row = m_id2row.take(identifier);
    m_row2id.remove(row);
    for(int r = row; r < m_row2id.count(); ++r)
        m_id2row[ m_row2id[r] ] = r;
    return true;
}

bool BaseModel::renameIdentifier(const QByteArray &fromIdentifier, const QByteArray &toIdentifier)
{
    if (!m_id2row.contains(fromIdentifier) || m_id2row.contains(toIdentifier))
        return false;
    int row = m_id2row.take(fromIdentifier);
    m_id2row[toIdentifier] = row;
    m_row2id[row] = toIdentifier;
    return true;
}

/*
bool BaseModel::insertRows(int row, int count, const QModelIndex &parent)
{
    //TODO we cannot do that cause we are missing the internalPointer here :-/
}

bool BaseModel::removeRows(int row, int count, const QModelIndex &parent)
{
    int end = row + count - 1;
    Q_ASSERT(end < m_row2id.count());
    beginRemoveRows(QModelIndex(), row, end);
    for(int r = end; r >= row; --r) {
        QByteArray identifier = m_row2id[r];
        m_id2row.remove(identifier);
        m_row2id.remove(r);
    }
    endRemoveRows();
    return true;
}
*/

void BaseModel::slotModelAboutToBeReset()
{
    qDebug() << "BaseModel::slotModelAboutToBeReset";
}

void BaseModel::slotLayoutAboutToBeChanged()
{
    qDebug() << "BaseModel::slotLayoutAboutToBeChanged";
}

void BaseModel::slotConnected()
{
    virtual_connected();
    emit connected();
}

void BaseModel::slotDisconnected(int err)
{
    if (m_clearIdentifiersOnDisconnect) {
        beginResetModel();
        virtual_disconnected(err);
        emit disconnected(err);
        clearIdentifiers();
        endResetModel();
    } else {
        virtual_disconnected(err);
        emit disconnected(err);
    }
}

BaseProxyModel::BaseProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

BaseProxyModel::~BaseProxyModel()
{
}

Manager *BaseProxyModel::manager() const
{
    BaseModel *m = baseModel();
    return m ? m->manager() : 0;
}

void BaseProxyModel::setManager(Manager *manager)
{
    BaseModel *m = baseModel();
    if (m)
        m->setManager(manager);
    else
        qWarning() << "BaseProxyModel::setManager Invalid BaseModel";
}

BaseModel *BaseProxyModel::baseModel() const
{
    return qobject_cast<BaseModel*>(sourceModel());
}

void BaseProxyModel::setSourceModel(QAbstractItemModel *model)
{
    if (QAbstractItemModel *prevModel = sourceModel()) {
        disconnect(prevModel, 0, this, 0);
    }
    QSortFilterProxyModel::setSourceModel(model);
    if (BaseModel *baseModel = qobject_cast<BaseModel*>(model)) {
        connect(baseModel, SIGNAL(connected()), this, SIGNAL(connected()));
        connect(baseModel, SIGNAL(disconnected(int)), this, SIGNAL(disconnected(int)));
    }
}

int BaseProxyModel::role(const QByteArray& roleName) const
{
    if (BaseModel *m = baseModel())
        return m->role(roleName);
    return -1;
}

QVariant BaseProxyModel::data(const QModelIndex &index, int role) const
{
    return QSortFilterProxyModel::data(index, role);
}

QVariant BaseProxyModel::data(const QByteArray &identifier, int role) const
{
    if (role < 0) {
        qWarning() << "BaseProxyModel::data Invalid role=" << role;
        return QVariant();
    }
    if (BaseModel *m = baseModel()) {
        QModelIndex idx = m->indexFromIdentifier(identifier);
        if (idx.isValid())
            return m->data(idx, role);
        else
            qWarning() << "BaseProxyModel::data Invalid identifier=" << identifier;
    }
    return QVariant();
}

QVariant BaseProxyModel::data(const QByteArray &identifier, const QByteArray &_role) const
{
    int r = role(_role);
    return data(identifier, r >= 0 ? r : Qt::DisplayRole);
}
