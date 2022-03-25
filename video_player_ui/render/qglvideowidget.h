#ifndef QGLVIDEOWIDGET_H
#define QGLVIDEOWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include "videoframe.h"
#include "video_pimpl.h"

class QGLVideoWidgetPrivate;
class QGLVideoWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QGLVideoWidget)

public:
    QGLVideoWidget(QWidget *parent = nullptr);
    virtual ~QGLVideoWidget() override;

signals:
    void appendFrame(void*);
    void playOver(int);
    void start(int);
    void pause(int);
    void total(int);
    void setpos(int);
    void frameRate(int);
public slots:
    void onViewAdjust(bool);
    void onAppendFrame(_VideoFramePtr);
    void onAppendFreq(float*, unsigned int);
    void onVideoSizeChanged(int,int);
    void onStart();
    void onStop();
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    VP_DECLARE(QGLVideoWidget)
};

#endif // QGLVIDEOWIDGET_H
