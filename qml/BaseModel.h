#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QByteArray>
#include <QVector>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QPersistentModelIndex>

class Manager;

class BaseModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit BaseModel(QObject *parent = 0);
    virtual ~BaseModel();

    virtual Manager *manager() const;
    virtual void setManager(Manager *manager);

    int role(const QByteArray& roleName) const;

    const QMap<QByteArray, int>& identifiersToRows() const;
    const QVector<QByteArray>& rowsToIdentifiers() const;

    int rowFromIdentifier(const QByteArray &identifier) const;
    QModelIndex indexFromIdentifier(const QByteArray &identifier) const;

    QByteArray identifierFromRow(int row) const;
    QByteArray identifierFromIndex(const QModelIndex &index) const;

signals:
    void connected();
    void disconnected(int);

protected:
    virtual void virtual_connected() {}
    virtual void virtual_disconnected(int) {}
    virtual void virtual_setManager() {}

    bool m_clearIdentifiersOnDisconnect;

    void clearIdentifiers();
    bool addIdentifier(const QByteArray &identifier);
    bool removeIdentifier(const QByteArray &identifier);
    bool renameIdentifier(const QByteArray &fromIdentifier, const QByteArray &toIdentifier);

    //virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private slots:
    void slotModelAboutToBeReset();
    void slotLayoutAboutToBeChanged();
    void slotConnected();
    void slotDisconnected(int);

private:
    Manager *m_manager;
    mutable QMap<QByteArray,int> m_roles;
    mutable QMap<QByteArray, int> m_id2row;
    mutable QVector<QByteArray> m_row2id;
};

class BaseProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit BaseProxyModel(QObject *parent = 0);
    virtual ~BaseProxyModel();

    Manager *manager() const;
    void setManager(Manager *manager);

    BaseModel *baseModel() const;
    virtual void setSourceModel(QAbstractItemModel *model);

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

signals:
    void connected();
    void disconnected(int);

public slots:
    int role(const QByteArray& roleName) const;
    QVariant data(const QByteArray &identifier, int role) const;
    QVariant data(const QByteArray &identifier, const QByteArray &role) const;
};

#endif // BASEMODEL_H
