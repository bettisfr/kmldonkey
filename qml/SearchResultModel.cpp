#include "SearchResultModel.h"
#include "Manager.h"
#include "BaseModel.h"

#include "searchinfo.h"
#include "fileinfo.h"
#include "network.h"

#include <QFileInfo>

class SearchResultModel::Private : public BaseModel
{
    friend class SearchResultModel;
public:
    explicit Private(SearchResultModel *q) : BaseModel(q), q(q), m_searchInfo(0)
    {
        QHash<int, QByteArray> roles;
        roles[SearchResultModel::IdentifierRole] = "identifier";
        roles[Qt::DisplayRole] = "name";
        roles[SearchResultModel::SizeRole] = "size";
        roles[SearchResultModel::NetworkNameRole] = "network";
        roles[SearchResultModel::AvailabilityRole] = "availability";
        roles[SearchResultModel::FormatRole] = "format";
        roles[SearchResultModel::AlreadyDoneRole] = "alreadyDone";
        setRoleNames(roles);
    }

    virtual int	columnCount(const QModelIndex & = QModelIndex()) const
    {
        return roleNames().count();
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        ResultInfo *ri = static_cast<ResultInfo*>(index.internalPointer());
        if (!ri)
            return QVariant();
        switch (role) {
            case SearchResultModel::IdentifierRole:
                return ri->resultNo();
            case Qt::DisplayRole:
                return ri->resultName();
            case SearchResultModel::SizeRole:
                return FileInfo::humanReadableSize( ri->resultSize() );
            case SearchResultModel::NetworkNameRole: {
                Network* nw = manager()->protocol()->findNetworkNo( ri->resultNetwork() );
                return nw ? nw->networkName() : QString::number( ri->resultNetwork() );
            }
            case SearchResultModel::AvailabilityRole: {
                const QMap<QString,QVariant>& tags = ri->resultTags();
                return tags.value( "availability" ).toInt();
            }
            case SearchResultModel::FormatRole: {
                QString f = ri->resultFormat();
                if (f.isEmpty())
                    f = QFileInfo(ri->resultName()).suffix();
                return f;
            }
            case SearchResultModel::AlreadyDoneRole:
                return ri->resultAlreadyDone();
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
        ResultInfo *ri = m_searchInfo ? m_searchInfo->searchResults().value(id) : 0;
        if (!ri)
            return QModelIndex();
        return createIndex(row, column, ri);
    }

    virtual QModelIndex	parent(const QModelIndex &) const
    {
        return QModelIndex();
    }

    virtual int	rowCount(const QModelIndex & = QModelIndex()) const
    {
        return rowsToIdentifiers().count();
    }

    SearchInfo* setSearchResult(const QByteArray &identifier)
    {
        m_identifier = identifier;
        m_searchInfo = 0;
        bool resetDone;
        SearchInfo *si = searchInfo(-1, &resetDone);
        if (si != m_searchInfo)
            qWarning() << "SearchResultModel::setSearchResult Invalid state: si != m_searchInfo";
        if (si && !resetDone)
            qWarning() << "SearchResultModel::setSearchResult Invalid state: si && !resetDone";
        if (!resetDone) {
            beginResetModel();
            clearIdentifiers();
            endResetModel();
        }
        return si;
    }

    void resetSearchInfo()
    {
        beginResetModel();
        clearIdentifiers();
        if (SearchInfo *si = searchInfo()) {
            if (!si->isFetched()) {
                qDebug() << "SearchResultModel: Search not fetched yet, fetching now searchNo=" << si->searchNo();
                manager()->protocol()->getSearch( si->searchNo() );
            } else {
                const ResultInfoHash& searchResults = si->searchResults();
                qDebug() << "SearchResultModel: SearchInfo for searchNum=" << m_identifier << "has searchResults.count=" << searchResults.count();
                for(ResultInfoHash::ConstIterator it = searchResults.constBegin(); it != searchResults.constEnd(); ++it) {
                    addIdentifier(QByteArray::number(it.key()));
                }
            }
        } else {
            qDebug() << "SearchResultModel: No SearchInfo for searchNum=" << m_identifier;
        }
        endResetModel();
    }

    SearchInfo* searchInfo(int whenIdentifier = -1, bool *resetDone = 0)
    {
        if (resetDone)
            *resetDone = false;

        if (m_searchInfo) {
            if (whenIdentifier >= 0 && whenIdentifier != m_searchInfo->searchNo())
                return 0;
            return m_searchInfo;
        }

        bool ok;
        int searchNum = m_identifier.toInt(&ok);
        if (!ok)
            return 0;

        if (whenIdentifier >= 0 && whenIdentifier != searchNum)
            return 0;

        SearchInfo *si = manager()->protocol()->findSearchNo(searchNum);
        if (!si)
            return 0;

        m_searchInfo = si;
        resetSearchInfo();

        if (resetDone)
            *resetDone = true;

        return m_searchInfo;
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
            QObject::connect(m->protocol(), SIGNAL(searchRequest(int)), q, SLOT(slotSearchRequest(int)));
            QObject::connect(m->protocol(), SIGNAL(searchUpdated(int,QList<ResultInfo*>)), q, SLOT(slotSearchUpdated(int,QList<ResultInfo*>)));
        }
    }

private:
    SearchResultModel *q;
    QByteArray m_identifier;
    SearchInfo *m_searchInfo;
};

