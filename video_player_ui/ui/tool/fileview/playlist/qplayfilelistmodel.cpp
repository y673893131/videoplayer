#include "qplayfilelistmodel.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtXmlPatterns/QXmlQuery>
#include <QDebug>
#include <QThread>
#include <QHeaderView>
#include <QTimer>
#include <QFile>
#include "framelesswidget/util.h"
#include "config/configDef.h"

QWorker::QWorker(QObject* /*parent*/)
    :QObject()
{
    m_thread = new QThread();
    moveToThread(m_thread);
    m_thread->start();
}

QWorker::~QWorker()
{
    m_thread->exit();
    m_thread->wait();
}

void QWorker::run()
{
    auto net = new QNetworkAccessManager();
    connect(net, &QNetworkAccessManager::finished, [this, net](QNetworkReply* response)
    {
        QStringList names,urls;
        qDebug() << "recv: "<< response->error();
        qDebug() << "code: "<< response->attribute(QNetworkRequest::Attribute::HttpStatusCodeAttribute).toInt();
        auto data = QString(response->readAll());
//        response->close();
//        net->deleteLater();
//        return;
        int start = 0;
        int pos = 0;
        QVector<std::shared_ptr<file_info_t>> datas;
        while(1)
        {
            pos = data.indexOf("<p>", start);
            if(pos < 0) break;
            pos += 3;
            auto pos1 = data.indexOf("</p>", pos);
            auto name = data.mid(pos, pos1 - pos);
            pos = data.indexOf("href=\"", pos1);
            if(pos < 0) break;
            pos = data.indexOf("href=\"", pos + 1);
            if(pos < 0) break;
            pos += 6;
            pos1 = data.indexOf("\"", pos);
            if(pos1 < 0) break;
            auto url = "http://ivi.bupt.edu.cn" + data.mid(pos, pos1 - pos);

            auto t = std::make_shared<file_info_t>();
            t->name = name;
            t->url = url;
            datas.push_back(t);
            pos = pos1;
            start = pos1;
        }

        response->close();
        net->deleteLater();
        emit finishWork(datas);
    });

    QNetworkRequest request;
    request.setUrl(QUrl("http://ivi.bupt.edu.cn/"));//北邮IVI
//    request.setUrl(QUrl("https://www.baidu.com"));
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.120 Safari/537.36");
    net->get(request);
}

class QPlayFileListModelPrivate : public VP_Data<QPlayFileListModel>
{
    VP_DECLARE_PUBLIC(QPlayFileListModel)
    inline QPlayFileListModelPrivate(QPlayFileListModel* parent)
        : VP_Data(parent)
        , m_nPlayMode(QPlayFileListModel::play_mode_local)
    {
    }

    bool remove(const QString&, QVector<std::shared_ptr<file_info_t>>& datas);
    void filter(const QString&, const QVector<std::shared_ptr<file_info_t>>& datas, QVector<std::shared_ptr<file_info_t>>& filters);
    QModelIndex findIndex(const QString&, const QVector<std::shared_ptr<file_info_t>>& datas);
private:
    QPlayFileListModel::play_mode m_nPlayMode;

    QVector<std::shared_ptr<file_info_t>> m_datas[QPlayFileListModel::play_mode_max];
    QVector<std::shared_ptr<file_info_t>> m_filters[QPlayFileListModel::play_mode_max];

    QString m_sFilter;
    QWorker* m_worker;
    QString m_playFile;
    QSize m_itemSize;
    friend class QWorker;
};

bool QPlayFileListModelPrivate::remove(const QString &sUrl, QVector<std::shared_ptr<file_info_t>> &datas)
{
    int row = 0;
    for(QVector<std::shared_ptr<file_info_t>>::iterator it = datas.begin();
        it != datas.end(); ++it)
    {
        if((*it)->url == sUrl)
        {
            datas.remove(row);
            return true;
        }

        ++row;
    }

    return false;
}

void QPlayFileListModelPrivate::filter(const QString & sFilter, const  QVector<std::shared_ptr<file_info_t>> &datas,  QVector<std::shared_ptr<file_info_t>>& filters)
{
    filters.clear();
    for(auto it = datas.constBegin();
        it != datas.constEnd(); ++it)
    {
        if((*it)->name.contains(sFilter, Qt::CaseInsensitive))
        {
            filters.push_back(*it);
        }
    }
}

QModelIndex QPlayFileListModelPrivate::findIndex(const QString & sUrl, const QVector<std::shared_ptr<file_info_t>> &datas)
{
    VP_Q(QPlayFileListModel);
    int row = 0;
    for(auto it = datas.constBegin();
        it != datas.constEnd(); ++it)
    {
        if((*it)->url == sUrl)
            return q->index(row, 0);
        ++row;
    }

    return QModelIndex();
}

QPlayFileListModel::QPlayFileListModel(QObject *parent)
    : QAbstractListModel(parent)
    , VP_INIT(new QPlayFileListModelPrivate(this))
{
    VP_D(QPlayFileListModel);
    d->m_itemSize = CALC_WIDGET_SIZE(nullptr, 200, 20);
    d->m_worker = new QWorker(this);
//    connect(this, &QPlayFileListModel::liveflush, m_worker, &QWorker::run);
    connect(this, &QPlayFileListModel::liveflush, [=]{
        QNetworkAccessManager net;
//        qDebug() << net.supportedSchemes();
        onInputUrlFile(":/url/iptv");
    });

    connect(d->m_worker, &QWorker::finishWork, this, [this](const QVector<std::shared_ptr<file_info_t>>& datas)
    {
        VP_D(QPlayFileListModel);
        d->m_datas[play_mode_live] = datas;
        onFilter(d->m_sFilter);
        emit layoutChanged();
    }, Qt::ConnectionType::QueuedConnection);

    QTimer::singleShot(0, [this]{ liveflush();});
}

