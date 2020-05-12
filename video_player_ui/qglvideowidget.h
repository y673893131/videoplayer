#ifndef QGLVIDEOWIDGET_H
#define QGLVIDEOWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include "videoframe.h"
#include "video_player_core.h"

struct _texture_obj_{
    void init(){
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

class QGLVideoWidget : public QOpenGLWidget, public QOpenGLFunctions, public video_interface
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

        TEXTURE_MAX
    };

public:
    QGLVideoWidget(QWidget *parent = nullptr);
    virtual ~QGLVideoWidget();

    void setVideoSize(int w, int h);
    // QOpenGLWidget interface
signals:
    void appendFrame(void*);
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void totalTime(const _int64 t);
    void displayCall(void *data, int width, int height);
    void endCall();
private:
    QOpenGLShader* m_vShader,* m_fShader;
    QOpenGLShaderProgram* m_program;
    int m_location[TEXTURE_MAX];
    GLfloat m_vertexVertices[8], m_textureVertices[8];
    _texture_obj_ m_texture[TEXTURE_MAX];
    QSize m_videoSize;
    /*_VideoFramePtr*/_video_frame_* m_pFrame;

    video_player_core* m_core;
};

#endif // QGLVIDEOWIDGET_H
