#include "qfilelistview.h"
#include "qplayfilelistmodel.h"
#include "qdatamodel.h"
#include "qfilelistdelegate.h"
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
QFileListView::QFileListView(QWidget* parent)
    :QListView(parent)
{
    setObjectName("list_file");
    verticalScrollBar()->setObjectName("scroll1");
    auto model = new QPlayFileListModel(this);
    auto data = new QDataModel(this);
    model->flush();

    auto delegate = new QFileListDelegate(this);
    setModel(model);
    setItemDelegate(delegate);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setMouseTracking(true);

    connect(data, &QDataModel::loadsuccessed, model, &QPlayFileListModel::setLocaleFiles);
    connect(this, &QFileListView::addLocalUrl, data, &QDataModel::onAddUrl);
    data->init();
}

bool QFileListView::isVerticalUnder()
{
    return verticalScrollBar()->underMouse();
}

void QFileListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    setCurrentIndex(QModelIndex());
    QListView::mousePressEvent(event);
    QModelIndex index = currentIndex();
    if (!index.isValid())
        return;
    if (event->button() == Qt::LeftButton)
    {
        auto url = index.data(QPlayFileListModel::role_url).toString();
        emit select(url);
    }
}
