#include "qglvideowidget.h"
#include "Log/Log.h"
#include "videoframe.h"
#include "config/config.h"
#include "framelesswidget/util.h"
#include <QDebug>
#include <QTimer>
#include <QFile>
//#include <QPushButton>
#include "qrenderprivate.h"

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

class QGLVideoWidgetPrivate : public VP_Data<QGLVideoWidget>, public QRenderPrivate
{
    VP_DECLARE_PUBLIC(QGLVideoWidget)
    inline QGLVideoWidgetPrivate(QGLVideoWidget* parent)
        : VP_Data(parent)
        , QRenderPrivate()
    {
    }

    ~QGLVideoWidgetPrivate()
    {
        if(m_freqLast)
        {
            delete m_freqLast;
            m_freqLast = nullptr;
        }
    }

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

    void init();
    void initViewScale();
    void scaleViewCalc(bool bFlush = false);
    bool paintImage();
    void paintFreq();
    void remove();
private:
    QOpenGLShader* m_vShader,* m_fShader;
    QOpenGLShaderProgram* m_program;
    int m_location[TEXTURE_MAX];
    GLfloat m_vertexVertices[8], m_textureVertices[8];
    _texture_obj_ m_texture[TEXTURE_MAX];
};

void QGLVideoWidgetPrivate::init()
{
    VP_Q(QGLVideoWidget);

    q->initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // program
    m_vShader = new QOpenGLShader(QOpenGLShader::Vertex, q);
    auto bCompile = m_vShader->compileSourceFile(":/shader/img_vsh");
    if(!bCompile)
    {
        Log(Log_Err, "vertex compile failed.");
    }

    m_fShader = new QOpenGLShader(QOpenGLShader::Fragment, q);
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

    m_program = new QOpenGLShaderProgram(q);
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
    q->glVertexAttribPointer(ATTR_TEXTURE_IN, 2, GL_FLOAT, 0, 0, m_textureVertices);
    q->glEnableVertexAttribArray(ATTR_TEXTURE_IN);

    // texture obj
    for (int n = 0; n < TEXTURE_MAX; ++n)
        m_texture[n].init();
}

void QGLVideoWidgetPrivate::initViewScale()
{
    VP_Q(QGLVideoWidget);
    q->glVertexAttribPointer(ATTR_VERTEX_IN, 2, GL_FLOAT, 0, 0, m_vertexVertices);
    q->glEnableVertexAttribArray(ATTR_VERTEX_IN);
}

void QGLVideoWidgetPrivate::scaleViewCalc(bool bFlush)
{
    VP_Q(QGLVideoWidget);
    if(setScale(q->width(), q->height()) || bFlush)
    {
        GLfloat vertexVertices[] = {
            -1.0f * m_fScaleX, -1.0f * m_fScaleY,
             1.0f * m_fScaleX, -1.0f * m_fScaleY,
             -1.0f * m_fScaleX, 1.0f * m_fScaleY,
             1.0f * m_fScaleX, 1.0f * m_fScaleY,
        };

        qDebug() << "scale:" << m_fScaleX << m_fScaleY;
        memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));
        initViewScale();
    }
}

void QGLVideoWidgetPrivate::paintFreq()
{
    VP_Q(QGLVideoWidget);
    if(!m_freq)
        return;
    glLineWidth(4);
    glBegin(GL_LINES);
    glColor3f(1.0f,0.36f,0.22f);
    unsigned int count = m_freqCount;
    auto size = CALC_WIDGET_SIZE(nullptr, count / 64 * 400, 200);
    float maxHeight = size.height() / (1.0f * q->height());
    float maxWidth = size.width() / (1.0f * q->width());
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
#include "control/videocontrol.h"
bool QGLVideoWidgetPrivate::paintImage()
{
    VP_Q(QGLVideoWidget);
    auto frame = m_frame.get();
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
            q->glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + index));
            glBindTexture(GL_TEXTURE_2D, m_texture[index].id);
            unsigned int w = 0, h = 0;
            auto data = frame->data(index, w, h);
//            qDebug() << width() << w << height() << h;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(w), static_cast<int>(h), 0, GL_RED, GL_UNSIGNED_BYTE, data);
            q->glUniform1i(m_location[index], index);
        }

#endif
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_program->release();
        return true;
    }

    return false;
}

void QGLVideoWidgetPrivate::remove()
{
    if(m_frame)
    {
        m_frame.reset();
    }

    if(m_freqLast)
    {
        delete m_freqLast;
        m_freqLast = nullptr;
    }

}

QGLVideoWidget::QGLVideoWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , VP_INIT(new QGLVideoWidgetPrivate(this))
//    , m_pFrame(nullptr)
{

    auto func = [this]
    {
        VP_D(QGLVideoWidget);
        d->setScaleType(GET_CONFIG_DATA(Config::Data_PlaySize).toInt());
        d->scaleViewCalc(true);
    };

    connect(Config::instance(), &Config::loadConfig, this, func);
    connect(Config::instance(), &Config::setConfig, this, func);
    setWindowFlag(Qt::FramelessWindowHint);

//    auto btn = new QPushButton("test_button", parent);
//    btn->setFixedSize(200,200);
//    btn->setStyleSheet("QPushButton{border-image:url(./sex_boy.png);color:red;}");
}

QGLVideoWidget::~QGLVideoWidget()
{
    qDebug() << "gl video widget quit.";
}

void QGLVideoWidget::initializeGL()
{
    VP_D(QGLVideoWidget);
    d->init();
}

void QGLVideoWidget::resizeGL(int w, int h)
{
    VP_D(QGLVideoWidget);
    glViewport(0, 0, w, h);
    d->setWindowSize(w, h);
    d->scaleViewCalc(true);
//    if(!m_videoSize.isEmpty())
//        onViewAdjust(m_bViewAdjust);
}

void QGLVideoWidget::paintGL()
{
    VP_D(QGLVideoWidget);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0,0.0,0.0,255.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!d->paintImage())
        d->paintFreq();
}

void QGLVideoWidget::onViewAdjust(bool bViewAdjust)
{
    VP_D(QGLVideoWidget);
//    if(m_bViewAdjust != bViewAdjust)
//    {
//        m_bViewAdjust = bViewAdjust;
//        if(!m_bViewAdjust)
//        {
//            GLfloat vertexVertices[] = {
//                -1.0f, -1.0f,
//                1.0f, -1.0f,
//                -1.0f, 1.0f,
//                1.0f, 1.0f,
//            };

//            memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));
//            initViewScale();
//        }
//        else
//            scaleViewCalc(true);
//        update();
//        return;
//    }

    d->scaleViewCalc();
}

void QGLVideoWidget::onAppendFrame(_VideoFramePtr frame)
{
    VP_D(QGLVideoWidget);
    d->m_frame = frame;
    update();
}

void QGLVideoWidget::onAppendFreq(float *data, unsigned int size)
{
    VP_D(QGLVideoWidget);
    d->m_freq = data;
    d->m_freqCount = size;
    update();
}

void QGLVideoWidget::onVideoSizeChanged(int width, int height)
{
    VP_D(QGLVideoWidget);
    d->setVideoSize(width, height);
    d->scaleViewCalc(true);
//    if(m_bViewAdjust)
//        scaleViewCalc(true);
}

void QGLVideoWidget::onStart()
{
}

void QGLVideoWidget::onStop()
{
    VP_D(QGLVideoWidget);
    d->remove();
    update();
}
