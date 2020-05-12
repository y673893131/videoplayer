#include "qglvideowidget.h"
#include "glvdieodefine.h"
#include "Log/Log.h"
#include "videoframe.h"
#include <QDebug>
QGLVideoWidget::QGLVideoWidget(QWidget *parent)
    :QOpenGLWidget(parent), m_pFrame(nullptr)
{
    connect(this, &QGLVideoWidget::appendFrame, this, [this](void* frame)
    {
        if(m_pFrame) delete m_pFrame;
        m_pFrame = (_video_frame_*)frame;
        update();
    }, Qt::ConnectionType::QueuedConnection);
}

QGLVideoWidget::~QGLVideoWidget()
{
    qDebug() << "gl video widget quit.";
}

void QGLVideoWidget::setVideoSize(int w, int h)
{
    m_videoSize.setWidth(w);
    m_videoSize.setHeight(h);
}

void QGLVideoWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    // program
    m_vShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    auto bCompile = m_vShader->compileSourceCode(vertexSrc);
    if(!bCompile){
        Log(Log_Err, "vertex compile failed.");
    }

    m_fShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    bCompile = m_fShader->compileSourceCode(fragmentSrc);
    if(!bCompile){
        Log(Log_Err, "fragment compile failed.");
    }

    m_program = new QOpenGLShaderProgram(this);
    m_program->addShader(m_fShader);
    m_program->addShader(m_vShader);
    m_program->bindAttributeLocation("vertexIn", ATTR_VERTEX_IN);
    m_program->bindAttributeLocation("textureIn", ATTR_TEXTURE_IN);
    m_program->link();

    m_location[TEXTURE_Y] = m_program->uniformLocation("tex_y");
    m_location[TEXTURE_U] = m_program->uniformLocation("tex_u");
    m_location[TEXTURE_V] = m_program->uniformLocation("tex_v");

    // vertex/texture vertices value
    GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         -1.0f, 1.0f,
         1.0f, 1.0f,
    };

    memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));
    glVertexAttribPointer(ATTR_VERTEX_IN, 2, GL_FLOAT, 0, 0, m_vertexVertices);
    glEnableVertexAttribArray(ATTR_VERTEX_IN);

    GLfloat textureVertices[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };

    memcpy(m_textureVertices, textureVertices, sizeof(textureVertices));
    glVertexAttribPointer(ATTR_TEXTURE_IN, 2, GL_FLOAT, 0, 0, m_textureVertices);
    glEnableVertexAttribArray(ATTR_TEXTURE_IN);

    // texture obj
    for (int n = 0; n < TEXTURE_MAX; ++n)
        m_texture[n].init();
}

void QGLVideoWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
//    if(w && h){
//        if(!h) h = 1;
//        if(!w) w = 1;
//        auto wScale = w * 1.0 / width();
//        auto hScale = h * 1.0 / height();
//        GLfloat scale = qMin(wScale, hScale);

//        GLfloat vertexVertices[] = {
//            -1.0f, -1.0f,
//             1.0f, -1.0f,
//            -1.0f, 1.0f,
//             1.0f, 1.0f,
//        };

//        memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));

//    }
}

void QGLVideoWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,255.0);
    glClear(GL_COLOR_BUFFER_BIT);

    auto frame = m_pFrame;
//    qDebug() << "paint: "<< frame;
    if(frame)
    {
        m_program->bind();
        for (int index = 0; index < TEXTURE_MAX; ++index) {
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D, m_texture[index].id);
            int w = 0, h = 0;
            auto data = frame->data(index, w, h);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
            glUniform1i(m_location[index], index);
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_program->release();
    }
}

void QGLVideoWidget::totalTime(const _int64 t)
{
    qDebug() << "total: " << t;
}
#include <QTimer>
void QGLVideoWidget::displayCall(void *data, int width, int height)
{
//    auto frame = std::make_shared<_video_frame_>((unsigned char*)data, width, height);
    auto frame = new _video_frame_((unsigned char*)data, width, height);
    emit appendFrame(frame);
}

void QGLVideoWidget::endCall()
{
    qDebug() << "play end.";
}
