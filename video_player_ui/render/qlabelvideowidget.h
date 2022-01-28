#ifndef QLABELVIDEOWIDGET_H
#define QLABELVIDEOWIDGET_H

#include <QLabel>

void yuv420p_to_rgb24(/*YUV_TYPE type, */unsigned char* yuvbuffer,unsigned char* rgbbuffer, int width,int height);
class QLabelVideoWidget : public QLabel
{
    Q_OBJECT
public:
    explicit QLabelVideoWidget(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // QLABELVIDEOWIDGET_H
