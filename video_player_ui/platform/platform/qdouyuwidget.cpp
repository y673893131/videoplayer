#include "qdouyuwidget.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>
#include <QKeyEvent>
#include <QDebug>
#include "platform/platform/qlivelist.h"
#include "platform/platform/qlivelistmodel.h"
#include "platform/room/qroomlistview.h"
#include "platform/room/qroomlistviewmodel.h"
#include "platform/room/qpagewidget.h"
QDouyuWidget::QDouyuWidget(QWidget *parent)
    :QFrameLessWidget(parent)
{
    init(parent);
}

void QDouyuWidget::init(QWidget *parent)
{
    initStyle();
    initUi(parent);
}

void QDouyuWidget::initStyle()
{
    setAttribute(Qt::WA_ShowModal, true);
    setDragSelf(true);
    setDoubleClickMax();
    auto hwnd = reinterpret_cast<HWND>(this->winId());
    setAreo(hwnd);
    setShadow(hwnd);
    installEventFilter(this);
}

void QDouyuWidget::initUi(QWidget */*parent*/)
{
    setObjectName("douyuWd");
    auto widget = this;
    m_title = new QWidget(widget);
    m_title->setObjectName("douyuTitleWd");

    m_close = new QPushButton("", m_title);
    m_close->setObjectName("btn_close");

    auto contentWidget = new QWidget(widget);
    contentWidget->setObjectName("douyuContentWd");

    auto liveList = new QLiveList(contentWidget);
    auto roomlist = new QRoomListView(contentWidget);

    auto search = new QLineEdit(contentWidget);
    search->setObjectName("room_search");
    search->setPlaceholderText(tr("search room"));

    auto page = new QPageWidget(contentWidget);

    auto layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_title, 0, Qt::AlignTop);
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

    auto titleLayout = new QHBoxLayout(m_title);
    titleLayout->addStretch();
    titleLayout->addWidget(m_close);

    auto layoutContent = new QHBoxLayout(contentWidget);
    layoutContent->setMargin(5);

    layoutContent->addLayout(layoutLeft);
    layoutContent->addLayout(layoutRight);

    CALC_WIDGET_WIDTH(liveList, 190);

    connect(m_close, &QPushButton::clicked, this, &QFrameLessWidget::hide);

    auto modelPlatform = reinterpret_cast<QLiveListModel*>(liveList->model());
    auto modelRoom = reinterpret_cast<QRoomListViewModel*>(roomlist->model());
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
        CENTER_DESKTOP(this);
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

bool QDouyuWidget::isValid()
{
    return m_title->underMouse() && !m_close->underMouse();
}

void QDouyuWidget::showEvent(QShowEvent *event)
{
    CALC_WIDGET_SIZE(this, 1344, 864);
    QFrameLessWidget::showEvent(event);
}
