#ifndef QGLVIDEOWIDGET_H
#define QGLVIDEOWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include "videoframe.h"

struct _texture_obj_{
    void init()
    {
        texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        texture->create();
        id = texture->textureId();
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    QOpenGLTexture* texture;
    uint id;
};

class QGLVideoWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
    enum enum_pragram_attr_index
    {
        ATTR_VERTEX_IN = 0,
        ATTR_TEXTURE_IN,
    };

    enum enum_texture_index
    {
        TEXTURE_Y = 0,
        TEXTURE_U,
        TEXTURE_V,
        TEXTURE_IMG,

        TEXTURE_MAX
    };

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
    void onAppendFrame(void*);
    void onVideoSizeChanged(int,int);
    void onStart();
    void onStop();
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initViewScale();
    void scaleViewCalc(bool bFlush = false);
private:
    QOpenGLShader* m_vShader,* m_fShader;
    QOpenGLShaderProgram* m_program;
    int m_location[TEXTURE_MAX];
    GLfloat m_vertexVertices[8], m_textureVertices[8];
    _texture_obj_ m_texture[TEXTURE_MAX];
    QSize m_videoSize;
    /*_VideoFramePtr*/_video_frame_* m_pFrame;
    bool m_bViewAdjust;
};

#endif // QGLVIDEOWIDGET_H
