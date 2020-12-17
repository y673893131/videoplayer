#include "qdouyuwidget.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>
#include <QKeyEvent>
#include "qlivelist.h"
#include "qlivelistmodel.h"
#include "qroomlistview.h"
#include "qroomlistviewmodel.h"
#include "qpagewidget.h"
QDouyuWidget::QDouyuWidget(QWidget *parent)
    :QFrameLessWidget(parent)
{
//    setAttribute(Qt::WA_ShowModal, true);
    installEventFilter(this);
    setObjectName("douyuWd");
    auto widget = new QWidget(this);
    widget->setObjectName("douyuWd");

    auto titleWidget = new QWidget(widget);
    titleWidget->setObjectName("douyuTitleWd");

    auto close = new QPushButton("", titleWidget);
    close->setObjectName("btn_close");

    auto contentWidget = new QWidget(widget);
    contentWidget->setObjectName("douyuContentWd");

    auto liveList = new QLiveList(contentWidget);
    auto roomlist = new QRoomListView(contentWidget);

    auto search = new QLineEdit(contentWidget);
    search->setObjectName("room_search");
    search->setPlaceholderText(tr("search room"));

    auto page = new QPageWidget(contentWidget);

    auto layoutThis = new QVBoxLayout(this);
    layoutThis->setMargin(0);
    layoutThis->setSpacing(0);
    layoutThis->addWidget(widget);

    auto layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(titleWidget, 0, Qt::AlignTop);
    layout->addWidget(contentWidget);

    auto layoutLeft = new QVBoxLayout;
    layoutLeft->setMargin(0);
    layoutLeft->addWidget(liveList, 0, Qt::AlignHCenter);

    auto layoutRightBottom = new QHBoxLayout;
    layoutRightBottom->addWidget(search);
    layoutRightBottom->addWidget(page);

    auto layoutRight = new QVBoxLayout;
    layoutRight->setMargin(0);
    layoutRight->addWidget(roomlist);
    layoutRight->addLayout(layoutRightBottom);

    auto titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->addStretch();
    titleLayout->addWidget(close);

    auto layoutContent = new QHBoxLayout(contentWidget);
    layoutContent->setMargin(5);

    layoutContent->addLayout(layoutLeft, 1);
    layoutContent->addLayout(layoutRight, 4);

    connect(close, &QPushButton::clicked, this, &QFrameLessWidget::hide);

    auto modelPlatform = (QLiveListModel*)liveList->model();
    auto modelRoom = (QRoomListViewModel*)roomlist->model();
    modelRoom->setRoomManager(modelPlatform->getRoomManager());
    m_timer = new QTimer();
    m_timer->setInterval(300);
    m_timer->setSingleShot(true);

    connect(modelPlatform, &QLiveListModel::loadRoomsInfo, modelRoom, &QRoomListViewModel::appendRooms);
    connect(modelPlatform, &QLiveListModel::loadRoomsInfo, roomlist, &QRoomListView::scrollToTop);
    connect(roomlist, &QRoomListView::doubleClicked, modelPlatform, &QLiveListModel::selectRoom);
    connect(modelPlatform, &QLiveListModel::play, this, &QDouyuWidget::play);
    connect(search, &QLineEdit::returnPressed, roomlist, [=]{roomlist->go(search->text());});
    connect(roomlist, &QRoomListView::doubleClicked, this, &QDouyuWidget::hide);
    connect(modelPlatform, &QLiveListModel::resetPage, page, &QPageWidget::setTotalCount);
    connect(modelPlatform, &QLiveListModel::loadRoomsInfo, page, &QPageWidget::setPageCount);
//    connect(page, &QPageWidget::pageChanged, modelPlatform, &QLiveListModel::onPageChanged);


    connect(m_timer, &QTimer::timeout, this, [=]{ modelPlatform->onPageChanged(page->curentPage()); });
    connect(page, &QPageWidget::pageChanged, this, [=](int){ m_timer->start(); });
    connect(this, &QDouyuWidget::showIndex, modelPlatform, [=](int type)
    {
        liveList->setCurrentIndex(QModelIndex());
        modelPlatform->setCurrentType(type);
        modelRoom->setRoomManager(modelPlatform->curData()->roomManager());
        show();
    });
}

bool QDouyuWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this)
    {
        if(event->type() == QEvent::KeyRelease)
        {
            auto keyEvent = reinterpret_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_F5)
            {
                m_timer->start();
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
