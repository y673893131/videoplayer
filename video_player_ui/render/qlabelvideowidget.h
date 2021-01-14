#ifndef QLABELVIDEOWIDGET_H
#define QLABELVIDEOWIDGET_H

#include <QLabel>
#include "video_player_core.h"

void yuv420p_to_rgb24(/*YUV_TYPE type, */unsigned char* yuvbuffer,unsigned char* rgbbuffer, int width,int height);
class QLabelVideoWidget : public QLabel, public video_interface
{
    Q_OBJECT
public:
    explicit QLabelVideoWidget(QWidget *parent = nullptr);

signals:

public slots:

    // video_interface interface
public:
    void totalTime(const int64_t t);
    void displayCall(void *data, int width, int height);
    void previewDisplayCall(void* data, int width, int height);
    void endCall();
};

#endif // QLABELVIDEOWIDGET_H
