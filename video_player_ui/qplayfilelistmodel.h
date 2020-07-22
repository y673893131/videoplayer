#ifndef QPLAYFILELISTMODEL_H
#define QPLAYFILELISTMODEL_H

#include <QAbstractListModel>
#include <QThread>

class QNetworkReply;
class QWorker : public QThread
{
    Q_OBJECT
public:
    QWorker(QObject* parnet, QNetworkReply* response);
    virtual ~QWorker();
signals:
    void finish();
    // QThread interface
protected:
    void run();
private:
    QNetworkReply* m_response;
};

class QPlayFileListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum user_role
    {
        role_play_mode = Qt::UserRole + 1,
        role_url
    };

    enum play_mode
    {
        play_mode_local,
        play_mode_live
    };
signals:
    void urlflush();
public slots:
    void setMode(int);
    void setLocaleFiles(const QVector<QStringList>&);
public:
    explicit QPlayFileListModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void flush();
private:
    play_mode m_nPlayMode;
    QStringList m_urlNames, m_urls, m_localNames, m_locals;
    friend class QWorker;
};

#endif // QPLAYFILELISTMODEL_H
