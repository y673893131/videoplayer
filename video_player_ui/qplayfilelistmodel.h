#ifndef QPLAYFILELISTMODEL_H
#define QPLAYFILELISTMODEL_H

#include <QAbstractListModel>
#include <QThread>

class QWorker : public QObject
{
    Q_OBJECT
public:
    QWorker(QObject* parnet);
    virtual ~QWorker();
signals:
    void finishWork(const QStringList&, const QStringList&);
    // QThread interface
public slots:
    void run();
private:
    QThread* m_thread;
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
public:
    bool isSelected(const QModelIndex& index) const;
    QModelIndex findIndex(const QString&);
signals:
    void urlflush();
    void liveflush();
    void removeUrl(const QString&);
public slots:
    void setMode(int);
    void setLocaleFiles(const QVector<QStringList>&);
    void play(const QString&);
    void removeIndex(const QModelIndex&);
    void onInputUrlFile(const QString&);
    void onFilter(const QString&);
public:
    explicit QPlayFileListModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
private:
    Qt::ItemFlags flags(const QModelIndex &index) const;
private:
    play_mode m_nPlayMode;
    QStringList m_urlNames, m_urls, m_localNames, m_locals;
    QStringList m_filterNames, m_filterUrls, m_filterLocalNames, m_filterLocalUrls;
    QString m_sFilter;
    QWorker* m_worker;
    QString m_playFile;
    friend class QWorker;
};

#endif // QPLAYFILELISTMODEL_H
