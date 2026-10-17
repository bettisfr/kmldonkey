#include "SearchResultsModel.h"
#include "Manager.h"
#include "BaseModel.h"
#include "SearchResultModel.h"

#include "searchinfo.h"

class SearchResultsModel::Private : public BaseModel
{
    friend class SearchResultsModel;
public:
    explicit Private(SearchResultsModel *q) : BaseModel(q), q(q), m_resultModel(0)
    {
        QHash<int, QByteArray> roles;
        roles[SearchResultsModel::IdentifierRole] = "identifier";
        roles[Qt::DisplayRole] = "name";
        roles[SearchResultsModel::FullQueryRole] = "fullQuery";
        roles[SearchResultsModel::SimpleQueryRole] = "simpleQuery";
        roles[SearchResultsModel::HitsRole] = "hits";
        roles[SearchResultsModel::FetchedRole] = "fetched";
        setRoleNames(roles);
    }

    ~Private()
    {
    }

    virtual int	columnCount(const QModelIndex & = QModelIndex()) const
    {
        return roleNames().count();
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        SearchInfo *si = static_cast<SearchInfo*>(index.internalPointer());
        //Q_ASSERT(si == search(index.row()));
        //Q_ASSERT(searchId(index.row()) == si->searchNo());
        switch (role) {
            case Qt::DisplayRole:
                return si->getQuerystring(true);
            case SearchResultsModel::IdentifierRole:
                return si->searchNo();
            case SearchResultsModel::FullQueryRole:
                return si->getQuerystring(false);
            case SearchResultsModel::SimpleQueryRole:
                return si->getQuerystring(true);
            case SearchResultsModel::HitsRole:
                return si->searchResults().count();
            case SearchResultsModel::FetchedRole:
                return si->isFetched();
            default:
                break;
        }
        return QVariant();
    }

    virtual QModelIndex	index(int row, int column, const QModelIndex & = QModelIndex()) const
    {
        QByteArray identifier = identifierFromRow(row);
        bool ok;
        int id = identifier.toInt(&ok);
        if (!ok)
            return QModelIndex();
        SearchInfo *si = manager()->protocol()->activeSearches().value(id);
        if (!si)
            return QModelIndex();
        return createIndex(row, column, si);
    }

    virtual QModelIndex	parent(const QModelIndex &) const
    {
        return QModelIndex();
    }

    virtual int	rowCount(const QModelIndex & = QModelIndex()) const
    {
        return rowsToIdentifiers().count();
    }

protected:
    virtual void virtual_connected()
    {
        qDebug()<<"SearchResultsModel activeSearches="<<manager()->protocol()->activeSearches().count();
        manager()->protocol()->getSearches();
    }

    virtual void virtual_disconnected(int)
    {
    }

    virtual void virtual_setManager()
    {
        if (Manager *m = manager()) {
            QObject::connect(m->protocol(), SIGNAL(searchRequest(int)), q, SLOT(slotSearchRequest(int)));
            QObject::connect(m->protocol(), SIGNAL(searchUpdated(int,QList<ResultInfo*>)), q, SLOT(slotSearchUpdated(int,QList<ResultInfo*>)));
        }
    }

private:
    SearchResultsModel *q;
    mutable SearchResultModel *m_resultModel;
};

SearchResultsModel::SearchResultsModel(QObject *parent)
    : BaseProxyModel(parent)
    , d(new Private(this))
{
    setSourceModel(d);
}

SearchResultsModel::~SearchResultsModel()
{
    qDebug()<<"SearchResultsModel dtor";
    delete d;
}

QObject* SearchResultsModel::resultModel()
{
    if (!d->m_resultModel) {
        d->m_resultModel = new SearchResultModel(this);
        d->m_resultModel->setManager(manager());
    }
    return d->m_resultModel;
}

QObject* SearchResultsModel::createResultModel(const QByteArray &searchIdentifier) const
{
    SearchResultModel *model = new SearchResultModel();
    model->setManager(manager());
    model->setSearchResult(searchIdentifier);
    return model;
}

void SearchResultsModel::slotSearchRequest(int searchNum)
{
    d->beginResetModel();
    d->clearIdentifiers();
    if (manager()->protocol()->isConnected()) {
        const SearchInfoHash &activeSearches = manager()->protocol()->activeSearches();
        for(SearchInfoHash::ConstIterator it = activeSearches.constBegin(); it != activeSearches.constEnd(); ++it) {
            d->addIdentifier(QByteArray::number(it.key()));
        }
    }
    qDebug() << "SearchResultsModel::slotSearchRequest(" << searchNum << ") identifierCount=" << d->rowsToIdentifiers().count();
    d->endResetModel();
}

void SearchResultsModel::slotSearchUpdated(int searchNum, QList<ResultInfo*> resultInfos)
{
    QModelIndex index = d->indexFromIdentifier(QByteArray::number(searchNum));
    qDebug() << "SearchResultsModel::slotSearchUpdated searchNum=" << searchNum << "alreadyKnown=" << index.isValid() << "resultInfos=" << resultInfos.count();
    if (index.isValid()) {
        //d->beginResetModel();
        emit d->dataChanged(index, index);
        //d->endResetModel();
    } else {
        //manager()->protocol()->getSearches();

        //d->beginResetModel();
        //d->addIdentifier(QByteArray::number(searchNum));
        //d->endResetModel();
//aaaaaaa;
        slotSearchRequest(searchNum);
    }
}
