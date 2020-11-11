#include "qplayfilelistmodel.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtXmlPatterns/QXmlQuery>
#include <QDebug>
#include <QThread>
#include <QHeaderView>
#include <QTimer>
#include <QFile>
QWorker::QWorker(QObject* parent)
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
        int start = 0;
        int pos = 0;
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
            names.push_back(name);
            urls.push_back(url);
            pos = pos1;
            start = pos1;
        }

        response->close();
        net->deleteLater();
        emit finishWork(names, urls);
    });
    qDebug() << net->supportedSchemes() << QThread::currentThreadId();
    QNetworkRequest request;
    request.setUrl(QUrl("http://ivi.bupt.edu.cn/"));//北邮IVI
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.120 Safari/537.36");
    net->get(request);
}

bool QPlayFileListModel::isSelected(const QModelIndex &index) const
{
    return index.data(role_url).toString() == m_playFile;
}

QModelIndex QPlayFileListModel::findIndex(const QString &url)
{
    auto row = m_urls.indexOf(url);
    if(row >= 0)
        return index(row, 0);
    row = m_locals.indexOf(url);
    if(row >= 0)
        return index(row, 0);
    return QModelIndex();
}

void QPlayFileListModel::setMode(int mode)
{
    if(m_nPlayMode != mode)
    {
        m_nPlayMode = (play_mode)mode;
        emit layoutChanged();
    }
}

void QPlayFileListModel::setLocaleFiles(const QVector<QStringList> &file)
{
    m_localNames.clear();
    m_locals.clear();
    for(auto it : file)
    {
        m_localNames.push_back(it[0]);
        m_locals.push_back(it[1]);
    }

    onFilter(m_sFilter);
    emit layoutChanged();
}

void QPlayFileListModel::play(const QString &sFile)
{
    m_playFile = sFile;
}

void QPlayFileListModel::removeIndex(const QModelIndex &index)
{
    auto name = index.data().toString();
    auto url = index.data(role_url).toString();
    auto pos = m_urls.indexOf(url);
    emit removeUrl(url);
    if(pos >= 0)
    {
        m_urls.removeAt(pos);
        m_urlNames.removeAt(pos);
        emit layoutChanged();
        return;
    }

    pos = m_locals.indexOf(url);
    if(pos >= 0)
    {
        m_locals.removeAt(pos);
        m_localNames.removeAt(pos);
        emit layoutChanged();
    }
}

void QPlayFileListModel::onInputUrlFile(const QString &file)
{
    QStringList title, urls;
    QFile f(file);
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
            title.push_back(list[0]);
            urls.push_back(list[1]);
        }while (true);
        f.close();

        m_urlNames = title;
        m_urls = urls;
        onFilter(m_sFilter);
        emit layoutChanged();
    }
}

void QPlayFileListModel::onFilter(const QString &sFilter)
{
    m_sFilter = sFilter;
    QStringList titles, urls;
    int n = 0;
    for(auto& it : m_urlNames)
    {
        if(it.contains(sFilter, Qt::CaseInsensitive))
        {
            titles.push_back(it);
            urls.push_back(m_urls[n]);
        }

        ++n;
    }

    m_filterNames = titles;
    m_filterUrls = urls;

    titles.clear();
    urls.clear();
    n = 0;
    for(auto& it : m_localNames)
    {
        if(it.contains(sFilter, Qt::CaseInsensitive))
        {
            titles.push_back(it);
            urls.push_back(m_locals[n]);
        }

        ++n;
    }

    m_filterLocalNames = titles;
    m_filterLocalUrls = urls;

    emit layoutChanged();
}

QPlayFileListModel::QPlayFileListModel(QObject *parent)
    : QAbstractListModel(parent), m_nPlayMode(play_mode_local)
{
    m_worker = new QWorker(this);
//    connect(this, &QPlayFileListModel::liveflush, m_worker, &QWorker::run);
    connect(this, &QPlayFileListModel::liveflush, [=]{
        onInputUrlFile(":/res/Resources/iptv.urls");
    });

    connect(m_worker, &QWorker::finishWork, this, [this](const QStringList& names, const QStringList& urls)
    {
       m_urlNames = names;
       m_urls = urls;
       onFilter(m_sFilter);
       emit layoutChanged();
    }, Qt::ConnectionType::QueuedConnection);

    QTimer::singleShot(0, [this]{ liveflush();});
}

QVariant QPlayFileListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    return QVariant();
}

int QPlayFileListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    if(m_nPlayMode == play_mode_live)
        return m_filterNames.size();
    else
        return m_filterLocalNames.size();
}

QVariant QPlayFileListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case role_play_mode:
        return m_nPlayMode;
    case Qt::DisplayRole:
        if(m_nPlayMode == play_mode_live)
        {
            if(index.row() < m_filterNames.size())
                return m_filterNames[index.row()];
        }
        else
        {
            if(index.row() < m_filterLocalNames.size())
                return m_filterLocalNames[index.row()];
        }
        break;
    case role_url:
        if(m_nPlayMode == play_mode_live)
        {
            if(index.row() < m_filterUrls.size())
                return m_filterUrls[index.row()];
        }
        else
        {
            if(index.row() < m_filterLocalUrls.size())
                return m_filterLocalUrls[index.row()];
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
