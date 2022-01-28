#include "qglvideowidget.h"
#include "Log/Log.h"
#include "videoframe.h"
#include "config/config.h"
#include "framelesswidget/util.h"
#include <QDebug>
#include <QTimer>
#include <QFile>
//#include <QPushButton>
QGLVideoWidget::QGLVideoWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_pFrame(nullptr)
    , m_freq(nullptr)
    , m_freqLast(nullptr)
    , m_bViewAdjust(GET_CONFIG_DATA(Config::Data_Adjust).toBool())
{
    connect(Config::instance(), &Config::loadConfig, this, [this]
    {
        m_bViewAdjust = GET_CONFIG_DATA(Config::Data_Adjust).toBool();
        qDebug() << "m_bViewAdjust" << m_bViewAdjust;
    });
    setWindowFlag(Qt::FramelessWindowHint);

//    auto btn = new QPushButton("test_button", parent);
//    btn->setFixedSize(200,200);
//    btn->setStyleSheet("QPushButton{border-image:url(./sex_boy.png);color:red;}");
}

QGLVideoWidget::~QGLVideoWidget()
{
    qDebug() << "gl video widget quit.";
    if(m_freqLast)
    {
        delete m_freqLast;
        m_freqLast = nullptr;
    }
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

    if(!paintImage())
        paintFreq();
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
            static float fScaleX = 1.0f;
            static float fScaleY = 1.0f;

            float f0 = m_videoSize.width() * 1.0f / m_videoSize.height();
            float f1 = width() * 1.0f / height();
            float fX = 1.0f;
            float fY = 1.0f;
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

            qDebug() << "scale:" << fX << fY << f0 << f1 << m_videoSize.width() << "/" << width() << m_videoSize.width() * 1.0 / width() << m_videoSize.height() << "/" << height() << m_videoSize.height() * 1.0 / height();
            memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));
            initViewScale();
        }while(0);
    }
}

bool QGLVideoWidget::paintImage()
{
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
            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + index));
            glBindTexture(GL_TEXTURE_2D, m_texture[index].id);
            unsigned int w = 0, h = 0;
            auto data = frame->data(index, w, h);
//            qDebug() << width() << w << height() << h;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(w), static_cast<int>(h), 0, GL_RED, GL_UNSIGNED_BYTE, data);
            glUniform1i(m_location[index], index);
        }

#endif
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_program->release();
        return true;
    }

    return false;
}

void QGLVideoWidget::paintFreq()
{
    if(!m_freq)
        return;
    glLineWidth(4);
    glBegin(GL_LINES);
    glColor3f(1.0f,0.36f,0.22f);
    unsigned int count = m_size;
    auto size = CALC_WIDGET_SIZE(nullptr, count / 64 * 400, 200);
    float maxHeight = size.height() / (1.0f * this->height());
    float maxWidth = size.width() / (1.0f * this->width());
    float xstart = -1.0;
    float ystart = -0.5;
    float xpos = xstart;

    float xstep = maxWidth / count;
    for(unsigned int k = 0; k < count; k++ )
    {
        xpos += xstep;
        glVertex2f(xpos, ystart);
        glVertex2f(xpos, m_freq[k] * maxHeight + ystart);
        xpos += xstep;
    }

    glColor4f(0.9f,0.2f,0.2f,0.5f);
    xpos = xstart;
    for(unsigned int k = 0; k < count; k++ )
    {
        xpos += xstep;
        glVertex2f(xpos, -(m_freq[k] * maxHeight) * 0.6f + ystart);
        glVertex2f(xpos, ystart);
        xpos += xstep;
    }

    xpos = xstart;
    glColor4f(1.0f,0.5f,0.5f, 0.5f);
    auto mH = maxHeight * 0.04f;
    if(!m_freqLast) m_freqLast = new float[count];
    for(unsigned int k = 0; k < count; k++ )
    {
        xpos += xstep;
        if(m_freqLast[k] >= mH)
        {
            glVertex2f(xpos, m_freqLast[k]*maxHeight + ystart - mH);
            glVertex2f(xpos, m_freqLast[k]*maxHeight + ystart);
        }
        xpos += xstep;
    }

    glEnd();
    memcpy(m_freqLast, m_freq, sizeof(float) * count);
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

void QGLVideoWidget::onAppendFrame(void *frame)
{
    if(m_pFrame) delete m_pFrame;
    m_pFrame = reinterpret_cast<_video_frame_*>(frame);
    update();
}

void QGLVideoWidget::onAppendFreq(float *data, unsigned int size)
{
    if(m_freq) {
        delete m_freq;
        m_freq = nullptr;
    }
    m_freq = data;
    m_size = size;
    update();
}

void QGLVideoWidget::onVideoSizeChanged(int width, int height)
{
    m_videoSize.setWidth(width);
    m_videoSize.setHeight(height);
    if(m_bViewAdjust)
        scaleViewCalc(true);
}

void QGLVideoWidget::onStart()
{
}

void QGLVideoWidget::onStop()
{
    if(m_pFrame)
    {
        delete m_pFrame;
        m_pFrame = nullptr;
        update();
    }
    if(m_freq)
    {
        delete m_freq;
        m_freq = nullptr;
        update();
    }

    if(m_freqLast)
    {
        delete m_freqLast;
        m_freqLast = nullptr;
        update();
    }
}
