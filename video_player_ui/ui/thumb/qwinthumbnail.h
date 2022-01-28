#ifndef QWINTHUMBNAIL_H
#define QWINTHUMBNAIL_H

#if WIN32
#include <QWidget>
# include <ShObjIdl.h>
#include "framelesswidget/framelesswidget.h"

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

class QWinThumbnail : public QFrameLessWidget
{
    Q_OBJECT
public:
    explicit QWinThumbnail(QWidget* parent);
    ~QWinThumbnail() override;
signals:
    void thumb(int);

public slots:
    void onStart();
    void onPause();
    void onEnd();

private:
    void init();
    void modifyBtn(bool bPlay);
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    UINT taskbar_wmsg;
    HIMAGELIST himl;
    ITaskbarList3 *p_taskbl;
};
#endif

#endif // QWINTHUMBNAIL_H
