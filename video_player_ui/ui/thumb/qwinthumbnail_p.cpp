#include "qwinthumbnail_p.h"
#if WIN32
#include <QtWinExtras>
#include "control/videocontrol.h"
#include <QDebug>
void QWinThumbnailPrivate::init()
{
    VP_Q(QWinThumbnail);

    QStringList icons;
    icons << ":/tool/prev_normal";
    icons << ":/tool/play_normal";
    icons << ":/tool/next_normal";

    m_taskbar = new QWinThumbnailToolBar(q);
    for(auto it : icons)
    {
        auto button = new QWinThumbnailToolButton(m_taskbar);
        button->setIcon(QIcon(it));
        m_taskbar->addButton(button);

        q->connect(button, &QWinThumbnailToolButton::clicked, q, [=]{
            Q_EMIT q->thumb(m_taskbar->buttons().indexOf(button));
        });
    }

    QTimer::singleShot(0, [=]{
        m_taskbar->setWindow(q->windowHandle());
//        auto wid = (HWND)q->winId();
//        auto ex = ::GetWindowLong(wid, GWL_EXSTYLE);
//        ex |= WS_EX_OVERLAPPEDWINDOW;
//        ex |= WS_EX_ACCEPTFILES;
//        ::SetWindowLong(wid, GWL_EXSTYLE, ex);
        auto control = VIDEO_CONTROL;
        q->connect(control, &QVideoControl::start, q, &QWinThumbnail::onStart);
        q->connect(control, &QVideoControl::continuePlay, q, &QWinThumbnail::onStart);
        q->connect(control, &QVideoControl::end, q, &QWinThumbnail::onEnd);
        q->connect(control, &QVideoControl::pausePlay, q, &QWinThumbnail::onPause);
    });

    q->connect(m_taskbarMenu, &QWinTaskbarMenu::cmd, q, &QWinThumbnail::cmd);
}

void QWinThumbnailPrivate::modifyBtn(bool bPlay)
{
    if(bPlay)
        m_taskbar->buttons()[thumb_play_or_pause]->setIcon(QIcon(":/tool/pause_normal"));
    else
        m_taskbar->buttons()[thumb_play_or_pause]->setIcon(QIcon(":/tool/play_normal"));
}

#endif
