#ifndef QLABELVIDEOWIDGET_H
#define QLABELVIDEOWIDGET_H

#include <QLabel>
#include "video_player_core.h"

class QLabelVideoWidget : public QLabel, public video_interface
{
    Q_OBJECT
public:
    explicit QLabelVideoWidget(QWidget *parent = nullptr);

signals:

public slots:

    // video_interface interface
public:
    void totalTime(const _int64 t);
    void displayCall(void *data, int width, int height);
    void endCall();
};

#endif // QLABELVIDEOWIDGET_H
