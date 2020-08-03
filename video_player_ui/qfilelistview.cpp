#include "qfilelistview.h"
#include "qplayfilelistmodel.h"
#include "qdatamodel.h"
#include "qfilelistdelegate.h"
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QFileInfo>
#include <QDir>
QFileListView::QFileListView(QWidget* parent)
    :QListView(parent)
{
    setObjectName("list_file");
    verticalScrollBar()->setObjectName("scroll1");
    horizontalScrollBar()->setObjectName("scroll1");
    auto model = new QPlayFileListModel(this);
    auto data = new QDataModel(this);
    auto delegate = new QFileListDelegate(this);
    auto menu = new QMenu(this);
    menu->setObjectName("menu1");
    auto openDir = menu->addAction(tr("open dir"));
    auto loadFile = menu->addAction(tr("load file"));
    setModel(model);
    delegate->setModel(model);
    setItemDelegate(delegate);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setMouseTracking(true);

    connect(data, &QDataModel::loadsuccessed, model, &QPlayFileListModel::setLocaleFiles);
    connect(data, &QDataModel::addUrlSuccess, [this, model](const QString& url)
    {
        setCurrentIndex(model->findIndex(url));
    });
    connect(this, &QFileListView::addLocalUrl, data, &QDataModel::onAddUrl);
    connect(this, &QFileListView::addLocalUrl, model, &QPlayFileListModel::play);
    connect(this, &QFileListView::remove, model, &QPlayFileListModel::removeIndex);
    connect(model, &QPlayFileListModel::removeUrl, data, &QDataModel::removeUrl);
    connect(this, &QFileListView::rightClick, [this, menu, openDir, loadFile](const QPoint& pt)
    {
        auto index = indexAt(pt);
        openDir->setVisible(index.isValid());
        loadFile->setVisible(!index.isValid());
        menu->popup(QCursor::pos());
        this->setFocus();
    });
    connect(openDir, &QAction::triggered, [this, menu]
    {
        auto index = indexAt(mapFromGlobal(menu->pos()));
        if(!index.isValid()) return;
        auto dir = index.data(QPlayFileListModel::role_url).toString();
        QFileInfo info(dir);
        QDesktopServices::openUrl(info.dir().path());
    });

    connect(loadFile, &QAction::triggered, this, &QFileListView::loadFile);
}

bool QFileListView::isVerticalUnder()
{
    return verticalScrollBar()->underMouse();
}

void QFileListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    auto index = indexAt(event->pos());
    if(!index.isValid()) return;

    if (event->button() == Qt::LeftButton)
    {
        setCurrentIndex(QModelIndex());
        setCurrentIndex(index);
        auto url = index.data(QPlayFileListModel::role_url).toString();
        emit select(url);
    }
}

void QFileListView::mouseMoveEvent(QMouseEvent *event)
{
    auto pos = event->pos();
    auto index = indexAt(pos);
    if(((QFileListDelegate*)itemDelegate())->inCloseArea(visualRect(index), pos))
        setCursor(QCursor(Qt::PointingHandCursor));
    else
        setCursor(QCursor(Qt::ArrowCursor));
    if(index.isValid())
        update(index);
    QListView::mouseMoveEvent(event);
}

void QFileListView::mousePressEvent(QMouseEvent *event)
{
    auto pos = event->pos();
    auto index = indexAt(pos);
    if(event->buttons() & Qt::RightButton)
        emit rightClick(pos);
    if(!index.isValid()) return;
    if(event->buttons() & Qt::LeftButton)
    {
        if(((QFileListDelegate*)itemDelegate())->inCloseArea(visualRect(index), pos))
            emit remove(index);
    }
    else
    {
        return;
    }

//    QListView::mousePressEvent(event);
}


void QFileListView::currentChanged(const QModelIndex &/*current*/, const QModelIndex &/*previous*/)
{
//    qDebug() << previous << "->" << current;
//    if(current.isValid())
//    {
//        auto url = current.data(QPlayFileListModel::role_url).toString();
//        emit select(url);
//    }
}
