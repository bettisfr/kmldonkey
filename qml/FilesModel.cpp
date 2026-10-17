#include "FilesModel.h"
#include "Manager.h"

#include "fileinfo.h"
#include "network.h"

#include <QDateTime>

class FilesModel::Private : public BaseModel
{
    friend class FilesModel;
public:
    explicit Private(FilesModel *q) : BaseModel(q), q(q)
    {
        QHash<int, QByteArray> roles;
        roles[FilesModel::Identifier] = "identifier";
        roles[Qt::DisplayRole] = "name";
        roles[FilesModel::ReceivedRole] = "received";
        roles[FilesModel::SizeRole] = "size";
        roles[FilesModel::ProgressValueRole] = "progressValue";
        roles[FilesModel::ProgressTextRole] = "progressText";
        roles[FilesModel::SpeedRole] = "speed";
        roles[FilesModel::EtaRole] = "eta";
        roles[FilesModel::PriorityRole] = "priority";
        roles[FilesModel::NetworkRole] = "network";
        roles[FilesModel::HashRole] = "hash";
        roles[FilesModel::LastSeenRole] = "lastSeen";
        roles[FilesModel::AgeRole] = "age";
        roles[FilesModel::StateTextRole] = "stateText";
        roles[FilesModel::StateValueRole] = "stateValue";
        setRoleNames(roles);

        m_updateTimer.setInterval(2000);
        connect(&m_updateTimer, SIGNAL(timeout()), q, SLOT(updateTimeout()));
        m_enableUpdates = false;
    }

    virtual int	columnCount(const QModelIndex & = QModelIndex()) const
    {
        return roleNames().count();
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        FileInfo *fi = static_cast<FileInfo*>(index.internalPointer());
        if (!fi)
            return QVariant();
        switch (role) {
            case FilesModel::Identifier:
                return fi->fileNo();
            case Qt::DisplayRole:
                return fi->fileName();
            case FilesModel::SizeRole:
                return FileInfo::humanReadableSize( fi->fileSize() );
            case FilesModel::ReceivedRole:
                return FileInfo::humanReadableSize( fi->fileDownloaded() );
            case FilesModel::ProgressValueRole: {
                if (fi->fileSize() == 0)
                    return 0.0;
                double v = ( fi->fileDownloaded() * 100.0 ) / fi->fileSize();
                return static_cast<double>(static_cast<int>(v * 100 + 0.5)) / 100.0;
            }
            case FilesModel::ProgressTextRole:
                return QString::number( fi->fileSize() ? ( fi->fileDownloaded() * 100.0 ) / fi->fileSize() : 0.0, 'f', 1 ) + "%";
            case FilesModel::SpeedRole:
            return ( fi->fileSpeed() > 0 ) ? FileInfo::humanReadableSpeed( fi->fileSpeed() ) : QString();
            case FilesModel::EtaRole:
                return FileInfo::calculateETA( fi );
            case FilesModel::PriorityRole:
                return FileInfo::humanReadablePriority( fi->filePriority() );
            case FilesModel::StateTextRole:
                switch (fi->fileState()) {
                    case FileInfo::Downloading: return fi->fileSpeed() ? tr("Downloading") : tr("Looking");
                    case FileInfo::Paused: return tr("Paused");
                    case FileInfo::Complete: return tr("Complete");
                    case FileInfo::Shared: return tr("Shared");
                    case FileInfo::Cancelled: return tr("Cancelled");
                    case FileInfo::New: return tr("New");
                    case FileInfo::Aborted: return tr("Aborted");
                    case FileInfo::Queued: return tr("Queued");
                }
                break;
            case FilesModel::StateValueRole:
                switch (fi->fileState()) {
                    case FileInfo::Downloading: return fi->fileSpeed() ? QLatin1String("Downloading") : QLatin1String("Looking");
                    case FileInfo::Paused: return QLatin1String("Paused");
                    case FileInfo::Complete: return QLatin1String("Complete");
                    case FileInfo::Shared: return QLatin1String("Shared");
                    case FileInfo::Cancelled: return QLatin1String("Cancelled");
                    case FileInfo::New: return QLatin1String("New");
                    case FileInfo::Aborted: return QLatin1String("Aborted");
                    case FileInfo::Queued: return QLatin1String("Queued");
                }
                break;
            case FilesModel::NetworkRole:
                return manager()->protocol()->findNetworkNo( fi->fileNetwork() )->networkName();
            case FilesModel::HashRole:
                return fi->fileUid();
            case FilesModel::LastSeenRole:
                return FileInfo::humanReadableTime( (time_t)fi->fileLastSeen(), true );
            case FilesModel::AgeRole: {
                time_t age = time( 0 ) - fi->fileAge();
                return FileInfo::humanReadableTime( age, true );
            }
            default:
                break;
        }
        return QVariant();
    }

