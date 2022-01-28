#ifndef QPLAYFILELISTMODEL_H
#define QPLAYFILELISTMODEL_H

#include <QAbstractListModel>
#include <QThread>
#include <QSize>
#include "config/qdatamodel.h"

class QWorker : public QObject
{
    Q_OBJECT
public:
    QWorker(QObject* parnet);
    virtual ~QWorker();
signals:
    void finishWork(const QVector<file_info_t>&);
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
        play_mode_live,

        play_mode_max
    };
public:
    bool isSelected(const QModelIndex& index) const;
    bool isOnline();
    QModelIndex findIndex(const QString&);
    QString current();
signals:
    void urlflush();
    void liveflush();
    void removeUrl(const QString&);
public slots:
    void setMode(int);
    void setLocaleFiles(const QVector<file_info_t>&);
    void onEnd();
    void play(const QString&);
    void removeIndex(const QModelIndex&);
    void onInputUrlFile(const QString&);
    void onFilter(const QString&);
    void onClean();
public:
    explicit QPlayFileListModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
private:
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex findIndex(const QString&, const QVector<file_info_t>& datas);
    bool remove(const QString&, QVector<file_info_t>& datas);
    void filter(const QString&, const QVector<file_info_t>& datas, QVector<file_info_t>& filters);
private:
    play_mode m_nPlayMode;

    QVector<file_info_t> m_datas[play_mode_max];
    QVector<file_info_t> m_filters[play_mode_max];

    QString m_sFilter;
    QWorker* m_worker;
    QString m_playFile;
    QSize m_itemSize;
    friend class QWorker;
};

#endif // QPLAYFILELISTMODEL_H
