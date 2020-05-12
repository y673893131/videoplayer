#include "widget.h"
#include <QDebug>
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QTimer>
#include "video_player_core.h"
#include "QGLVideoWidget.h"

class t : public QObjectUserData
{
public:
    t(){
        id = 0;
    }
    int id;
};

Widget::Widget(QWidget *parent)
    : QFrameLessWidget(parent)
{
    resize(1024, 765);
    setBackgroundColor("#2E2F30");

    auto layout = new QGridLayout;
    layout->setMargin(10);
    setLayout(layout);
    QStringList files;
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\1.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\2.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\3.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\4.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\1.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\2.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\3.mp4";
//    files << "C:\\Users\\Administrator\\Desktop\\test\\mp4\\4.mp4";
//    files << "http://ivi.bupt.edu.cn/hls/cctv5phd.m3u8";//CCTV 高清
//    files << "http://ivi.bupt.edu.cn/hls/zjhd.m3u8";//浙江 高清
//    files << "rtmp://172.16.62.127:1935/live/room";
//    files << "rtmp://58.200.131.2:1935/livetv/hunantv";//湖南卫视
    files << "http://ivi.bupt.edu.cn/hls/hunanhd.m3u8";//湖南卫视 高清
    int count = files.count();
    int colcount = sqrt(count);
    for (int n = 0;n < count; ++n) {
#define _OPENGL_
#ifdef _OPENGL_
        auto video = new QGLVideoWidget(this);
#else
        auto video = new QLabel(this);
#endif
        auto btn_play = new QPushButton("play", this);
        auto btn_pause = new QPushButton("pause", this);
        auto btn_stop = new QPushButton("stop", this);
        auto layout1 = new QVBoxLayout;
        auto layout2 = new QHBoxLayout;

        layout1->addWidget(video);
        layout1->addLayout(layout2);
        layout2->addWidget(btn_play);
        layout2->addWidget(btn_pause);
        layout2->addWidget(btn_stop);
        layout->addLayout(layout1, n % colcount, n / colcount);

        auto core = new video_player_core();
        core->_init();
        core->_setCallBack(video);
        auto file = files[n];
        auto timer = new QTimer(this);
        timer->setInterval(1000);
        auto data = new t();
        btn_play->setUserData(0, data);
        connect(timer, &QTimer::timeout, [btn_play, core]{
            if(core->_getState() != video_player_core::state_running)
                return;
            auto data = (t*)btn_play->userData(0);
            ++data->id;
            btn_play->setText(QString("play:%1").arg(data->id));
        });

        connect(btn_play, &QPushButton::clicked, [btn_play, timer, file, core, video, this]{
            core->_stop();
            core->_setSrc(file.toStdString().c_str());

            qDebug() << "==>>start play:" << file;
            qDebug() << video->size();
//            core->_setsize(video->width(), video->height());
            core->_play();
            auto data = (t*)btn_play->userData(0);
            data->id = 0;
            timer->start();
        });

        connect(btn_pause, &QPushButton::clicked, [core, video, this]{
            if(core->_getState() == video_player_core::state_paused)
                core->_continue();
            else
                core->_pause();
        });

        connect(btn_stop, &QPushButton::clicked, [core, video, this]{
            core->_stop();
        });
    }
}

Widget::~Widget()
{
}

void Widget::resizeEvent(QResizeEvent *event)
{
    __super::resizeEvent(event);
//    if(m_core)
//        m_core->_setsize(m_img->width(), m_img->height());
}
