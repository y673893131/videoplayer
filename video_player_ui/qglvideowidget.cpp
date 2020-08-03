#include "qglvideowidget.h"
#include "glvdieodefine.h"
#include "Log/Log.h"
#include "videoframe.h"
#include "config.h"
#include <QDebug>
#include <QTimer>
#include <QFile>
QGLVideoWidget::QGLVideoWidget(QWidget *parent)
    :QOpenGLWidget(parent), m_pFrame(nullptr), m_bViewAdjust(GET_CONFIG_DATA(Config::Data_Adjust).toBool()), m_frameCount(0)
{
    connect(this, &QGLVideoWidget::appendFrame, this, [this](void* frame)
    {
        if(m_pFrame) delete m_pFrame;
        m_pFrame = (_video_frame_*)frame;
        update();
        ++m_frameCount;
    }, Qt::QueuedConnection);
    auto timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [this]
    {
        emit frameRate(m_frameCount);
        m_frameCount = 0;
    }, Qt::QueuedConnection);

    connect(this, &QGLVideoWidget::start, this, [timer]
    {
        timer->start();
    }, Qt::QueuedConnection);

    connect(this, &QGLVideoWidget::playOver, timer, &QTimer::stop, Qt::ConnectionType::QueuedConnection);
    connect(Config::instance(), &Config::loadConfig, [this]{
        m_bViewAdjust = GET_CONFIG_DATA(Config::Data_Adjust).toBool();
        qDebug() << "m_bViewAdjust" << m_bViewAdjust;
    });
}

QGLVideoWidget::~QGLVideoWidget()
{
    qDebug() << "gl video widget quit.";
}

void QGLVideoWidget::setVideoSize(int width, int height)
{
    m_videoSize.setWidth(width);
    m_videoSize.setHeight(height);
    if(m_bViewAdjust)
        scaleViewCalc(true);
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
    initViewScale();

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
    if(!m_videoSize.isEmpty())
        onViewAdjust(m_bViewAdjust);
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
            glUniform1i(m_location[index], index);
        }
#endif
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_program->release();
    }
}

void QGLVideoWidget::initViewScale()
{
    glVertexAttribPointer(ATTR_VERTEX_IN, 2, GL_FLOAT, 0, 0, m_vertexVertices);
    glEnableVertexAttribArray(ATTR_VERTEX_IN);
}

void QGLVideoWidget::scaleViewCalc(bool bFlush)
{
    if(m_videoSize.width() != width() || m_videoSize.height() != height() || bFlush)
    {
        do {
            static float fScaleX = 1.0;
            static float fScaleY = 1.0;

            float f0 = m_videoSize.width() * 1.0 / m_videoSize.height();
            float f1 = width() * 1.0 / height();
            float fX = 1.0;
            float fY = 1.0;
            if(f0 > f1)
                fY = f1 / f0;
            else if(f0 < f1)
                fX = f0 / f1;

            if(fScaleX == fX && fScaleY == fY && !bFlush)
                break;
            fScaleX = fX;
            fScaleY = fY;
            GLfloat vertexVertices[] = {
                -1.0f * fX, -1.0f * fY,
                 1.0f * fX, -1.0f * fY,
                 -1.0f * fX, 1.0f * fY,
                 1.0f * fX, 1.0f * fY,
            };

//            qDebug() << "scale:" << fX << fY << f0 << f1 << m_videoSize.width() << "/" << width() << m_videoSize.width() * 1.0 / width() << m_videoSize.height() << "/" << height() << m_videoSize.height() * 1.0 / height();
            memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));
            initViewScale();
        }while(0);
    }
}

void QGLVideoWidget::onViewAdjust(bool bViewAdjust)
{
    if(m_bViewAdjust != bViewAdjust)
    {
        m_bViewAdjust = bViewAdjust;
        if(!m_bViewAdjust)
        {
            GLfloat vertexVertices[] = {
                -1.0f, -1.0f,
                1.0f, -1.0f,
                -1.0f, 1.0f,
                1.0f, 1.0f,
            };

            memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));
            initViewScale();
        }
        else
            scaleViewCalc(true);
        update();
        return;
    }

    scaleViewCalc();
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