    virtual QModelIndex	index(int row, int column, const QModelIndex & = QModelIndex()) const
    {
        QByteArray identifier = identifierFromRow(row);
        FileInfo *fi = fileInfo(identifier);
        if (!fi)
            return QModelIndex();
        return createIndex(row, column, fi);
    }

    virtual QModelIndex	parent(const QModelIndex &) const
    {
        return QModelIndex();
    }

    virtual int	rowCount(const QModelIndex & = QModelIndex()) const
    {
        return rowsToIdentifiers().count();
    }

    FileInfo* fileInfo(const QByteArray &identifier) const
    {
        bool ok;
        int id = identifier.toInt(&ok);
        if (!ok)
            return 0;

        FileInfo *fi = manager()->protocol()->downloadFiles().value(id);
        if (fi)
            return fi;

        fi = manager()->protocol()->downloadedFiles().value(id);
        return fi;
    }

    void updateFiles()
    {
        beginResetModel();
        m_updateFileList.clear();
        clearIdentifiers();
        if (manager()->protocol()->isConnected()) {
            const FileInfoHash &downloadFiles = manager()->protocol()->downloadFiles();
            for(FileInfoHash::ConstIterator it = downloadFiles.constBegin(); it != downloadFiles.constEnd(); ++it) {
                addIdentifier(QByteArray::number(it.key()));
            }
            const FileInfoHash &downloadedFiles = manager()->protocol()->downloadedFiles();
            for(FileInfoHash::ConstIterator it = downloadedFiles.constBegin(); it != downloadedFiles.constEnd(); ++it) {
                addIdentifier(QByteArray::number(it.key()));
            }
        }
        endResetModel();
        emit q->fileChanged();
    }

    QSet<int> updateFileList() const
    {
        return m_updateFileList;
    }

    QSet<int> takeUpdateFileList()
    {
        QSet<int> r = m_updateFileList;
        m_updateFileList.clear();
        return r;
    }

    void addUpdateFile(int fileNo)
    {
        m_updateFileList.insert(fileNo);
    }

    void removeUpdateFile(int fileNo)
    {
        m_updateFileList.remove(fileNo);
    }

protected:
    void virtual_connected()
    {
        q->update();
        if (m_enableUpdates && m_updateTimer.interval() >= 0)
            m_updateTimer.start();
    }

    void virtual_disconnected(int)
    {
        m_updateTimer.stop();
    }

    void virtual_setManager()
    {
        if (Manager *m = manager()) {
            connect(m->protocol(), SIGNAL(updatedDownloadFiles()), q, SLOT(updatedFiles()));
            connect(m->protocol(), SIGNAL(updatedDownloadedFiles()), q, SLOT(updatedFiles()));
            connect(m->protocol(), SIGNAL(fileUpdated(int,DonkeyProtocol::FromCore)), q, SLOT(updatedFile(int,DonkeyProtocol::FromCore)));
            connect(m->protocol(), SIGNAL(fileAdded(int,bool)), q, SLOT(addedFile(int)));
            connect(m->protocol(), SIGNAL(fileRemoved(int)), q, SLOT(removedFile(int)));
        }
    }

private:
    FilesModel *q;
    QTimer m_updateTimer;
    bool m_enableUpdates;
    QSet<int> m_updateFileList;
};

FilesModel::FilesModel(QObject *parent)
    : BaseProxyModel(parent)
    , d(new Private(this))
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
    setSourceModel(d);
}

FilesModel::~FilesModel()
{
    delete d;
}

int FilesModel::updateInterval() const
{
    return d->m_updateTimer.interval();
}

void FilesModel::setUpdateInterval(int milliseconds)
{
    if (milliseconds < 0)
        d->m_updateTimer.stop();
    d->m_updateTimer.setInterval(milliseconds);
}

bool FilesModel::enableUpdates() const
{
    return d->m_enableUpdates;
}

void FilesModel::setEnableUpdates(bool enable)
{
    if (d->m_enableUpdates == enable)
        return;
    d->m_enableUpdates = enable;
    if (d->m_enableUpdates) {
        if (d->m_updateTimer.interval() >= 0)
            updateTimeout();
    } else {
        qDebug() << "FilesModel::setEnableUpdates enable=" << enable;
        d->m_updateTimer.stop();
    }
}

void FilesModel::update()
{
    qDebug() << "FilesModel::update";
    d->m_updateTimer.stop();
    manager()->protocol()->updateDownloadFiles();
    manager()->protocol()->updateDownloadedFiles();
}

