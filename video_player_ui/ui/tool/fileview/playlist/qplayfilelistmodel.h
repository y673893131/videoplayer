#ifndef QPLAYFILELISTMODEL_H
#define QPLAYFILELISTMODEL_H

#include <QAbstractListModel>
#include <QThread>
#include <QSize>
#include <memory>
#include "video_pimpl.h"

struct file_info_t;

class QWorker : public QObject
{
    Q_OBJECT
public:
    QWorker(QObject* parnet);
    virtual ~QWorker();
signals:
    void finishWork(const QVector<std::shared_ptr<file_info_t>>&);
    // QThread interface
public slots:
    void run();
private:
    QThread* m_thread;
};

class QPlayFileListModelPrivate;
class QPlayFileListModel : public QAbstractListModel
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QPlayFileListModel)

public:
    enum play_mode
    {
        play_mode_local,
        play_mode_live,

        play_mode_max
    };

    enum user_role
    {
        role_play_mode = Qt::UserRole + 1,
        role_url
    };

public:
    bool isSelected(const QModelIndex& index) const;
    bool isOnline();
    QModelIndex findIndex(const QString&);
    QString current();
    QString title(const QString& sUrl);
signals:
    void urlflush();
    void liveflush();
    void removeUrl(const QString&);
public slots:
    void setMode(int);
    void setLocaleFiles(const QVector<std::shared_ptr<file_info_t>>&);
    void onEnd();
    void play(const QString&);
    void removeIndex(const QModelIndex&);
    void onInputUrlFile(const QString&);
    void onFilter(const QString&);
    void onClean();
public:
    explicit QPlayFileListModel(QObject *parent = nullptr);
    ~QPlayFileListModel() override;
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
private:
    Qt::ItemFlags flags(const QModelIndex &index) const override;
private:
    VP_DECLARE(QPlayFileListModel)
};

#endif // QPLAYFILELISTMODEL_H
