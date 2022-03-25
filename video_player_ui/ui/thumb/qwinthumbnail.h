#ifndef QWINTHUMBNAIL_H
#define QWINTHUMBNAIL_H

#if WIN32
#include <QWidget>
#include "framelesswidget/framelesswidget.h"
#include "video_pimpl.h"

enum HBitmapFormat
{
    NoAlpha,
    PremultipliedAlpha,
    Alpha
};


enum thumb_type
{
    thumb_prev,
    thumb_play_or_pause,
    thumb_next,


    thumb_max
};

enum cmd_type
{
    cmd_type_prev,
    cmd_type_next,
    cmd_type_clean,
    cmd_type_stop,

    cmd_type_play,

    cmd_type_max
};

class QWinThumbnailPrivate;
class QWinThumbnail : public QFrameLessWidget
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QWinThumbnail)
public:
    QWinThumbnail(QWinThumbnailPrivate* pri, QWidget* parent);
    ~QWinThumbnail() override;
signals:
    void thumb(int);
    void cmd(int, const QString&);

public slots:
    void onStart();
    void onPause();
    void onEnd();
};
#endif

#endif // QWINTHUMBNAIL_H
