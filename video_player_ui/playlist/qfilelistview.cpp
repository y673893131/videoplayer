#include "qfilelistview.h"
#include "qplayfilelistmodel.h"
#include "config/qdatamodel.h"
#include "qfilelistdelegate.h"
#include "ui/tool/menu/qplaymenubase.h"
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QAction>
#include <QDesktopServices>
#include <QFileInfo>
#include <QDir>
#include "control/videocontrol.h"
#ifdef Q_OS_WIN
#include <Windows.h>
#include <ShlObj.h>
#endif

QFileListView::QFileListView(QWidget* parent)
    :QListView(parent)
{
    setObjectName("list_file");
    verticalScrollBar()->setObjectName("scroll1");
    horizontalScrollBar()->setObjectName("scroll1");
    auto model = new QPlayFileListModel(this);
    auto delegate = new QFileListDelegate(this);
    m_menu = new QPlayMenuBase(this);
    m_openDir = m_menu->addAction(tr("open dir"));
    m_loadFile = m_menu->addAction(tr("load file"));
    setModel(model);
    delegate->setModel(model);
    setItemDelegate(delegate);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setMouseTracking(true);

    connect(DATA(), &QDataModel::loadsuccessed, model, &QPlayFileListModel::setLocaleFiles);
    connect(DATA(), &QDataModel::addUrlSuccess, this, &QFileListView::onAddUrlSuccessed);
    connect(this, &QFileListView::addLocalUrl, DATA(), &QDataModel::onAddUrl);
    connect(this, &QFileListView::addLocalUrl, model, &QPlayFileListModel::play);
    connect(this, &QFileListView::select, model, &QPlayFileListModel::play);
    connect(this, &QFileListView::remove, model, &QPlayFileListModel::removeIndex);
    connect(model, &QPlayFileListModel::removeUrl, DATA(), &QDataModel::removeUrl);
    connect(this, &QFileListView::rightClick, this, &QFileListView::onPopMenu);
    connect(m_openDir, &QAction::triggered, this, &QFileListView::onBrowerFile);
    connect(m_loadFile, &QAction::triggered, this, &QFileListView::loadFile);

    connect(this, &QFileListView::inputUrlFile, model, &QPlayFileListModel::onInputUrlFile);
    connect(this, &QFileListView::filter, model, &QPlayFileListModel::onFilter);
}

bool QFileListView::isVerticalUnder()
{
    return verticalScrollBar()->underMouse();
}

bool QFileListView::isMouseOver(const QModelIndex &index)
{
    auto indexMouse = indexAt(mapFromGlobal(QCursor::pos()));
    return indexMouse == index;
}

void QFileListView::onAddUrlSuccessed(const QString &url)
{
    auto model = qobject_cast<QPlayFileListModel*>(this->model());
    setCurrentIndex(model->findIndex(url));
}

void QFileListView::onPopMenu(const QPoint &point)
{
    auto index = indexAt(point);
    m_openDir->setVisible(index.isValid());
    m_loadFile->setVisible(!index.isValid());
    m_menu->popup(QCursor::pos());
    setFocus();
}

void QFileListView::onBrowerFile()
{
    auto index = indexAt(mapFromGlobal(m_menu->pos()));
    if(!index.isValid()) return;
    auto dir = index.data(QPlayFileListModel::role_url).toString();
#ifdef Q_OS_WIN
    auto winPath = dir.replace("/", "\\");
    CoInitialize(nullptr);
    auto pidl = ILCreateFromPath(winPath.toStdWString().data());
    if (pidl)
    {
        SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
        ILFree(pidl);
    }
    CoUninitialize();
#else
    QFileInfo info(dir);
    auto path = info.dir().path();
    QDesktopServices::openUrl(path);
#endif
}

void QFileListView::onEnd()
{
    auto model = qobject_cast<QPlayFileListModel*>(this->model());
    model->onEnd();
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
    auto delegate = reinterpret_cast<QFileListDelegate*>(itemDelegate());
    if(delegate->inCloseArea(visualRect(index), pos))
        setCursor(QCursor(Qt::PointingHandCursor));
    else
        setCursor(QCursor(Qt::ArrowCursor));
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
        auto delegate = reinterpret_cast<QFileListDelegate*>(itemDelegate());
        if(delegate->inCloseArea(visualRect(index), pos))
        {
            qDebug() << "remove" << index;
            emit remove(index);
            event->ignore();
        }
    }
    else
    {
        return;
    }
}
