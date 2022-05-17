#include "widget.h"
#include <QDebug>
#include <QBoxLayout>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QApplication>
#include <QIcon>
#include <QThread>
#include <math.h>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QMimeData>
#include <QDragEnterEvent>
#include "video_player_core.h"
#include "control/videocontrol.h"
#include "qtoolwidgets.h"
#include "config/qdatamodel.h"
#include "Log/Log.h"
#include "config/config.h"
#ifdef unix
#include <unistd.h>
#else
#include "render/qdirect3d11widget.h"
#include "render/qglvideowidget.h"
#endif
#include "render/qrenderfactory.h"
#include "filter/qinputfilter.h"

#include "render/videoframe.h"
//#define QSS_MONITOR

#ifdef Q_OS_WIN
#include "ui/thumb/qwinthumbnail_p.h"
class WidgetPrivate : public QWinThumbnailPrivate
#else
#include "framelesswidget/nativeevent_p.h"
class WidgetPrivate : public CNativeEvent_p
#endif
{
    VP_DECLARE_PUBLIC(Widget)
    inline WidgetPrivate(Widget* parent)
#ifdef Q_OS_WIN
        : QWinThumbnailPrivate(parent)
#else
        : CNativeEvent_p(parent)
#endif
        , m_render(nullptr)
        , m_toolbar(nullptr)
        , m_control(nullptr)
    {
    }

    QRenderFactory* m_render;
    QToolWidgets* m_toolbar;
    QVideoControl* m_control;
};

Widget::Widget(QWidget *parent)
#ifdef Q_OS_WIN
    : QWinThumbnail(new WidgetPrivate(this), parent)
#else
    : QFrameLessWidget(new WidgetPrivate(this), parent)
#endif
{
    init();
    APPEND_EXCEPT_FILTER(this);
//    setDragSelf(true);
//    setAreo(reinterpret_cast<void*>(winId()));
//    setShadow(reinterpret_cast<void*>(winId()));
//    setStyleSheet("background: transparent;");
}

Widget::~Widget()
{
}

void Widget::init()
{
    initData();
    initStyle();
    initResource();
}

void Widget::initData()
{
    DATA();
}

void Widget::initStyle()
{
    qApp->setApplicationName("vPlay");
    qApp->setWindowIcon(QIcon(":/app/logo"));
    InitLogInstance(qApp->applicationDirPath().toStdString().c_str(), "log_ui_");
    setAcceptDrops(true);
    setMouseTracking(true);
}

void Widget::initResource()
{
    VP_D(Widget);

    d->m_control = new QVideoControl(this);
    d->m_render = new QRenderFactory(this);
    d->m_toolbar = new QToolWidgets(d->m_render->renderWidget());
    d->m_control->setToolBar(d->m_toolbar);
    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(d->m_render->renderWidget());

#ifdef unix
    auto layoutRender = new QVBoxLayout(d->m_render->renderWidget());
    layoutRender->setMargin(0);
    layoutRender->addWidget(d->m_toolbar);
#endif
    initConnect();
    flushQss();
}

