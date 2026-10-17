#ifndef SEARCHRESULTSMODEL_H
#define SEARCHRESULTSMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "BaseModel.h"

class Manager;
class ResultInfo;

class SearchResultsModel : public BaseProxyModel
{
    Q_OBJECT
public:
    explicit SearchResultsModel(QObject *parent = 0);
    virtual ~SearchResultsModel();

    enum Roles {
        IdentifierRole = Qt::UserRole,
        FullQueryRole,
        SimpleQueryRole,
        HitsRole,
        FetchedRole
    };

public slots:
    QObject* resultModel();
    QObject* createResultModel(const QByteArray &searchIdentifier) const;

private slots:
    void slotSearchRequest(int searchNum);
    void slotSearchUpdated(int searchNum, QList<ResultInfo*> resultInfos);

private:
    class Private;
    friend class Private;
    Private *d;
};


#endif // SEARCHRESULTSMODEL_H
