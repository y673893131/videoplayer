#include "widget.h"
#include <QDebug>
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QApplication>
#include <math.h>
#include <QMessageBox>
#include <QKeyEvent>
#include "video_player_core.h"
#include "qglvideowidget.h"
#include "qtoolwidgets.h"
#include "Log/Log.h"
#ifdef unix
#include <unistd.h>
#endif

Widget::Widget(QWidget *parent)
    : QFrameLessWidget(parent)
{
    qApp->setApplicationName("vPlay");
    InitLogInstance(qApp->applicationDirPath().toStdString().c_str(), "log_");
    qApp->installEventFilter(this);
    setAcceptDrops(true);
    for (int n = 0;n < 1; ++n)
    {
        m_video = new QGLVideoWidget(this);
        m_toolbar = new QToolWidgets(this);
        m_video->setAutoFillBackground(true);

        static video_player_core* core = nullptr;
        core = new video_player_core();
        core->_init();
        core->_setCallBack(m_video);

        auto funcStop = [this]
        {
            int index = m_toolbar->index();
            core->_stop(index);
            int state = core->_state(index);
            qDebug() << "index: " << index << " state: " << state;
            while(state != video_player_core::state_uninit
                   && state != video_player_core::state_stopped)
            {
#ifdef unix
                usleep(100 * 1000);
#else
                Sleep(100);
#endif
                state = core->_state(index);
            }
        };

        connect(m_toolbar, &QToolWidgets::play, [this, funcStop](const QString& filename)
        {
            if(filename.isEmpty())
            {
                emit m_toolbar->loadFile();
                return ;
            }

            if(core)
            {
                if(core->_getState() == video_player_core::state_paused)
                {
                    qDebug() << "countinue play.";
                    core->_continue(m_toolbar->index());
                    return;
                }
            }

            funcStop();

            core->_setSrc(filename.toUtf8().toStdString());

            qDebug() << "==>>start play:" << filename;
            qDebug() << m_video->size();
            Log(Log_Info, "--->>play: %s", filename.toLocal8Bit().toStdString().c_str());
            core->_play();
        });

        connect(m_toolbar, &QToolWidgets::exit, [funcStop]
        {
            Log(Log_Opt, "quit begin.");
            funcStop();
            Log(Log_Opt, "quit end.");
            qApp->quit();
        });

        connect(m_video, &QGLVideoWidget::start, m_toolbar, &QToolWidgets::start);
        connect(m_video, &QGLVideoWidget::playOver, [this](int index)
        {
            emit m_toolbar->stop();
        });
        connect(m_video, &QGLVideoWidget::setpos, m_toolbar, &QToolWidgets::setPosSeconds);
        connect(m_video, &QGLVideoWidget::total, m_toolbar, &QToolWidgets::setTotalSeconds);
        connect(m_toolbar, &QToolWidgets::pause, [this]
        {
            if(!core)return;
            core->_pause(m_toolbar->index());
            qDebug() << "pause play." << m_toolbar->index();
        });

        connect(m_toolbar, &QToolWidgets::continuePlay, [this]
        {
            if(!core)return;
            core->_continue(m_toolbar->index());
            qDebug() << "continue play." << m_toolbar->index();
        });

        connect(m_toolbar, &QToolWidgets::stop, [this]
        {
            if(!core)return;
            core->_stop(m_toolbar->index());
        });
        connect(m_toolbar, &QToolWidgets::setSeekPos, [this](int value)
        {
            if(!core)return;
            core->_seek(m_toolbar->index(), value);
        });

        connect(m_toolbar, &QToolWidgets::setVol, [this](int value)
        {
            if(!core)return;
            core->_setVol(m_toolbar->index(), value);
        });
        connect(m_toolbar, &QToolWidgets::mute, [this](bool bMute)
        {
            if(!core)return;
            core->_setMute(m_toolbar->index(), bMute);
        });

        connect(m_toolbar, &QToolWidgets::viewAdjust, m_video, &QGLVideoWidget::onViewAdjust);
        connect(m_toolbar, &QToolWidgets::topWindow, [this](bool bTop)
        {
            qDebug() << "topWindow:" << bTop;
            m_bTopWindow = bTop;
            updateTopWindow();
        });
        connect(m_video, &QGLVideoWidget::frameRate, m_toolbar, &QToolWidgets::frameRate);
    }

    flushSheetStyle();
    auto timer = new QTimer();
    timer->setInterval(200);
    connect(timer, &QTimer::timeout, this, &Widget::flushSheetStyle);
    timer->start();
}

Widget::~Widget()
{
}

void Widget::flushSheetStyle()
{
#define QSS_FILE ":/res/qss.qss"
//#define QSS_FILE "./Resources/res.qss"
    QFileInfo fi(QSS_FILE);
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

void Widget::resizeEvent(QResizeEvent *event)
{
    QFrameLessWidget::resizeEvent(event);
    m_video->resize(size());
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
        emit m_toolbar->move();
    }
    else if(watched == this && event->type() == QEvent::KeyPress)
    {
        auto keyEvent = (QKeyEvent*)(event);
        if(keyEvent->key() == Qt::Key_Escape)
        {
            if(isFullScreen())
            {
                showNormal();
            }
            else
            {
                auto btn = QMessageBox::information(this, tr("tips"),tr("quit") + " " + qApp->applicationName() + "?"
                                                    , QMessageBox::Ok | QMessageBox::Cancel);
                if(btn == QMessageBox::Ok)
                    emit m_toolbar->exit();
            }
        }

        return true;
    }

    return QFrameLessWidget::eventFilter(watched, event);
}

bool Widget::isValid()
{
    return m_toolbar->isUnderValid();
}

#include <QMimeData>
#include <QDragEnterEvent>
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
    qDebug() << "QDragEnterEvent";
    auto urls = event->mimeData()->urls();
    if(!urls.empty())
    {
        auto file = urls.begin()->toLocalFile();
        QStringList types;
        types << ".mp4" << ".flv" << ".avi" << ".mkv";
        if(checkFile(file, types))
            event->accept();
    }
}

void Widget::dropEvent(QDropEvent *event)
{
    emit m_toolbar->play(event->mimeData()->urls().begin()->toLocalFile());
}
