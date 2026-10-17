#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "BaseModel.h"

class Manager;
class ResultInfo;

class SearchResultModel : public BaseProxyModel
{
    Q_OBJECT
public:
    explicit SearchResultModel(QObject *parent = 0);
    virtual ~SearchResultModel();

    enum Roles {
        IdentifierRole = Qt::UserRole,
        SizeRole,
        NetworkNameRole,
        AvailabilityRole,
        FormatRole,
        AlreadyDoneRole
    };

public slots:
    void setSearchResult(const QByteArray &identifier);
    bool startDownload(const QByteArray &identifier, bool force = true, const QString &fileName = QString());
    bool stopSearch();

private slots:
    void slotSearchRequest(int searchNum);
    void slotSearchUpdated(int searchNum, QList<ResultInfo*> resultInfos);

private:
    class Private;
    friend class Private;
    Private *d;
};

#endif // SEARCHRESULTMODEL_H
