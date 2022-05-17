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
#include "config/config.h"
#include "ui/qvolumewidget.h"
#include "ui/tool/fileview/qfileview.h"
#include "ui/tool/play_control/qplaycontrol.h"
#include "ui/tool/progress/qplayprogress.h"
#include "ui/tool/subtitle/qplaysubtitle.h"
#include "ui/tool/title/qplaytitle.h"
#include "ui/tool/live_platform/qliveplatform.h"
#include "ui/tool/menu/qplaymenu.h"
#include "ui/tool/output/qoutputwidget.h"
#include "platform/platform/qdouyuwidget.h"
#ifdef GAME_PLATFORM_LIVE
#include "platform/platform/qliveplatformmanager.h"
#endif
#include "control/videocontrol.h"
#include "filter/qinputfilter.h"
#ifdef Q_OS_WIN
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#endif
#include "filter/qinputfilter.h"

#include "framelesswidget/nativeevent_p.h"

#include "ui/pop/qopenlocalfilewidget.h"
#include "ui/pop/qinputurlwidget.h"
#include "ui/pop/qmediainfowidget.h"

class QToolWidgetsPrivate : public CNativeEvent_p
{
    VP_DECLARE_PUBLIC(QToolWidgets)

    enum pop
    {
        pop_local_file,
        pop_url_input,
        pop_media_info,

        pop_max
    };

    inline QToolWidgetsPrivate(QToolWidgets* parent)
        : CNativeEvent_p(parent)
        , m_bLocalFile(false)
        , m_playMenu(nullptr)
    {
#ifdef Q_OS_WIN
        m_flags = Qt::Window | Qt::FramelessWindowHint;
#else
        m_flags = Qt::FramelessWindowHint;
#endif
        m_pops.resize(pop_max);
    }

    QPopWidget *pop(int index);
    bool isPopUnderMouse();

    void popMenu();

    void createCenterToolbar();

    QWidget* m_backWd, *m_openfileWd;
    QToolBase* m_tools[QToolWidgets::tool_max];
    QPushButton *m_openfile,*m_min,*m_max,*m_close;
    QVolumeWidget* m_volume;

    bool m_bLocalFile;
    QPlayMenu* m_playMenu;
    QVideoControl* m_contorl;
    QTimer* m_autoHidetimer;

    QVector<QPopWidget*> m_pops;
};

QPopWidget *QToolWidgetsPrivate::pop(int index)
{
    VP_Q(QToolWidgets);
    if(!m_pops[index])
    {
        switch (index) {
        case pop_local_file:
            m_pops[index] = new QOpenLocalFileWidget(q);
            break;
        case pop_url_input:
            m_pops[index] = new QInputUrlWidget(q);
            break;
        case pop_media_info:
            m_pops[index] = new QMediaInfoWidget(q);
            break;
        }
    }

    return m_pops[index];
}

bool QToolWidgetsPrivate::isPopUnderMouse()
{
    for(auto&& it : qAsConst(m_pops))
    {
        if(it->isVisible() && it->underMouse())
            return true;
    }

    return false;
}

void QToolWidgetsPrivate::popMenu()
{
    if(!m_playMenu)
    {
        VP_Q(QToolWidgets);
        m_playMenu = new QPlayMenu(q, q);
        m_playMenu->initConnect();
        m_playMenu->onLoadConfig();
    }

    m_playMenu->onPop();
}

void QToolWidgetsPrivate::createCenterToolbar()
{
    VP_Q(QToolWidgets);
    m_openfileWd = new QWidget(q);
    m_openfile = new QPushButton(QToolWidgets::tr("open media file"), m_openfileWd);
    m_openfile->setObjectName("btn_openfile");

    QFileIconProvider f;
    m_openfile->setIcon(f.icon(QFileIconProvider::File));

    auto layout = new QHBoxLayout(m_openfileWd);
    layout->setMargin(0);
    layout->addWidget(m_openfile);
}

QToolWidgets::QToolWidgets(QWidget *parent)
    : QFrameLessWidget(new QToolWidgetsPrivate(this), parent)
{
    init(parent);
}

