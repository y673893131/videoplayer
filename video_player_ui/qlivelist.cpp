#include "qlivelist.h"
#include "qlivelistmodel.h"
#include <QHeaderView>
#include <QTimer>
#include <QScrollBar>
QLiveList::QLiveList(QWidget* parent)
    :QTreeView(parent)
{
    setObjectName("platform_tree");
    header()->hide();
    setRootIsDecorated(false);
    verticalScrollBar()->setObjectName("scroll1");
    horizontalScrollBar()->setObjectName("scroll1");
    auto model = new QLiveListModel(this);
    setModel(model);

    auto timer = new QTimer(this);
    timer->setInterval(300);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, model, [=]{ model->selectGame(currentIndex()); });
    connect(this, &QLiveList::select, timer, [timer](const QModelIndex&){ timer->start(); });
}

void QLiveList::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    emit select(current);
}
