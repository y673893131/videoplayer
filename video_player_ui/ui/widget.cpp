#include "widget.h"
#include <QDebug>
#include <QBoxLayout>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QApplication>
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
//#define QSS_MONITOR

Widget::Widget(QWidget *parent)
    : QFrameLessWidget(parent)
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
    InitLogInstance(qApp->applicationDirPath().toStdString().c_str(), "log_ui_");
    setAcceptDrops(true);
}

void Widget::initResource()
{
    m_control = new QVideoControl(this);
    m_render = new QRenderFactory(this);
//    m_video = new VIDEO_TYPE(this);
    m_toolbar = new QToolWidgets(m_render->renderWidget());
    m_toolbar->show();

    m_control->setToolBar(m_toolbar);

    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_render->renderWidget());

    initConnect();
    flushQss();
}

void Widget::initConnect()
{
    auto renderWd = m_render->renderWidget();
    connect(this, &Widget::rightClicked, m_toolbar, &QToolWidgets::showMenu, Qt::QueuedConnection);
    connect(this, &Widget::leftPress, m_toolbar, &QToolWidgets::onLeftPress);
    connect(this, &Widget::inputUrlFile, m_toolbar, &QToolWidgets::inputUrlFile);
    connect(this, SIGNAL(leftDoubleClicked()), m_toolbar, SLOT(onFull()));

    connect(m_control, SIGNAL(appendFrame(void*)), renderWd, SLOT(onAppendFrame(void*)), Qt::QueuedConnection);
    connect(m_control, SIGNAL(start(int)), renderWd, SLOT(onStart()));
    connect(m_control, SIGNAL(end(int)), renderWd, SLOT(onStop()));
    connect(m_control, SIGNAL(videoSizeChanged(int, int)), renderWd, SLOT(onVideoSizeChanged(int, int)));

    connect(m_toolbar, &QToolWidgets::_move, this, [this](const QPoint& pt){ if(pos() != pt){ move(pt);} });
    connect(m_toolbar, &QToolWidgets::_resize, this, [this](const QSize& sz){ if(size() != sz){ resize(sz);} });
    connect(m_toolbar, &QToolWidgets::showMin, this, &Widget::showMinimized);
    connect(m_toolbar, &QToolWidgets::exit, this, &Widget::onExit);
    connect(m_toolbar, &QToolWidgets::topWindow, this, &Widget::onTopWindow);
    connect(m_toolbar, SIGNAL(viewAdjust(bool)), renderWd, SLOT(onViewAdjust(bool)));

    QTimer::singleShot(1000, [=]
    {
        auto bTop = GET_CONFIG_DATA(Config::Data_TopWindow).toBool();
        onTopWindow(bTop);
    });
}

void Widget::flushQss()
{
    onFlushSheetStyle();

#if QSS_MONITOR
    auto timer = new QTimer();
    timer->setInterval(200);
    connect(timer, &QTimer::timeout, this, &Widget::onFlushSheetStyle);
    timer->start();
#endif
    flushInitSize();
}

void Widget::flushInitSize()
{
    if(m_render->isUpdate())
    {
        CALC_WIDGET_SIZE(m_toolbar, 960, 756);
        CENTER_DESKTOP(m_toolbar);
    }
    else
    {
        QTimer::singleShot(0, [this]{
            CALC_WIDGET_SIZE(m_toolbar, 960, 756);
            CENTER_DESKTOP(m_toolbar);
        });
    }
}

void Widget::onExit()
{
#ifdef Q_OS_WIN
    Log(Log_Opt, "quit begin.");
    ::TerminateProcess(::GetCurrentProcess(), 0);
    Log(Log_Opt, "quit end.");
    return;
#endif
    Log(Log_Opt, "quit begin.");
    m_control->waittingStoped();
    Log(Log_Opt, "quit end.");
    qApp->quit();
}

void Widget::onTopWindow(bool bTop)
{
    qDebug() << "topWindow:" << bTop;
    m_bTopWindow = bTop;
    updateTopWindow();
}

void Widget::onFlushSheetStyle()
{
#if QSS_MONITOR
    #define QSS_FILE "./Resources/res.qss"
#else
    #define QSS_FILE ":/res/qss.qss"
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
    QFrameLessWidget::mouseMoveEvent(event);
    emit m_toolbar->hideOrShow(false);
}


inline bool checkFile(const QString& file, const QStringList& types)
{
    for(auto type : types)
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
        types << ".mp4" << ".flv" << ".avi" << ".mkv" << ".rmvb" << ".urls" << ".mp3" << ".aac"<< ".h264";
        if(checkFile(file, types))
            event->accept();
    }
}

void Widget::dropEvent(QDropEvent *event)
{
    auto file = event->mimeData()->urls().begin()->toLocalFile();
    if(file.contains(".url"))
    {
        emit inputUrlFile(file);
        return;
    }
    emit m_toolbar->load(event->mimeData()->urls().begin()->toLocalFile());
}


void Widget::closeEvent(QCloseEvent *event)
{
    event->ignore();
    onExit();
}