void QToolWidgets::init(QWidget *parent)
{
    initStyle();
    initUi(parent);
    initLayout();
    initSize();
    initConnect();
#ifdef Q_OS_WIN
    qApp->installNativeEventFilter(this);
#endif
}

void QToolWidgets::initStyle()
{
#ifdef Q_OS_WIN
    setAttribute(Qt::WA_TranslucentBackground);
#endif
}

void QToolWidgets::initUi(QWidget *)
{
    VP_D(QToolWidgets);

    setObjectName("toolWd");
    d->m_contorl = VIDEO_CONTROL;
    d->m_volume = new QVolumeWidget(this);
    d->createCenterToolbar();
    d->m_tools[tool_play_subtitle] = new QPlaySubtitle(this);//stay bottom parent, first create
    d->m_tools[tool_play_output] = new QOutputWidget(this);
    d->m_tools[tool_play_title] = new QPlayTitle(this);
    d->m_tools[tool_file_list] = new QFileView(this);
    d->m_tools[tool_play_control] = new QPlayControl(this);
    d->m_tools[tool_play_progress] = new QPlayProgress(this);
#ifdef GAME_PLATFORM_LIVE
    d->m_tools[tool_live_platform] = new QLivePlatform(this);
#else
    d->m_tools[tool_live_platform] = nullptr;
#endif
    d->m_autoHidetimer = new QTimer(this);
    d->m_autoHidetimer->setSingleShot(true);
    d->m_autoHidetimer->setInterval(3000);
}

void QToolWidgets::initLayout()
{
    VP_D(QToolWidgets);
    auto layout = new QVBoxLayout(this);
    auto layoutMid = new QHBoxLayout;
    auto layoutMidCenter = new QVBoxLayout;

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(d->m_tools[tool_play_title]);
    layout->addLayout(layoutMid);
    layout->addWidget(d->m_tools[tool_play_progress]);
    layout->addWidget(d->m_tools[tool_play_control]);

    layoutMid->setContentsMargins(5, 0, 0, 0);
#ifdef GAME_PLATFORM_LIVE
    layoutMid->addWidget(d->m_tools[tool_live_platform]);
#endif
    layoutMid->addStretch();
    layoutMid->addLayout(layoutMidCenter);
    layoutMid->addStretch();
    layoutMid->addWidget(d->m_tools[tool_file_list]);

    layoutMidCenter->addStretch();
    layoutMidCenter->addWidget(d->m_openfile);
    layoutMidCenter->addStretch();
}

void QToolWidgets::initSize()
{
    VP_D(QToolWidgets);
#ifdef GAME_PLATFORM_LIVE
    CALC_WIDGET_WIDTH(d->m_tools[tool_live_platform], 200);
#endif
    CALC_WIDGET_WIDTH(d->m_tools[tool_file_list], 200);
    CALC_WIDGET_HEIGHT(d->m_tools[tool_play_title], 30);
    CALC_WIDGET_HEIGHT(d->m_tools[tool_play_progress], 10);
    CALC_WIDGET_HEIGHT(d->m_tools[tool_play_control], 40);
    auto subtitleMinHeight = CALC_WIDGET_HEIGHT(nullptr, 85);
    d->m_tools[tool_play_subtitle]->setMinimumHeight(subtitleMinHeight);
    CALC_WIDGET_HEIGHT(d->m_tools[tool_play_output], 40);
    CALC_WIDGET_SIZE(this, 800, 600);
    CENTER_DESKTOP(this);
}

