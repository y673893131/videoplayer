#include "qtoolwidgets.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QApplication>
#include <QSlider>
#include <QListView>
#include <QLineEdit>
#include <QFileIconProvider>
#include <QDebug>
#include <QPropertyAnimation>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QStackedWidget>
#include <QTimer>
#include <QTime>
#include <QToolTip>
#include "playlist/qplayfilelistmodel.h"
#include "playlist/qfilelistview.h"
#include "config/config.h"
#include "ui/qinputurlwidget.h"
#include "ui/tool/fileview/qfileview.h"
#include "ui/tool/play_control/qplaycontrol.h"
#include "ui/tool/progress/qplayprogress.h"
#include "ui/tool/subtitle/qplaysubtitle.h"
#include "ui/tool/title/qplaytitle.h"
#include "ui/tool/live_platform/qliveplatform.h"
#include "ui/tool/menu/qplaymenu.h"
#include "ui/tool/output/qoutputwidget.h"
#include "platform/platform/qdouyuwidget.h"
#include "platform/platform/qliveplatformmanager.h"
#include "control/videocontrol.h"
#include "filter/qinputfilter.h"
#ifdef Q_OS_WIN
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#endif
#include "filter/qinputfilter.h"

QToolWidgets::QToolWidgets(QWidget *parent)
    : QWidget(parent)
    , m_bLocalFile(false)
    , m_index(0)
    , m_playMode(QPlayFileListModel::play_mode_local)
    , m_totalSeconds(0)
{
    init(parent);
//    APPEND_EXCEPT_FILTER(this);
}

void QToolWidgets::init(QWidget *parent)
{
    initStyle();
    initUi(parent);
    initLayout();
    initSize();
    initConnect();
    qApp->installNativeEventFilter(this);
}

void QToolWidgets::initStyle()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
#ifdef Q_OS_WIN
    auto hwnd = reinterpret_cast<HWND>(this->winId());
//    setAreo(hwnd);
//    setShadow(hwnd);
#endif
}

void QToolWidgets::initUi(QWidget *parent)
{
    setObjectName("toolWd");
    m_contorl = VIDEO_CONTROL;
    m_inputUrl = new QInputUrlWidget(this);

    CreateMenu(parent);
    CreateCenterToolbar();

    m_tools[tool_play_subtitle] = new QPlaySubtitle(this);//stay bottom parent, first create
    m_tools[tool_play_output] = new QOutputWidget(this);
    m_tools[tool_play_title] = new QPlayTitle(this);
    m_tools[tool_file_list] = new QFileView(this);
    m_tools[tool_play_control] = new QPlayControl(this);
    m_tools[tool_play_progress] = new QPlayProgress(this);
    m_tools[tool_live_platform] = new QLivePlatform(this);


    m_autoHidetimer = new QTimer(this);
    m_autoHidetimer->setSingleShot(true);
    m_autoHidetimer->setInterval(3000);
}

void QToolWidgets::initLayout()
{
    auto layout = new QVBoxLayout(this);
    auto layoutMid = new QHBoxLayout;
    auto layoutMidCenter = new QVBoxLayout;

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_tools[tool_play_title]);
    layout->addLayout(layoutMid);
    layout->addWidget(m_tools[tool_play_progress]);
    layout->addWidget(m_tools[tool_play_control]);

    layoutMid->setContentsMargins(5, 0, 0, 0);
    layoutMid->addWidget(m_tools[tool_live_platform]);
    layoutMid->addStretch();
    layoutMid->addLayout(layoutMidCenter);
    layoutMid->addStretch();
    layoutMid->addWidget(m_tools[tool_file_list]);

    layoutMidCenter->addStretch();
    layoutMidCenter->addWidget(m_openfile);
    layoutMidCenter->addStretch();
}

void QToolWidgets::initSize()
{
    CALC_WIDGET_WIDTH(m_tools[tool_live_platform], 200);
    CALC_WIDGET_WIDTH(m_tools[tool_file_list], 200);
    CALC_WIDGET_HEIGHT(m_tools[tool_play_title], 40);
    CALC_WIDGET_HEIGHT(m_tools[tool_play_control], 85);
    CALC_WIDGET_HEIGHT(m_tools[tool_play_subtitle], 85);
    CALC_WIDGET_HEIGHT(m_tools[tool_play_output], 40);
}

void QToolWidgets::initConnect()
{
    connect(this, &QToolWidgets::inputUrl, m_inputUrl, &QInputUrlWidget::showInit);
    connect(m_inputUrl, &QInputUrlWidget::inputUrl, this, &QToolWidgets::load);
    connect(this, &QToolWidgets::start, [this](int index)
    {
        m_autoHidetimer->start();
        m_index = index;
    });

    connect(this, &QToolWidgets::hideOrShow, m_autoHidetimer, &QTimer::stop);
    connect(m_autoHidetimer, &QTimer::timeout, [this]
    {
        if(m_contorl->isPlaying())
            emit hideOrShow(true);
    });

    connect(QInputFilter::instance(), &QInputFilter::mouseMove, [this]
    {
        emit hideOrShow(false);
        m_autoHidetimer->start();

        auto pos = QCursor::pos();
        pos = mapFromGlobal(pos);
        auto y0 = m_tools[tool_play_title]->pos().y() + m_tools[tool_play_title]->height();
        auto y1 = m_tools[tool_play_progress]->pos().y();
        if(pos.x() < 25 && pos.y() > y0 && pos.y() < y1)
            emit moveShowPlatform();
    });

    connect(m_openfile, &QPushButton::clicked, this, &QToolWidgets::onLoadFile);
    connect(this, &QToolWidgets::loadFile, this, &QToolWidgets::onLoadFile);

    connect(this, &QToolWidgets::setTotalSeconds, [this]{
        m_openfile->hide();
    });

    m_playMenu->initConnect();
    for(int i = 0; i < tool_max; ++i)
    {
        m_tools[i]->initConnect();
    }

    connect(this, &QToolWidgets::hideOrShow, this, &QToolWidgets::onAutoVisable);

    connect(QInputFilter::instance(), &QInputFilter::escap, this, [=]{if(isFullScreen()) showNormal(); });
}

