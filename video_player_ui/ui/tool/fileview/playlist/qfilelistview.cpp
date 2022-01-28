#include "qfilelistview.h"
#include "qplayfilelistmodel.h"
#include "config/qdatamodel.h"
#include "qfilelistdelegate.h"
#include "control/videocontrol.h"
#include "ui/tool/menu/qfilelistmenu.h"
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QAction>
#include <QDesktopServices>
#include <QFileInfo>
#include <QDir>
#include <QToolTip>
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
    m_menu = new QFileListMenu(this);
    setModel(model);
    delegate->setModel(model);
    setItemDelegate(delegate);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);

    auto control = VIDEO_CONTROL;
    connect(DATA(), &QDataModel::loadsuccessed, model, &QPlayFileListModel::setLocaleFiles);
    connect(DATA(), &QDataModel::addUrlSuccess, this, &QFileListView::onAddUrlSuccessed);
    connect(this, &QFileListView::addLocalUrl, DATA(), &QDataModel::onAddUrl);
    connect(DATA(), &QDataModel::addUrlSuccess, model, &QPlayFileListModel::play);
    connect(this, &QFileListView::select, model, &QPlayFileListModel::play);
    connect(this, &QFileListView::remove, model, &QPlayFileListModel::removeIndex);
    connect(model, &QPlayFileListModel::removeUrl, DATA(), &QDataModel::removeUrl);
    connect(this, &QFileListView::rightClick, this, &QFileListView::onPopMenu);
    connect(m_menu->actions()[QFileListMenu::action_open_dir], &QAction::triggered, this, &QFileListView::onBrowerFile);
    connect(m_menu->actions()[QFileListMenu::action_load_file], &QAction::triggered, this, &QFileListView::loadFile);
    connect(m_menu->actions()[QFileListMenu::action_clean_file_lsit], &QAction::triggered, model, &QPlayFileListModel::onClean);
    connect(m_menu->actions()[QFileListMenu::action_clean_file_lsit], &QAction::triggered, DATA(), &QDataModel::onClean);
    connect(m_menu->actions()[QFileListMenu::action_delete], &QAction::triggered, this, &QFileListView::onDeleteFile);
    connect(control, &QVideoControl::total, this, [=](int times){DATA()->onUpdateTimees(model->current(), times);});

    connect(this, &QFileListView::inputUrlFile, model, &QPlayFileListModel::onInputUrlFile);
    connect(this, &QFileListView::filter, model, &QPlayFileListModel::onFilter);
}

bool QFileListView::isVerticalUnder()
{
    return verticalScrollBar()->underMouse();
}

bool QFileListView::isMouseOver(const QModelIndex &index) const
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
    auto model = qobject_cast<QPlayFileListModel*>(this->model());
    auto isOnline = model->isOnline();

    m_menu->actions()[QFileListMenu::action_open_dir]->setVisible(index.isValid() && !isOnline);
    m_menu->actions()[QFileListMenu::action_delete]->setVisible(index.isValid() && !isOnline);
    m_menu->actions()[QFileListMenu::action_clean_file_lsit]->setVisible(!isOnline);

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

void QFileListView::onDeleteFile()
{
    auto index = indexAt(mapFromGlobal(m_menu->pos()));
    if(!index.isValid()) return;
    auto mdoel = qobject_cast<QPlayFileListModel*>(this->model());
    mdoel->removeIndex(index);
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
    auto rect = visualRect(index);
    if(delegate->inCloseArea(this, rect, pos))
        setCursor(QCursor(Qt::PointingHandCursor));
    else
        setCursor(QCursor(Qt::ArrowCursor));
    if(index.isValid())
    {
        update(index);
        QToolTip::showText(QCursor::pos(), index.data().toString());
    }
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
        if(delegate->inCloseArea(this, visualRect(index), pos))
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
