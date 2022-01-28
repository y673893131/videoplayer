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
        QVector<file_info_t> datas;
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

            file_info_t t;
            t.name = name;
            t.url = url;
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

bool QPlayFileListModel::isSelected(const QModelIndex &index) const
{
    return index.data(role_url).toString() == m_playFile;
}

bool QPlayFileListModel::isOnline()
{
    return m_nPlayMode == play_mode_live;
}

QModelIndex QPlayFileListModel::findIndex(const QString &url)
{
    auto index = findIndex(url, m_datas[play_mode_live]);
    if(index.isValid())
        return index;
    return findIndex(url, m_datas[play_mode_local]);
}

QString QPlayFileListModel::current()
{
    return m_playFile;
}

void QPlayFileListModel::setMode(int mode)
{
    if(m_nPlayMode != mode)
    {
        m_nPlayMode = static_cast<play_mode>(mode);
        emit layoutChanged();
    }
}

void QPlayFileListModel::setLocaleFiles(const QVector<file_info_t> &fileInfos)
{
    m_datas[play_mode_local] = fileInfos;
    onFilter(m_sFilter);
    emit layoutChanged();
}

void QPlayFileListModel::onEnd()
{
    play("");
}

void QPlayFileListModel::play(const QString &sFile)
{
    m_playFile = sFile;
    emit layoutChanged();
}

void QPlayFileListModel::removeIndex(const QModelIndex &index)
{
    auto name = index.data().toString();
    auto url = index.data(role_url).toString();
    emit removeUrl(url);
    if(remove(url, m_datas[play_mode_live]) || remove(url, m_datas[play_mode_local]))
    {
        onFilter(m_sFilter);
        emit layoutChanged();
    }
}

void QPlayFileListModel::onInputUrlFile(const QString &file)
{
    m_datas[play_mode_live].clear();
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

            file_info_t t;
            t.name = list[0];
            t.url = list[1].replace("\r\n", "");
            title.push_back(t.name);
            m_datas[play_mode_live].push_back(t);
        }while (true);
        f.close();

        onFilter(m_sFilter);
        emit layoutChanged();
    }
}

void QPlayFileListModel::onFilter(const QString &sFilter)
{
    m_sFilter = sFilter;
    filter(sFilter, m_datas[play_mode_live], m_filters[play_mode_live]);
    filter(sFilter, m_datas[play_mode_local], m_filters[play_mode_local]);
    emit layoutChanged();
}

void QPlayFileListModel::onClean()
{
    m_datas[play_mode_local].clear();
    onFilter(m_sFilter);
    emit layoutChanged();
}

QPlayFileListModel::QPlayFileListModel(QObject *parent)
    : QAbstractListModel(parent), m_nPlayMode(play_mode_local)
{
    m_itemSize = CALC_WIDGET_SIZE(nullptr, 200, 20);
    m_worker = new QWorker(this);
//    connect(this, &QPlayFileListModel::liveflush, m_worker, &QWorker::run);
    connect(this, &QPlayFileListModel::liveflush, [=]{
        QNetworkAccessManager net;
//        qDebug() << net.supportedSchemes();
        onInputUrlFile(":/url/iptv");
    });

    connect(m_worker, &QWorker::finishWork, this, [this](const QVector<file_info_t>& datas)
    {
        m_datas[play_mode_live] = datas;
        onFilter(m_sFilter);
        emit layoutChanged();
    }, Qt::ConnectionType::QueuedConnection);

    QTimer::singleShot(0, [this]{ liveflush();});
}

QVariant QPlayFileListModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
    return QVariant();
}

int QPlayFileListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_filters[m_nPlayMode].size();
}

QVariant QPlayFileListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case role_play_mode:
        return m_nPlayMode;
    case Qt::DisplayRole:
        if(index.row() < m_filters[m_nPlayMode].size())
        {
            return m_filters[m_nPlayMode][index.row()].name;
        }
        break;
//    case Qt::SizeHintRole:
//        return m_itemSize;
    case role_url:
        if(index.row() < m_filters[m_nPlayMode].size())
        {
            return m_filters[m_nPlayMode][index.row()].url;
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

QModelIndex QPlayFileListModel::findIndex(const QString & sUrl, const QVector<file_info_t> &datas)
{
    int row = 0;
    for(QVector<file_info_t>::const_iterator it = datas.constBegin();
        it != datas.constEnd(); ++it)
    {
        if(it->url == sUrl)
            return index(row, 0);
        ++row;
    }

    return QModelIndex();
}

bool QPlayFileListModel::remove(const QString &sUrl, QVector<file_info_t> &datas)
{
    int row = 0;
    for(QVector<file_info_t>::iterator it = datas.begin();
        it != datas.end(); ++it)
    {
        if(it->url == sUrl)
        {
            datas.remove(row);
            return true;
        }

        ++row;
    }

    return false;
}

void QPlayFileListModel::filter(const QString & sFilter, const QVector<file_info_t> &datas, QVector<file_info_t>& filters)
{
    filters.clear();
    for(QVector<file_info_t>::const_iterator it = datas.constBegin();
        it != datas.constEnd(); ++it)
    {
        if(it->name.contains(sFilter, Qt::CaseInsensitive))
        {
            filters.push_back(*it);
        }
    }
}
