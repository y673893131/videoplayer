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
#include "video_player_core.h"
#include "QGLVideoWidget.h"
#include "QToolWidgets.h"

Widget::Widget(QWidget *parent)
    : QFrameLessWidget(parent)
{
    resize(1024, 765);
    setBackgroundColor("#2E2F30");
    QStringList files;
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\1.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\2.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\3.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\4.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\1.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\2.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\3.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\4.mp4";
    files << "http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8";//CCTV 高清
//    files << "http://ivi.bupt.edu.cn/hls/zjhd.m3u8";//浙江 高清
//    files << "rtmp://172.16.62.127:1935/live/room";
//    files << "rtmp://58.200.131.2:1935/livetv/hunantv";//湖南卫视
//    files << "http://ivi.bupt.edu.cn/hls/hunanhd.m3u8";//湖南卫视 高清
    int count = files.count();
    int colcount = sqrt(count);
    for (int n = 0;n < count; ++n) {
#define _OPENGL_
#ifdef _OPENGL_
        m_video = new QGLVideoWidget(this);
#else
        m_video = new QLabel(this);
#endif
        m_toolbar = new QToolWidgets(this);
        m_video->setAutoFillBackground(true);

        auto core = new video_player_core();
        core->_init();
        core->_setCallBack(m_video);
        auto file = files[n];
        connect(m_toolbar, &QToolWidgets::play, [file, core, this](const QString& filename){
            QString playfile = file;
            if(!filename.isEmpty()){
                playfile = filename;
            }else
            {
                if(core->_getState() == video_player_core::state_paused){
                    qDebug() << "countinue play.";
                    core->_continue();
                    return;
                }
            }

            core->_stop();
            core->_setSrc(playfile.toStdString().c_str());

            qDebug() << "==>>start play:" << playfile;
            qDebug() << m_video->size();
//            core->_setsize(m_video->width(), m_video->height());
            core->_play();
        });

        connect(m_toolbar, &QToolWidgets::pause, [core]{
            if(core->_getState() == video_player_core::state_paused)
            {
                core->_continue();
                qDebug() << "countinue play.";
            }else{
                core->_pause();
                qDebug() << "pause play.";
            }
        });

        connect(m_toolbar, &QToolWidgets::stop, [core]{
            core->_stop();
        });
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
    QFileInfo fi(":/res/qss.qss");
    QDateTime lastMdTime = fi.lastModified();
    if (m_last != lastMdTime)
    {
        m_last = lastMdTime;
        QFile qss(":/res/qss.qss");
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
        qDebug() << "flush style.";
    }
}

void Widget::resizeEvent(QResizeEvent *event)
{
    __super::resizeEvent(event);
    m_video->resize(size());
}

bool Widget::isValid()
{
    return m_toolbar->isUnderValid();
}
