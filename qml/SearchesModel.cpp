#include "SearchesModel.h"
#include "SearchQueriesModel.h"
#include "SearchResultsModel.h"
#include "Manager.h"

class SearchesModel::Private : public BaseModel
{
    friend class SearchesModel;
public:
    explicit Private(SearchesModel *q) : BaseModel(q), q(q)
    {
        QHash<int, QByteArray> roles;
        roles[SearchesModel::IdentifierRole] = "identifier";
        roles[Qt::DisplayRole] = "name";
        roles[SearchesModel::FullQueryRole] = "fullQuery";
        roles[SearchesModel::SimpleQueryRole] = "simpleQuery";
        roles[SearchesModel::HitsRole] = "hits";
        roles[SearchesModel::FetchedRole] = "fetched";
        roles[SearchesModel::TypeRole] = "type";
        setRoleNames(roles);

        m_queriesModel = new SearchQueriesModel(this);
        m_resultsModel = new SearchResultsModel(this);

        qRegisterMetaType<QModelIndex>("QModelIndex");

        connect(m_queriesModel, SIGNAL(modelReset()), q, SLOT(slotReset()));

        connect(m_resultsModel, SIGNAL(modelReset()), q, SLOT(slotReset()));
        connect(m_resultsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), q, SLOT(slotReset()));
    }

    virtual Manager *manager() const
    {
        //Q_ASSERT(BaseModel::manager());
        //Q_ASSERT(m_queriesModel && m_resultsModel);
        //Q_ASSERT(m_queriesModel->baseModel() && m_resultsModel->baseModel());
        //Q_ASSERT(BaseModel::manager() == m_queriesModel->manager());
        //Q_ASSERT(m_queriesModel->manager() == m_resultsModel->manager());
        return BaseModel::manager();
    }

    virtual void setManager(Manager *manager)
    {
        //Q_ASSERT(m_queriesModel && m_resultsModel);
        //Q_ASSERT(m_queriesModel->baseModel() && m_resultsModel->baseModel());
        //Q_ASSERT(BaseModel::manager() == m_queriesModel->manager());
        //Q_ASSERT(m_queriesModel->manager() == m_resultsModel->manager());
        m_queriesModel->setManager(manager);
        m_resultsModel->setManager(manager);
        BaseModel::setManager(manager);
    }

    virtual int	columnCount(const QModelIndex & = QModelIndex()) const
    {
        return roleNames().count();
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (index.row() < m_queriesModel->rowCount()) {
            const QModelIndex idx = m_queriesModel->index(index.row(), index.column());
            switch (role) {
                case SearchesModel::IdentifierRole:
                    return m_queriesModel->data(idx, SearchQueriesModel::IdentifierRole);
                case Qt::DisplayRole:
                    return m_queriesModel->data(idx, Qt::DisplayRole);
                case SearchesModel::FullQueryRole:
                    return m_queriesModel->data(idx, SearchQueriesModel::FullQueryRole);
                case SearchesModel::SimpleQueryRole:
                    return m_queriesModel->data(idx, SearchQueriesModel::SimpleQueryRole);
                case SearchesModel::HitsRole:
                    return "";
                case SearchesModel::FetchedRole:
                    return true;
                case SearchesModel::TypeRole:
                    return tr("Query");
                default:
                    break;
            }
        } else if (index.row() < m_queriesModel->rowCount() + m_resultsModel->rowCount()) {
            const QModelIndex idx = m_resultsModel->index(index.row() - m_queriesModel->rowCount(), index.column());
            switch (role) {
                case SearchesModel::IdentifierRole:
                    return m_resultsModel->data(idx, SearchResultsModel::IdentifierRole);
                case Qt::DisplayRole:
                    return m_resultsModel->data(idx, Qt::DisplayRole);
                case SearchesModel::FullQueryRole:
                    return m_resultsModel->data(idx, SearchResultsModel::FullQueryRole);
                case SearchesModel::SimpleQueryRole:
                    return m_resultsModel->data(idx, SearchResultsModel::SimpleQueryRole);
                case SearchesModel::HitsRole:
                    return m_resultsModel->data(idx, SearchResultsModel::HitsRole);
                case SearchesModel::FetchedRole:
                    return m_resultsModel->data(idx, SearchResultsModel::FetchedRole);
                case SearchesModel::TypeRole:
                    return tr("Result");
                default:
                    break;
            }
        }
        return QVariant();
    }

    virtual QModelIndex	index(int row, int column, const QModelIndex & = QModelIndex()) const
    {
        QModelIndex idx;
        if (row < m_queriesModel->rowCount()) {
            idx = m_queriesModel->index(row, column);
        } else if (row < m_queriesModel->rowCount() + m_resultsModel->rowCount()) {
            idx = m_resultsModel->index(row, column);
        }
        return createIndex(row, column, idx.internalPointer());
    }

    virtual QModelIndex	parent(const QModelIndex &) const
    {
        return QModelIndex();
    }

    virtual int	rowCount(const QModelIndex & = QModelIndex()) const
    {
        return m_queriesModel->rowCount() + m_resultsModel->rowCount();
    }

private:
    SearchesModel *q;
    SearchQueriesModel *m_queriesModel;
    SearchResultsModel *m_resultsModel;
};

SearchesModel::SearchesModel(QObject *parent)
    : BaseProxyModel(parent)
    , d(new Private(this))
{
    setSourceModel(d);
    setSortRole(Qt::DisplayRole);
}

SearchesModel::~SearchesModel()
{
    delete d;
}

QObject* SearchesModel::queriesModel() const
{
    return d->m_queriesModel;
}

QObject* SearchesModel::resultsModel() const
{
    return d->m_resultsModel;
}

void SearchesModel::slotReset()
{
    qDebug()<<"SearchesModel::slotReset";
    d->reset();
}