void QToolWidgets::initConnect()
{
    VP_D(QToolWidgets);
    auto control = VIDEO_CONTROL;
    d->m_volume->initConnect();
    for(int i = 0; i < QToolWidgetsPrivate::pop_max; ++i)
    {
        d->pop(i)->initConnect(this);
    }

    connect(this, &QToolWidgets::showMenu, this, [=]{ d->popMenu(); });
    connect(control, &QVideoControl::start, d->m_autoHidetimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(this, &QToolWidgets::hideOrShow, d->m_autoHidetimer, &QTimer::stop);
    connect(this, &QToolWidgets::hideOrShow, this, &QToolWidgets::onAutoVisable);
    connect(d->m_autoHidetimer, &QTimer::timeout, this, [this, d]
    {
        if(d->m_contorl->isPlaying())
            emit hideOrShow(true);
    });

    connect(QInputFilter::instance(), &QInputFilter::mouseMove, this, [this, d]
    {
        emit hideOrShow(false);
        d->m_autoHidetimer->start();

        auto pos = QCursor::pos();
        pos = mapFromGlobal(pos);
        auto y0 = d->m_tools[tool_play_title]->pos().y() + d->m_tools[tool_play_title]->height();
        auto y1 = d->m_tools[tool_play_progress]->pos().y();
        if(pos.x() < 25 && pos.y() > y0 && pos.y() < y1)
            emit moveShowPlatform();
    });

    connect(d->m_openfile, &QPushButton::clicked, this, &QToolWidgets::onLoadFile);
    connect(this, &QToolWidgets::loadFile, this, &QToolWidgets::onLoadFile);

    connect(this, &QToolWidgets::setTotalSeconds, this, [d]{
        d->m_openfile->hide();
    });

    connect(control, &QVideoControl::end, this, [d]{
        d->m_openfile->show();
    });

    connect(control, &QVideoControl::exceptionEnd, this, [d]{
        d->m_openfile->show();
    });

    for(int i = 0; i < tool_max; ++i)
    {
        if(d->m_tools[i])
        {
            d->m_tools[i]->initConnect();
        }
    }

    connect(QInputFilter::instance(), &QInputFilter::escap, this, [=]{if(isFullScreen()) showNormal(); });

    connect(this, &QToolWidgets::topWindow, this, [=](bool bTop){ SET_CONFIG_DATA(bTop, Config::Data_TopWindow);});
}

void QToolWidgets::setExists(bool bExists)
{
    VP_D(QToolWidgets);
    d->m_bLocalFile = bExists;
}

void QToolWidgets::onLoadFile()
{
    auto fileNames = QFileDialog::getOpenFileNames(
                this,
                QString(),
                QString(),
                "All Files (*.*);;mp4 (*.mp4);;flv (*.flv);;avi (*.avi);;mkv (*.mkv);;rmvb (*.rmvb);;url (*.urls);;mp3 (*.mp3);;wav (*.wav);;m4a (*.m4a);;flac (*.flac)");

    if(!fileNames.isEmpty())
    {
        emit load(fileNames);
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
        setMaximumSize(UTIL->desktopSize());
    }
    else
    {
        setMaximumSize(UTIL->desktopSize());
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
        setMaximumSize(UTIL->desktopSize());
        emit showNor();
    }
    else
    {
        widget->showFullScreen();
        emit showFull();
    }
}

void QToolWidgets::onAutoVisable(bool bHide)
{
    VP_D(QToolWidgets);
    if(bHide && !d->isPopUnderMouse())
    {
        qApp->setOverrideCursor(Qt::BlankCursor);
    }
    else
    {
        qApp->restoreOverrideCursor();
    }
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

#ifdef Q_OS_WIN
bool QToolWidgets::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    VP_D(QToolWidgets);
    auto msg = reinterpret_cast<MSG*>(message);
//    auto widget = QWidget::find((int)msg->hwnd);
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

    if(d->nativeEvent(eventType, message, result, this))
        return true;

    return false;
}
#endif

void QToolWidgets::resizeEvent(QResizeEvent *event)
{
    VP_D(QToolWidgets);
    QWidget::resizeEvent(event);
    emit _resize(event->size());
    d->m_tools[tool_play_subtitle]->setFixedHeight(height());
    d->m_tools[tool_play_subtitle]->setFixedWidth(width());
    d->m_tools[tool_play_subtitle]->move(0, height() - d->m_tools[tool_play_subtitle]->height());
    d->m_tools[tool_play_output]->setFixedWidth(width());
    d->m_tools[tool_play_output]->move(0, d->m_tools[tool_play_title]->height());
}

void QToolWidgets::paintEvent(QPaintEvent *event)
{
//    static int count = 0;
//    qDebug() << __FUNCTION__ << count++ ;
    QFrameLessWidget::paintEvent(event);
}
