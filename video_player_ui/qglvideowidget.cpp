#include "qglvideowidget.h"
#include "glvdieodefine.h"
#include "Log/Log.h"
#include "videoframe.h"
#include <QDebug>
#include <QTimer>
#include <QFile>
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
    auto bCompile = m_vShader->compileSourceFile(":/shader/img_vsh");
    if(!bCompile)
    {
        Log(Log_Err, "vertex compile failed.");
    }

    m_fShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
#ifdef FRAME_RGB
    QFile f(":/shader/img_fsh");
    if(f.open(QFile::ReadOnly))
    {
        auto s = QString("#define FRAME_RGB\n") + QString(f.readAll());
        bCompile = m_fShader->compileSourceCode(s);
        f.close();
    }
    else
        bCompile = m_fShader->compileSourceFile(":/shader/img_fsh");
#else
    bCompile = m_fShader->compileSourceFile(":/shader/img_fsh");
#endif
    if(!bCompile)
    {
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
    m_location[TEXTURE_IMG] = m_program->uniformLocation("tex_img");

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
}

void QGLVideoWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,255.0);
    glClear(GL_COLOR_BUFFER_BIT);

    auto frame = m_pFrame;
    if(frame)
    {
        m_program->bind();
#ifdef FRAME_RGB
        glActiveTexture(GL_TEXTURE0 + TEXTURE_IMG);
        glBindTexture(GL_TEXTURE_2D, m_texture[TEXTURE_IMG].id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->w, frame->h, 0, GL_RGB, GL_UNSIGNED_BYTE, frame->framebuffer);
        glUniform1i(m_location[TEXTURE_IMG], TEXTURE_IMG);
#else
        for (int index = TEXTURE_Y; index <= TEXTURE_V; ++index) {
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D, m_texture[index].id);
            int w = 0, h = 0;
            auto data = frame->data(index, w, h);
            auto p = &data[w*h - 1];
//            int nn = p - data;
//            if(index == 2)
//            {
//                QFile f("./a.yuv");
//                if(f.open(QFile::WriteOnly))
//                {
//                    f.write((char*)frame->framebuffer, frame->w * frame->h * 3 / 2);
//                    f.close();
//                }
//            }
//            qDebug() << "index:" << index << "address:" << (char*)data << nn;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
            glUniform1i(m_location[index], index);
        }
#endif
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_program->release();
    }
}

void QGLVideoWidget::totalTime(const int64_t t)
{
    qDebug() << "total: " << t;
    emit total(t / 1000);
}

void QGLVideoWidget::posChange(const int64_t t)
{
//    qDebug() << "pos: " << t;
    emit setpos(t);
}

#include "qlabelvideowidget.h"
void QGLVideoWidget::displayCall(void *data, int width, int height)
{
//    auto frame = std::make_shared<_video_frame_>((unsigned char*)data, width, height);
    auto frame = new _video_frame_((unsigned char*)data, width, height);
//    char* rgb = new char[width * height * 3];
//    memset(rgb, 0x00, width * height * 3);
//    yuv420p_to_rgb24((unsigned char*)data, (unsigned char*)rgb, width, height);
//    QImage img((uchar*)rgb, width, height, QImage::Format_RGB888);
//    img.save("C:\\Users\\Administrator\\Desktop\\help\\test\\mp4\\a.jpg", "jpg");

    emit appendFrame(frame);
}

void QGLVideoWidget::startCall(int index)
{
    qDebug() << "play start." << index;
    emit start(index);
}

void QGLVideoWidget::endCall(int index)
{
    qDebug() << "play end." << index;
    emit playOver(index);
}
