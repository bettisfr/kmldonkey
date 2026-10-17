#ifndef SEARCHESMODEL_H
#define SEARCHESMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "BaseModel.h"

class Manager;

class SearchesModel : public BaseProxyModel
{
    Q_OBJECT
public:
    explicit SearchesModel(QObject *parent = 0);
    virtual ~SearchesModel();

    enum Roles {
        IdentifierRole = Qt::UserRole,
        FullQueryRole,
        SimpleQueryRole,
        HitsRole,
        FetchedRole,
        TypeRole
    };

signals:
    
public slots:
    QObject* queriesModel() const;
    QObject* resultsModel() const;

private slots:
    void slotReset();

private:
    class Private;
    friend class Private;
    Private *d;
};

#endif // SEARCHESMODEL_H