void Widget::initConnect()
{
    VP_D(Widget);
    auto renderWd = d->m_render->renderWidget();
    connect(this, &Widget::rightClicked, d->m_toolbar, &QToolWidgets::showMenu, Qt::QueuedConnection);
    connect(this, &Widget::leftPress, d->m_toolbar, &QToolWidgets::onLeftPress);
    connect(this, &Widget::inputUrlFile, d->m_toolbar, &QToolWidgets::inputUrlFile);
    connect(this, SIGNAL(leftDoubleClicked()), d->m_toolbar, SLOT(onFull()));
#ifdef Q_OS_WIN
    connect(this, &Widget::thumb, d->m_toolbar, &QToolWidgets::thumb);
    connect(this, &Widget::cmd, d->m_toolbar, &QToolWidgets::cmd);
    connect(d->m_toolbar, &QToolWidgets::_move, this, [this](const QPoint& pt){ if(pos() != pt){ move(pt);} });
    connect(d->m_toolbar, &QToolWidgets::_resize, this, [this](const QSize& sz){ if(size() != sz){ resize(sz);} });
#endif

    connect(d->m_toolbar, &QToolWidgets::showMin, this, &Widget::showMinimized);
    connect(d->m_control, SIGNAL(appendFrame(_VideoFramePtr)), renderWd, SLOT(onAppendFrame(_VideoFramePtr)), Qt::QueuedConnection);
    connect(d->m_control, SIGNAL(appendFreq(float*, unsigned int)), renderWd, SLOT(onAppendFreq(float*, unsigned int)), Qt::QueuedConnection);
    connect(d->m_control, SIGNAL(start(int)), renderWd, SLOT(onStart()));
    connect(d->m_control, SIGNAL(end(int)), renderWd, SLOT(onStop()));
    connect(d->m_control, SIGNAL(videoSizeChanged(int, int)), renderWd, SLOT(onVideoSizeChanged(int, int)));
    connect(d->m_toolbar, &QToolWidgets::windowTitleChanged, this, &Widget::setWindowTitle);

    connect(QInputFilter::instance(), &QInputFilter::cap, d->m_render, &QRenderFactory::onCap);
    connect(d->m_toolbar, &QToolWidgets::exit, this, &Widget::onExit);
    connect(d->m_toolbar, &QToolWidgets::topWindow, this, &Widget::onTopWindow);
    connect(d->m_toolbar, SIGNAL(viewAdjust(bool)), renderWd, SLOT(onViewAdjust(bool)));

    QTimer::singleShot(1000, this, [=]
    {
        auto bTop = GET_CONFIG_DATA(Config::Data_TopWindow).toBool();
        onTopWindow(bTop);
        d->m_toolbar->show();
    });
}

void Widget::flushQss()
{
    onFlushSheetStyle();

#if defined(QSS_MONITOR) or _DEBUG
    auto timer = new QTimer();
    timer->setInterval(200);
    connect(timer, &QTimer::timeout, this, &Widget::onFlushSheetStyle);
    timer->start();
#endif
    flushInitSize();
}

void Widget::flushInitSize()
{
    VP_D(Widget);
#ifdef unix
    CALC_WIDGET_SIZE(this, 800, 600);
    CENTER_DESKTOP(this);
#else
    d->m_toolbar->show();
#endif
}

void Widget::onExit()
{
#ifdef Q_OS_WIN
    Log(Log_Opt, "quit begin.");
    ::TerminateProcess(::GetCurrentProcess(), 0);
    Log(Log_Opt, "quit end.");
    return;
#endif

    VP_D(Widget);
    Log(Log_Opt, "quit begin.");
    d->m_control->waittingStoped();
    Log(Log_Opt, "quit end.");
    qApp->quit();
}

void Widget::onTopWindow(bool bTop)
{
    qDebug() << "topWindow:" << bTop;
    setTopWindow(bTop);
    updateTopWindow();
}

void Widget::onFlushSheetStyle()
{
#if QSS_MONITOR
    #define QSS_FILE "./Resources/res.qss"
#else
    #define QSS_FILE ":/style/qss"
#endif
    QFileInfo fi(QSS_FILE);
    static QDateTime m_last;
    QDateTime lastMdTime = fi.lastModified();
    if (m_last != lastMdTime)
    {
        m_last = lastMdTime;
        QFile qss(QSS_FILE);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
        qDebug() << "flush style.";
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    VP_D(Widget);
    QFrameLessWidget::mouseMoveEvent(event);
    emit d->m_toolbar->hideOrShow(false);
}

inline bool checkFile(const QString& file, const QStringList& types)
{
    for(auto&& type : qAsConst(types))
    {
        if(type.length() > file.length()) continue;
        if(file.lastIndexOf(type, file.length() - type.length(), Qt::CaseInsensitive) > 0)
            return true;
    }

    return false;
}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    auto urls = event->mimeData()->urls();
    if(!urls.empty())
    {
        auto file = urls.begin()->toLocalFile();
        QStringList types;
        types << ".mp4" << ".m4a" << ".flv" << ".avi" << ".mkv" << ".rmvb" << ".urls" << ".mp3" << ".wav" << ".aac"<< ".h264" << ".flac";
        if(checkFile(file, types))
            event->accept();
    }
}

void Widget::dropEvent(QDropEvent *event)
{
    VP_D(Widget);
    auto file = event->mimeData()->urls().begin()->toLocalFile();
    if(file.contains(".url"))
    {
        emit inputUrlFile(file);
        return;
    }
    emit d->m_toolbar->load(QStringList(event->mimeData()->urls().begin()->toLocalFile()));
}


void Widget::closeEvent(QCloseEvent *event)
{
    event->ignore();
    onExit();
}