int QToolWidgets::index()
{
    return m_index;
}

void QToolWidgets::setExists(bool bExists)
{
    m_bLocalFile = bExists;
}

void QToolWidgets::onLoadFile()
{
    auto fileNames = QFileDialog::getOpenFileNames(
                this,
                QString(),
                QString(),
                "All Files (*.*);;mp4 (*.mp4);;flv (*.flv);;avi (*.avi);;mkv (*.mkv);;rmvb (*.rmvb);;url (*.urls);;mp3 (*.mp3)");

    if(!fileNames.isEmpty())
    {
        emit load(fileNames.at(0));
    }
}

void QToolWidgets::onLeftPress()
{
#ifdef Q_OS_WIN
    if(isFullScreen())
        return;
    if(::ReleaseCapture()){
        ::SendMessage(HWND(this->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
    }
#endif
}

void QToolWidgets::onMax()
{
    auto widget =/* parentWidget()*/this;
    if(!widget)
        widget = this;

    if(widget->isMaximized() || widget->isFullScreen())
    {
        widget->showNormal();
        setMaximumSize(qApp->desktop()->availableGeometry().size());
    }
    else
    {
        setMaximumSize(qApp->desktop()->availableGeometry().size());
        widget->showMaximized();
    }
}

void QToolWidgets::onFull()
{
    auto widget =/* parentWidget()*/this;
    if(!widget)
        widget = this;
    if(widget->isMaximized() || widget->isFullScreen())
    {
        widget->showNormal();
        setMaximumSize(qApp->desktop()->availableGeometry().size());
    }
    else
    {
//        setMaximumSize(qApp->desktop()->screenGeometry().size());
        widget->showFullScreen();
    }
}

void QToolWidgets::onAutoVisable(bool bHide)
{
    if(bHide)
    {
        qApp->setOverrideCursor(Qt::BlankCursor);
    }
    else
    {
        qApp->restoreOverrideCursor();
    }
}

void QToolWidgets::CreateCenterToolbar()
{
    m_openfile = new QPushButton(tr("open media file..."), this);
    m_openfile->setObjectName("btn_openfile");

    QFileIconProvider f;
    m_openfile->setIcon(f.icon(QFileIconProvider::File));
}

void QToolWidgets::CreateMenu(QWidget* parent)
{
    m_playMenu = new QPlayMenu(this, parent);
}

void QToolWidgets::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(event->buttons() & Qt::RightButton)
        emit showMenu();
}

void QToolWidgets::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if(event->buttons() == Qt::LeftButton)
    {
#ifdef Q_OS_WIN
        onLeftPress();
#endif
    }
}

void QToolWidgets::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
    if(event->button() == Qt::LeftButton)
    {
        onFull();
    }
}

void QToolWidgets::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    emit _move(event->pos());
}

bool QToolWidgets::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN
    auto msg = reinterpret_cast<MSG*>(message);
//    auto widget = QWidget::find((int)msg->hwnd);
//    if(msg->hwnd == reinterpret_cast<HWND>(m_inputUrl->winId()))
//        qDebug() << __FUNCTION__ << widget << msg->message;
    if(msg->hwnd != reinterpret_cast<HWND>(this->winId()))
    {
        // resize box
        // 因为设置了WA_TranslucentBackground，不能捕获到鼠标的消息（用于缩放），需要转发给自己
        // 如果不设置WA_TranslucentBackground，背景无法穿透，不能看到视频界面
        if((isActiveWindow() || (parentWidget() && parentWidget()->isActiveWindow()))
                && (msg->message == WM_NCHITTEST || msg->message == WM_NCLBUTTONDOWN || msg->message == WM_KEYDOWN))
        {
            msg->hwnd = reinterpret_cast<HWND>(winId());
        }
        else
        {
            return false;
        }
    }

    if(_nativeEvent(eventType, message, result, this))
        return true;
#endif
    return false;
}

void QToolWidgets::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    UTIL->setWindowEllispeFrame(this, 0, 0);
    emit _resize(event->size());
    m_tools[tool_play_subtitle]->setFixedWidth(width());
    m_tools[tool_play_subtitle]->move(0, height() - m_tools[tool_play_subtitle]->height());
    m_tools[tool_play_output]->setFixedWidth(width());
    m_tools[tool_play_output]->move(0, m_tools[tool_play_title]->height());
}