SearchResultModel::SearchResultModel(QObject *parent)
    : BaseProxyModel(parent)
    , d(new Private(this))
{
    setDynamicSortFilter(true);
    //setSortCaseSensitivity(Qt::CaseInsensitive);
    setSortRole(SearchResultModel::AvailabilityRole);
    sort(0, Qt::DescendingOrder);

    setSourceModel(d);
}

SearchResultModel::~SearchResultModel()
{
    qDebug()<<"SearchResultModel dtor";
    delete d;
}

void SearchResultModel::slotSearchRequest(int searchNum)
{
    bool resetDone;
    SearchInfo *si = d->searchInfo(searchNum, &resetDone);
    if (si && !resetDone)
        d->reset();
}

void SearchResultModel::slotSearchUpdated(int searchNum, QList<ResultInfo*> resultInfos)
{
    Q_UNUSED(resultInfos);

    bool resetDone;
    SearchInfo *si = d->searchInfo(searchNum, &resetDone);
    if (!si || resetDone)
        return;

    /*
    Q_FOREACH(ResultInfo *ri, resultInfos) {
        QByteArray identifier = QByteArray::number(ri->resultNo());
        int row = d->rowFromIdentifier(identifier);
        if (row < 0) {
            int count = d->rowsToIdentifiers().count();
            d->beginInsertRows(QModelIndex(), count, count);
            d->addIdentifier(identifier);
            d->endInsertRows();
        } else {
            QModelIndex idx = d->index(row, 0);
            if (idx.isValid()) {
                emit d->dataChanged(idx, idx);
            } else {
                d->resetSearchInfo();
                return;
            }
        }
    }
    */

    d->resetSearchInfo();
}

void SearchResultModel::setSearchResult(const QByteArray &identifier)
{
    qDebug() << "SearchResultModel::setSearchResult identifier=" << identifier;
    d->setSearchResult(identifier);
}

bool SearchResultModel::startDownload(const QByteArray &searchIdentifier, bool force, const QString &fileName)
{
    if (!manager())
        return false;
    SearchInfo *si = d->searchInfo();
    if (!si)
        return false;
    bool ok;
    int num = searchIdentifier.toInt(&ok);
    ResultInfo *ri = ok ? si->searchResults().value(num) : 0;
    if (!ri)
        return false;
    QStringList fileNames;
    fileNames.append(fileName.isEmpty() ? ri->resultName() : fileName);
    manager()->protocol()->startDownload(fileNames, num, force);
    return true;
}

bool SearchResultModel::stopSearch()
{
    if (!manager())
        return false;
    SearchInfo *si = d->searchInfo();
    if (!si)
        return false;
    manager()->protocol()->stopSearch( si->searchNo() );
    return true;
}
