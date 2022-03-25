#include "libass.h"
#include <QDebug>
#include <QImage>
#include <QFile>
extern "C"
{
    #include <ass\ass.h>
}

#ifdef WIN32
#include <Windows.h>
#endif
#include <Log/Log.h>

class LibassPrivate
{
    Q_DECLARE_PUBLIC(Libass)
    inline LibassPrivate(Libass* parent)
        : q_ptr(parent)
        , m_assLib(nullptr)
        , m_assRender(nullptr)
        , m_assTrack(nullptr)
        , m_bFlush(false)
    {

    }

    ~LibassPrivate()
    {
        if(m_assLib)
        {
            ass_library_done(m_assLib);
            m_assLib = nullptr;
        }

        if(m_assRender)
        {
            ass_renderer_done(m_assRender);
            m_assRender = nullptr;
        }

        if(m_assTrack)
        {
            ass_free_track(m_assTrack);
            m_assTrack = nullptr;
        }
    }

private:
    Libass* const q_ptr;

    ASS_Library* m_assLib;
    ASS_Renderer* m_assRender;
    ASS_Track* m_assTrack;
    QImage m_image;
    bool m_bFlush;
};


Libass::Libass(QObject* parent)
    : QObject(parent)
    , d_ptr(new LibassPrivate(this))

{
    initAAS();
}

Libass::~Libass()
{

}

QImage &Libass::image()
{
    Q_D(Libass);
    return d->m_image;
}

bool Libass::isFlush()
{
    Q_D(Libass);
    auto bFlush = d->m_bFlush;
    d->m_bFlush = false;
    return bFlush;
}

void Libass::callback(int level, const char *fmt, va_list va, void *data)
{
#define MSGL_FATAL 0
#define MSGL_ERR 1
#define MSGL_WARN 2
#define MSGL_INFO 4
#define MSGL_V 6
#define MSGL_DBG2 7

    if (level > MSGL_WARN)
        return;

    auto pThis = reinterpret_cast<Libass*>(data);
    char buffer[1024];
#ifdef WIN32
    _vsnprintf_s(buffer, 1024, fmt, va);
#else
    _snprintf(buffer, 1024, fmt, va);
#endif
    auto s = QString::fromUtf8(buffer);
    LogB(Log_Debug,"[ass:%d] %s", level, s.toLocal8Bit().toStdString().c_str());
    emit pThis->reportError(s);
}

void Libass::initStreamHeader(const QString& header)
{
    auto data = header.toUtf8();
    initTrack(data);
}

void Libass::initAASFile(const QString &sFile)
{
    QFile file(sFile);
    if(file.open(QFile::ReadOnly))
    {
        auto data = file.readAll();
        initTrack(data);
        file.close();
    }
}

void Libass::initTrack(const QByteArray &data)
{
    Q_D(Libass);
    if(d->m_assTrack)
    {
        ass_free_track(d->m_assTrack);
        d->m_assTrack = nullptr;
    }
    d->m_assTrack = ass_new_track(d->m_assLib);
    ass_process_codec_private(d->m_assTrack, const_cast<char*>(data.constData()), data.size());
}

#define _r(c)  ((c)>>24)
#define _g(c)  (((c)>>16)&0xFF)
#define _b(c)  (((c)>>8)&0xFF)
#define _a(c)  ((c)&0xFF)

void Libass::render(const QString &aas, int pts, int dur)
{
    Q_D(Libass);
    if(d->m_assRender)
    {
        if(d->m_assTrack)
        {
            auto data = aas.toUtf8();
//            ass_process_chunk(d->m_assTrack, data.data(), data.size(), pts, dur);
            ass_process_data(d->m_assTrack, data.data(), data.size());
        }

        int changed = 0;
        if(d->m_assTrack->n_events)
        {
            pts = static_cast<int>(d->m_assTrack->events[0].Start);
        }

        auto image = ass_render_frame(d->m_assRender, d->m_assTrack, pts, &changed);

        if(!image)
        {
            qDebug() << "image is null" << changed << pts << dur << aas;
            return;
        }

        d->m_bFlush = false;
//        qDebug() << changed << image << image->w << image->h << d->m_assTrack->n_events;
        if(image)
        {
            ASS_Image *i = image;
            QImage& img = d->m_image;
            unsigned char *dst = img.bits();
            while(i)
            {
                int x, y;

                auto r = (i->color & 0xff000000) >> 24;
                auto g = (i->color & 0x00ff0000) >> 16;
                auto b = (i->color & 0x0000ff00) >> 8;

                unsigned char *src = i->bitmap;
                dst = img.bits() + i->dst_y * img.width() * 4 + i->dst_x * 4;
                for (y = 0; y < i->h; ++y) {
                    for (x = 0; x < i->w; ++x) {
                        auto alpha = (255 - src[x]) / 255.0f;
                        auto pixel = dst + x * 4;
                        pixel[0] = static_cast<unsigned char>((1 - alpha) * b + alpha * pixel[0]);
                        pixel[1] = static_cast<unsigned char>((1 - alpha) * g + alpha * pixel[1]);
                        pixel[2] = static_cast<unsigned char>((1 - alpha) * r + alpha * pixel[2]);
                        pixel[3] = static_cast<unsigned char>((1 - alpha) * src[x] + alpha * pixel[3]);
                    }
                    src += i->stride;
                    dst += img.width() * 4;
                }

                i = i->next;
            }

//            img.save("1.png", "png");
        }
    }
}

void Libass::setFrameSize(int w, int h)
{
    Q_D(Libass);
    if(d->m_assRender)
    {
        ass_set_frame_size(d->m_assRender, w, h);
        d->m_image = QImage(QSize(w, h), QImage::Format_ARGB32);
        d->m_image.fill(Qt::transparent);
    }
}

void Libass::flush()
{
    Q_D(Libass);
    if(d->m_assTrack)
    {
        d->m_image.fill(Qt::transparent);
        ass_flush_events(d->m_assTrack);
        d->m_bFlush = true;
    }
}

void Libass::initAAS()
{
    Q_D(Libass);

    if(d->m_assLib)
        return;

    // init ass
    if(!d->m_assLib)
    {
        d->m_assLib = ass_library_init();
        ass_set_message_cb(d->m_assLib, callback, this);
    }

    if(!d->m_assRender)
    {
        d->m_assRender = ass_renderer_init(d->m_assLib);
    }

    ass_set_fonts(d->m_assRender, nullptr, "Arial",
                  ASS_FONTPROVIDER_AUTODETECT, nullptr, 1);
}
