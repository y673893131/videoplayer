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
#include "qdatamodel.h"
#include "Log/Log.h"
#ifdef unix
#include <unistd.h>
#else
#include "render/qdirect3d11widget.h"
#include "render/qglvideowidget.h"
#endif
#include "render/qrenderfactory.h"

Widget::Widget(QWidget *parent)
    : QFrameLessWidget(parent)
{
    init();
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
    qApp->installEventFilter(this);
    setAcceptDrops(true);
}

void Widget::initResource()
{
    m_render = new QRenderFactory(this);
//    m_video = new VIDEO_TYPE(this);
    m_toolbar = new QToolWidgets(this);
    m_toolbar->show();
    m_control = new QVideoControl(this);
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
    connect(this, &Widget::rightClicked, m_toolbar, &QToolWidgets::showMenu);
    connect(this, &Widget::leftPress, m_toolbar, &QToolWidgets::onLeftPress);
    connect(this, &Widget::inputUrlFile, m_toolbar, &QToolWidgets::inputUrlFile);

    connect(m_control, SIGNAL(appendFrame(void*)), renderWd, SLOT(onAppendFrame(void*)), Qt::QueuedConnection);
    connect(m_control, SIGNAL(start(int)), renderWd, SLOT(onStart()));
    connect(m_control, SIGNAL(end(int)), renderWd, SLOT(onStop()));
    connect(m_control, SIGNAL(videoSizeChanged(int, int)), renderWd, SLOT(onVideoSizeChanged(int, int)));

    connect(m_toolbar, &QToolWidgets::_move, this, [this](const QPoint& pt){ if(pos() != pt){ move(pt);} });
    connect(m_toolbar, &QToolWidgets::_resize, this, [this](const QSize& sz){ if(size() != sz){ resize(sz);} });
    connect(m_toolbar, &QToolWidgets::exit, this, &Widget::onExit);
    connect(m_toolbar, &QToolWidgets::topWindow, this, &Widget::onTopWindow);
    connect(m_toolbar, SIGNAL(viewAdjust(bool)), renderWd, SLOT(onViewAdjust(bool)));
}

void Widget::flushQss()
{
    onFlushSheetStyle();

//    auto timer = new QTimer();
//    timer->setInterval(200);
//    connect(timer, &QTimer::timeout, this, &Widget::onFlushSheetStyle);
//        timer->start();
    flushInitSize();
}

void Widget::flushInitSize()
{
    if(m_render->isUpdate())
    {
        CALC_WIDGET_SIZE(m_toolbar, 0.5f, 0.7f);
        CENTER_DESKTOP(m_toolbar);
    }
    else
    {
        QTimer::singleShot(0, [this]{
            CALC_WIDGET_SIZE(m_toolbar, 0.5f, 0.7f);
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
#define QSS_FILE ":/res/qss.qss"
//#define QSS_FILE "./Resources/res.qss"
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

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if(/*watched == this && */event->type() == QEvent::MouseMove)
    {
        emit m_toolbar->mouseMove();
    }
    else if(watched == this && event->type() == QEvent::KeyPress)
    {
        auto keyEvent = reinterpret_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Escape)
        {
            if(m_toolbar->isFullScreen())
            {
                m_toolbar->showNormal();
            }
//            else
//            {
//                auto btn = QMessageBox::information(this, tr("tips"),tr("quit") + " " + qApp->applicationName() + "?"
//                                                    , QMessageBox::Ok | QMessageBox::Cancel);
//                if(btn == QMessageBox::Ok)
//                    emit m_toolbar->exit();
//            }
        }

        return true;
    }

    return QFrameLessWidget::eventFilter(watched, event);
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
        types << ".mp4" << ".flv" << ".avi" << ".mkv" << ".rmvb" << ".urls";
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
    emit m_toolbar->play(event->mimeData()->urls().begin()->toLocalFile());
}


void Widget::closeEvent(QCloseEvent *event)
{
    event->ignore();
    onExit();
}
