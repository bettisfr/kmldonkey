#ifndef SEARCHQUERIESMODEL_H
#define SEARCHQUERIESMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "BaseModel.h"

class Manager;
class SearchQuery;

class SearchQueriesModel : public BaseProxyModel
{
    Q_OBJECT
public:
    explicit SearchQueriesModel(QObject *parent = 0);
    virtual ~SearchQueriesModel();
    SearchQuery* searchQuery(const QString &identifier);

    enum Roles {
        IdentifierRole = Qt::UserRole,
        FullQueryRole,
        SimpleQueryRole
    };

    int startSearchQuery(SearchQuery *searchQuery, int maxHits = 200, const QString &searchType = "remote", int network = 0);

public slots:
    QObject* query(const QString &identifier);
    QString queryString(const QString &identifier);

    QByteArray startSearch(const QString &queryString, int maxHits = 200, const QString &searchType = "remote", int network = 0);
    QByteArray startSearchQuery(QObject *searchQuery, int maxHits = 200, const QString &searchType = "remote", int network = 0);

    bool stopSearch(const QString &identifier);

private slots:
    void slotDefinedSearchesUpdated();

private:
    class Private;
    friend class Private;
    Private *d;
};

#endif // SearchQueriesModelL_H
