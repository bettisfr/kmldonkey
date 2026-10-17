#include "SearchQueriesModel.h"
#include "Manager.h"
#include "BaseModel.h"

#include "searchquery.h"

#include <QDeclarativeComponent>

class SearchQueriesModel::Private : public BaseModel
{
    friend class SearchQueriesModel;
public:
    explicit Private(SearchQueriesModel *q) : BaseModel(q), q(q)
    {
        QHash<int, QByteArray> roles;
        roles[SearchQueriesModel::IdentifierRole] = "identifier";
        roles[Qt::DisplayRole] = "name";
        roles[FullQueryRole] = "fullQuery";
        roles[SimpleQueryRole] = "simpleQuery";
        setRoleNames(roles);
    }

    virtual int	columnCount(const QModelIndex & = QModelIndex()) const
    {
        return roleNames().count();
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        SearchQuery *sq = static_cast<SearchQuery*>(index.internalPointer());
        if (!sq) {
            qDebug()<<"Error: Invalid index in SearchQueriesModel::Private::data  with role="<<role<<"isValid="<<index.isValid()<<"row="<<index.row();
            return QVariant();
        }
        switch (role) {
            case Qt::DisplayRole:
            case SearchQueriesModel::IdentifierRole:
                return identifierFromRow(index.row());
            case SearchQueriesModel::FullQueryRole:
                return sq->getQuerystring(false);
            case SearchQueriesModel::SimpleQueryRole:
                return sq->getQuerystring(true);
            default:
                break;
        }
        return QVariant();
    }

    virtual QModelIndex	index(int row, int column, const QModelIndex & = QModelIndex()) const
    {
        QByteArray identifier = identifierFromRow(row);
        SearchQuery *sq = manager()->protocol()->definedSearches().value(identifier);
        if (!sq)
            return QModelIndex();
        return createIndex(row, column, sq);
    }

    virtual QModelIndex	parent(const QModelIndex &) const
    {
        return QModelIndex();
    }

    virtual int	rowCount(const QModelIndex & = QModelIndex()) const
    {
        if (manager()->protocol()->isConnected())
            return manager()->protocol()->definedSearches().count();
        return 0;
    }

protected:
    virtual void virtual_connected()
    {
    }

    virtual void virtual_disconnected(int)
    {
    }

    virtual void virtual_setManager()
    {
        if (Manager *m = manager()) {
            QObject::connect(m->protocol(), SIGNAL(definedSearchesUpdated()), q, SLOT(slotDefinedSearchesUpdated()));
        }
    }

private:
    SearchQueriesModel *q;
};

SearchQueriesModel::SearchQueriesModel(QObject *parent)
    : BaseProxyModel(parent)
    , d(new Private(this))
{
    setSourceModel(d);
}

SearchQueriesModel::~SearchQueriesModel()
{
    delete d;
}

SearchQuery* SearchQueriesModel::searchQuery(const QString &identifier)
{
    QModelIndex index = d->indexFromIdentifier(identifier.toUtf8());
    if (!index.isValid())
        return 0;
    SearchQuery *sq = static_cast<SearchQuery*>(index.internalPointer());
    return sq;
}

QObject* SearchQueriesModel::query(const QString &identifier)
{
    return searchQuery(identifier);
}

QString SearchQueriesModel::queryString(const QString &identifier)
{
    SearchQuery *sq = searchQuery(identifier);
    return sq ? sq->queryString() : QString();
}

int SearchQueriesModel::startSearchQuery(SearchQuery *searchQuery, int maxHits, const QString &searchType, int network)
{
    static int searchNum = 0;
    for(++searchNum; manager()->protocol()->findSearchNo(searchNum) || manager()->protocol()->findUnmappedSearchNo(searchNum); ++searchNum);

    DonkeyProtocol::SearchType st = DonkeyProtocol::RemoteSearch;
    if (searchType == "local")
        st = DonkeyProtocol::LocalSearch;
    else if (searchType == "subscribe")
        st = DonkeyProtocol::SubscribeSearch;

    manager()->protocol()->startSearch(searchNum, searchQuery, maxHits, st, network);
    return searchNum;
}

QByteArray SearchQueriesModel::startSearch(const QString &queryString, int maxHits, const QString &searchType, int network)
{
    SearchQuery *sq = SearchQuery::getQuery(queryString);
    int searchNum = sq ? startSearchQuery(sq, maxHits, searchType, network) : -1;
    delete sq;
    return QByteArray::number(searchNum);
}

QByteArray SearchQueriesModel::startSearchQuery(QObject *searchQuery, int maxHits, const QString &searchType, int network)
{
    SearchQuery *sq = dynamic_cast<SearchQuery*>(searchQuery);
    int searchNum = sq ? startSearchQuery(sq, maxHits, searchType, network) : -1;
    return QByteArray::number(searchNum);
}

bool SearchQueriesModel::stopSearch(const QString &identifier)
{
    bool ok;
    int searchNum = identifier.toInt(&ok);
    if (!ok || !manager()->protocol()->findSearchNo(searchNum))
        return false;
    manager()->protocol()->stopSearch(searchNum);
    return true;
}

void SearchQueriesModel::slotDefinedSearchesUpdated()
{
    qDebug() << "SearchQueriesModel::slotDefinedSearchesUpdated";
    d->beginResetModel();
    d->clearIdentifiers();
    if (manager()->protocol()->isConnected()) {
        foreach(QString s, manager()->protocol()->definedSearches().keys())
            d->addIdentifier(s.toUtf8());
    }
    d->endResetModel();
}