QPlayFileListModel::~QPlayFileListModel()
{
}

bool QPlayFileListModel::isSelected(const QModelIndex &index) const
{
    VP_D(const QPlayFileListModel);
    return index.data(role_url).toString() == d->m_playFile;
}

bool QPlayFileListModel::isOnline()
{
    VP_D(QPlayFileListModel);
    return d->m_nPlayMode == play_mode_live;
}

QModelIndex QPlayFileListModel::findIndex(const QString &url)
{
    VP_D(QPlayFileListModel);
    auto index = d->findIndex(url, d->m_datas[play_mode_live]);
    if(index.isValid())
        return index;
    return d->findIndex(url, d->m_datas[play_mode_local]);
}

QString QPlayFileListModel::current()
{
    VP_D(QPlayFileListModel);
    return d->m_playFile;
}

QString QPlayFileListModel::title(const QString& sUrl)
{
    VP_D(QPlayFileListModel);
    auto index = d->findIndex(sUrl, d->m_datas[play_mode_live]);
    if(index.isValid())
        return index.data().toString();
    return QString();
}

void QPlayFileListModel::setMode(int mode)
{
    VP_D(QPlayFileListModel);
    if(d->m_nPlayMode != mode)
    {
        d->m_nPlayMode = static_cast<QPlayFileListModel::play_mode>(mode);
        emit layoutChanged();
    }
}

void QPlayFileListModel::setLocaleFiles(const QVector<std::shared_ptr<file_info_t>> &fileInfos)
{
    VP_D(QPlayFileListModel);
    d->m_datas[play_mode_local] = fileInfos;
    onFilter(d->m_sFilter);
    emit layoutChanged();
}

void QPlayFileListModel::onEnd()
{
    play("");
}

void QPlayFileListModel::play(const QString &sFile)
{
    VP_D(QPlayFileListModel);
    d->m_playFile = sFile;
    emit layoutChanged();
}

void QPlayFileListModel::removeIndex(const QModelIndex &index)
{
    VP_D(QPlayFileListModel);
    auto name = index.data().toString();
    auto url = index.data(role_url).toString();
    emit removeUrl(url);
    if(d->remove(url, d->m_datas[play_mode_live]) || d->remove(url, d->m_datas[play_mode_local]))
    {
        onFilter(d->m_sFilter);
        emit layoutChanged();
    }
}

void QPlayFileListModel::onInputUrlFile(const QString &file)
{
    VP_D(QPlayFileListModel);
    d->m_datas[play_mode_live].clear();
    QFile f(file);
    QStringList title;
    if(f.open(QFile::ReadOnly))
    {
        QByteArray arr;
        do
        {
            arr = f.readLine();
            if(arr.isEmpty())
                break;
            auto line = QString::fromUtf8(arr);
            auto list = line.split(',');
            if(list.size() != 2)
                continue;
            if(title.contains(list[0]))
                continue;

            auto t = std::make_shared<file_info_t>();
            t->name = list[0];
            t->url = list[1].replace("\r\n", "");
            title.push_back(t->name);
            d->m_datas[play_mode_live].push_back(t);
        }while (true);
        f.close();

        onFilter(d->m_sFilter);
        emit layoutChanged();
    }
}

void QPlayFileListModel::onFilter(const QString &sFilter)
{
    VP_D(QPlayFileListModel);
    d->m_sFilter = sFilter;
    d->filter(sFilter, d->m_datas[play_mode_live], d->m_filters[play_mode_live]);
    d->filter(sFilter, d->m_datas[play_mode_local], d->m_filters[play_mode_local]);
    emit layoutChanged();
}

void QPlayFileListModel::onClean()
{
    VP_D(QPlayFileListModel);
    d->m_datas[play_mode_local].clear();
    onFilter(d->m_sFilter);
    emit layoutChanged();
}

QVariant QPlayFileListModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
    return QVariant();
}

int QPlayFileListModel::rowCount(const QModelIndex &parent) const
{
    VP_D(const QPlayFileListModel);
    if (parent.isValid())
        return 0;

    return d->m_filters[d->m_nPlayMode].size();
}

QVariant QPlayFileListModel::data(const QModelIndex &index, int role) const
{
    VP_D(const QPlayFileListModel);
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case role_play_mode:
        return d->m_nPlayMode;
    case Qt::DisplayRole:
        if(index.row() < d->m_filters[d->m_nPlayMode].size())
        {
            return d->m_filters[d->m_nPlayMode][index.row()]->name;
        }
        break;
//    case Qt::SizeHintRole:
//        return m_itemSize;
    case role_url:
        if(index.row() < d->m_filters[d->m_nPlayMode].size())
        {
            return d->m_filters[d->m_nPlayMode][index.row()]->url;
        }
        break;
    }

    return QVariant();
}


Qt::ItemFlags QPlayFileListModel::flags(const QModelIndex &index) const
{
    if(isSelected(index))
        return QAbstractListModel::flags(index) | Qt::ItemIsSelectable;
    else
        return QAbstractListModel::flags(index) & ~Qt::ItemIsSelectable;
}