bool FilesModel::saveFile(const QByteArray &identifier, const QString &fileName)
{
    FileInfo *fi = d->fileInfo(identifier);
    if (!fi) return false;
    QString name = fileName.isEmpty() ? fi->fileName() : fileName;
    manager()->protocol()->saveFile(fi->fileNo(), name);
    return true;
}

bool FilesModel::renameFile(const QByteArray &identifier, const QString &newFileName)
{
    FileInfo *fi = d->fileInfo(identifier);
    if (!fi) return false;
    manager()->protocol()->renameFile(fi->fileNo(), newFileName);
    return true;
}

bool FilesModel::cancelFile(const QByteArray &identifier)
{
    FileInfo *fi = d->fileInfo(identifier);
    if (!fi) return false;
    manager()->protocol()->cancelFile(fi->fileNo());
    return true;
}

bool FilesModel::pauseFile(const QByteArray &identifier)
{
    FileInfo *fi = d->fileInfo(identifier);
    if (!fi) return false;
    manager()->protocol()->pauseFile(fi->fileNo(), true);
    return true;
}

bool FilesModel::resumeFile(const QByteArray &identifier)
{
    FileInfo *fi = d->fileInfo(identifier);
    if (!fi) return false;
    manager()->protocol()->pauseFile(fi->fileNo(), false);
    return true;
}

bool FilesModel::retryFile(const QByteArray &identifier)
{
    FileInfo *fi = d->fileInfo(identifier);
    if (!fi) return false;
    manager()->protocol()->retryFile(fi->fileNo());
    return true;
}

void FilesModel::updatedFiles()
{
    qDebug() << "FilesModel::updatedFiles()";
    d->m_updateTimer.stop();
    d->updateFiles();
    if (d->m_enableUpdates && d->m_updateTimer.interval() >= 0)
        d->m_updateTimer.start();
}

void FilesModel::updatedFile(int fileNo, DonkeyProtocol::FromCore reason)
{
    if (reason != DonkeyProtocol::File_downloaded && reason != DonkeyProtocol::File_info)
        return;
    d->addUpdateFile(fileNo);
}

void FilesModel::updateTimeout()
{
    d->m_updateTimer.stop();
    QSet<int> files = d->takeUpdateFileList();
    //qDebug() << "FilesModel::updateTimeout identifiers=" << files;
    Q_FOREACH(int fileNo, files) {
        QByteArray identifier = QByteArray::number(fileNo);
        QModelIndex index = d->indexFromIdentifier(identifier);
        if (index.isValid()) {
            emit d->dataChanged(index, index);
            emit fileChanged(identifier);
        } else {
            qDebug() << "FilesModel::updateTimeout Need to update all files";
            d->updateFiles();
            break; // job done
        }
    }
    if (d->m_enableUpdates && d->m_updateTimer.interval() >= 0)
        d->m_updateTimer.start();
}

void FilesModel::addedFile(int fileNo)
{
    QByteArray id = QByteArray::number(fileNo);
    if (d->identifiersToRows().contains(id)) {
        updatedFile(fileNo, DonkeyProtocol::File_info);
    } else {
        qDebug() << "FilesModel::addedFile(" << fileNo << ")";
        beginResetModel();
        d->removeUpdateFile(fileNo);
        QByteArray identifier = QByteArray::number(fileNo);
        d->addIdentifier(identifier);
        endResetModel();
        emit fileChanged(identifier);
    }
}

void FilesModel::removedFile(int fileNo)
{
    qDebug() << "FilesModel::removedFile(" << fileNo << ")";
    /*
    QModelIndex idx = d->indexFromIdentifier(QByteArray::number(fileNo));
    if (idx.isValid()) {
        d->removeRow(idx.row());
    } else {
        d->updateFiles();
    }
    */
    d->updateFiles();
}

bool FilesModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    FileInfo *leftFi = static_cast<FileInfo*>(left.internalPointer());
    FileInfo *rightFi = static_cast<FileInfo*>(right.internalPointer());

    QString leftState = d->data(left, StateValueRole).toString();
    QString rightState = d->data(right, StateValueRole).toString();
    if (leftState != rightState) {
        return leftState > rightState;
    }

    if (!leftFi->fileSize() || !rightFi->fileSize()) {
        if (leftFi->fileSize())
            return false;
        if (rightFi->fileSize())
            return true;
        return leftFi->fileName() < rightFi->fileName();
    }

    double leftProgress = ( leftFi->fileDownloaded() * 100.0 ) / leftFi->fileSize();
    double rightProgress = ( rightFi->fileDownloaded() * 100.0 ) / rightFi->fileSize();
    return leftProgress < rightProgress;
}
