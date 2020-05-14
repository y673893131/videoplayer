#include "qplayfilelistmodel.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtXmlPatterns/QXmlQuery>
#include <QDebug>
#include <QThread>

QWorker::QWorker(QObject* parent, QNetworkReply* response)
    :QThread(parent), m_response(response)
{
}

void QWorker::run()
{
    auto obj = qobject_cast<QPlayFileListModel*>(parent());
    obj->m_urlNames.clear();
    obj->m_urls.clear();
    qDebug() << "recv: "<< m_response->error();
    qDebug() << "code: "<< m_response->attribute(QNetworkRequest::Attribute::HttpStatusCodeAttribute).toInt();
    auto data = QString(m_response->readAll());
    int start = 0;
    int pos = 0;
    while(1){
        pos = data.indexOf("<p>", start);
        if(pos < 0) break;
        pos += 3;
        auto pos1 = data.indexOf("</p>", pos);
        auto name = data.mid(pos, pos1 - pos);
        obj->m_urlNames.push_back(name);
        pos = pos1;
        start = pos1;
    }

    m_response->close();
    emit finish();
    this->deleteLater();
}

void QPlayFileListModel::setMode(int mode)
{
    if(m_nPlayMode != mode){
        m_nPlayMode = (play_mode)mode;
        emit layoutChanged();
    }
}

QPlayFileListModel::QPlayFileListModel(QObject *parent)
    : QAbstractListModel(parent), m_nPlayMode(play_mode_local)
{
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
        return m_urlNames.size();
    else
        return m_localNames.size();
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
            return m_urlNames[index.row()];
        else
            return m_localNames[index.row()];
    }
    // FIXME: Implement me!
    return QVariant();
}

void QPlayFileListModel::flush()
{
    static auto manager = new QNetworkAccessManager(this);
    qDebug() << manager->supportedSchemes();
    QNetworkRequest request;
    request.setUrl(QUrl("http://ivi.bupt.edu.cn/"));
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.120 Safari/537.36");
    auto response = manager->get(request);
    connect(manager,&QNetworkAccessManager::finished, [response, this]{
        QWorker* worker = new QWorker(this, response);
        worker->start();
        connect(worker, &QWorker::finish, this, [this]
        {
           emit layoutChanged();
        }, Qt::ConnectionType::QueuedConnection);
    });
}
