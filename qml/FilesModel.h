#ifndef FILESMODEL_H
#define FILESMODEL_H

#include <QVector>
#include <QObject>

#include "BaseModel.h"

#include "donkeyprotocol.h"

class Manager;

class FilesModel : public BaseProxyModel
{
    Q_OBJECT
public:
    explicit FilesModel(QObject *parent = 0);
    virtual ~FilesModel();

    enum Roles {
        Identifier = Qt::UserRole,
        ReceivedRole,
        SizeRole,
        ProgressValueRole,
        ProgressTextRole,
        SpeedRole,
        EtaRole,
        PriorityRole,
        NetworkRole,
        HashRole,
        LastSeenRole,
        AgeRole,
        StateTextRole,
        StateValueRole
    };

public slots:
    int updateInterval() const;
    void setUpdateInterval(int milliseconds);

    bool enableUpdates() const;
    void setEnableUpdates(bool enable);

    void update();

    bool saveFile(const QByteArray &identifier, const QString &fileName = QString());
    bool renameFile(const QByteArray &identifier, const QString &newFileName);
    bool cancelFile(const QByteArray &identifier);
    bool pauseFile(const QByteArray &identifier);
    bool resumeFile(const QByteArray &identifier);
    bool retryFile(const QByteArray &identifier);

signals:
    void fileChanged(const QByteArray &identifier = QByteArray());

private slots:
    void updatedFiles();
    void updatedFile(int fileNo, DonkeyProtocol::FromCore reason);
    void updateTimeout();
    void addedFile(int fileNo);
    void removedFile(int fileNo);

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    class Private;
    friend class Private;
    Private *d;

    void updateFiles();
};

#endif // FILESMODEL_H
