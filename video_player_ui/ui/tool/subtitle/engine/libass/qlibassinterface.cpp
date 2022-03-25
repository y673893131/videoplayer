#include "qlibassinterface.h"

#ifdef WIN32
#include <Windows.h>
#endif
#include "../apidef.h"

namespace libass{

#ifdef ASS_C_LOAD
typedef void (*msg_cb)(int, const char *, va_list, void *);

C_API_DEFINE_BEGIN

C_API_MODULE_DEFINE(h);
C_API_DEFINE(ASS_Library*, ass_library_init, void);
C_API_DEFINE(void, ass_library_done, ASS_Library*);
C_API_DEFINE(void, ass_set_message_cb, ASS_Library *, msg_cb, void *);
C_API_DEFINE(ASS_Renderer*, ass_renderer_init, ASS_Library*);
C_API_DEFINE(void, ass_renderer_done, ASS_Renderer*);
C_API_DEFINE(void, ass_set_fonts, ASS_Renderer *, const char *, const char *, int, const char *, int);
C_API_DEFINE(void, ass_set_frame_size, ASS_Renderer*, int, int);
C_API_DEFINE(void, ass_free_track, ASS_Track*);
C_API_DEFINE(ASS_Track*, ass_new_track, ASS_Library*);
C_API_DEFINE(void, ass_process_codec_private, ASS_Track *, char *, int);
C_API_DEFINE(void, ass_process_data, ASS_Track *, char *, int);
C_API_DEFINE(ASS_Image*, ass_render_frame, ASS_Renderer *, ASS_Track *, long long, int*);
C_API_DEFINE(void, ass_flush_events, ASS_Track*);

C_APC_DEFINE_END

#endif
}

#ifdef ASS_C_LOAD
#define LIBASS_CALL C_API_CALL_FUNC
#else
#define LIBASS_CALL(errorRet, space, name, ...) \
    return ::name(__VA_ARGS__);
#endif

class QLibASSInterfacePrivate : public VP_Data<QLibASSInterface>
{
    VP_DECLARE_PUBLIC(QLibASSInterface)
    inline QLibASSInterfacePrivate(QLibASSInterface* parent)
        : VP_Data(parent)
    {
        load();
    }

    ~QLibASSInterfacePrivate()
    {
        free();
    }
    void load()
    {
#ifdef ASS_C_LOAD
        using namespace libass;
        libass::h = LoadLibraryA("libass.dll");
        if(libass::h)
        {
            C_API_LOAD_FUNC(ass_set_message_cb)
            C_API_LOAD_FUNC(ass_library_init)
            C_API_LOAD_FUNC(ass_library_done)
            C_API_LOAD_FUNC(ass_renderer_init)
            C_API_LOAD_FUNC(ass_renderer_done)
            C_API_LOAD_FUNC(ass_set_fonts)
            C_API_LOAD_FUNC(ass_set_frame_size)
            C_API_LOAD_FUNC(ass_free_track)
            C_API_LOAD_FUNC(ass_new_track)
            C_API_LOAD_FUNC(ass_process_codec_private)
            C_API_LOAD_FUNC(ass_process_data)
            C_API_LOAD_FUNC(ass_render_frame)
            C_API_LOAD_FUNC(ass_flush_events)
        }
#endif
    }
    void free()
    {
#ifdef ASS_C_LOAD
        C_API_MODULE_FREE(libass::h)
#endif
    }
};

QLibASSInterface::QLibASSInterface()
    : VP_INIT(new QLibASSInterfacePrivate(this))
{
}

QLibASSInterface::~QLibASSInterface()
{
}

void QLibASSInterface::ass_set_message_cb(ASS_Library *priv, void (*msg_cb)(int, const char *, va_list, void *), void *data)
{
    LIBASS_CALL(, libass, ass_set_message_cb, priv, msg_cb, data)
}

ASS_Library *QLibASSInterface::ass_library_init()
{
    LIBASS_CALL(nullptr, libass, ass_library_init)
}

void QLibASSInterface::ass_library_done(ASS_Library *priv)
{
    LIBASS_CALL(, libass, ass_library_done, priv)
}

ASS_Renderer *QLibASSInterface::ass_renderer_init(ASS_Library *lib)
{
    LIBASS_CALL(nullptr, libass, ass_renderer_init, lib)
}

void QLibASSInterface::ass_renderer_done(ASS_Renderer *priv)
{
    LIBASS_CALL(, libass, ass_renderer_done, priv)
}

void QLibASSInterface::ass_set_fonts(ASS_Renderer *priv, const char *default_font, const char *default_family, int dfp, const char *config, int update)
{
    LIBASS_CALL(, libass, ass_set_fonts, priv, default_font, default_family, dfp, config, update)
}

void QLibASSInterface::ass_set_frame_size(ASS_Renderer *priv, int w, int h)
{
    LIBASS_CALL(, libass, ass_set_frame_size, priv , w, h)
}

void QLibASSInterface::ass_free_track(ASS_Track *track)
{
    LIBASS_CALL(, libass, ass_free_track, track)
}

ASS_Track *QLibASSInterface::ass_new_track(ASS_Library *lib)
{
    LIBASS_CALL(nullptr, libass, ass_new_track, lib)
}

void QLibASSInterface::ass_process_codec_private(ASS_Track *track, char *data, int size)
{
    LIBASS_CALL(, libass, ass_process_codec_private, track, data, size)
}

void QLibASSInterface::ass_process_data(ASS_Track *track, char *data, int size)
{
    LIBASS_CALL(, libass, ass_process_data, track, data, size)
}

ASS_Image *QLibASSInterface::ass_render_frame(ASS_Renderer *priv, ASS_Track *track, long long now, int *detect_change)
{
    LIBASS_CALL(nullptr, libass, ass_render_frame, priv, track, now, detect_change)
}

void QLibASSInterface::ass_flush_events(ASS_Track *track)
{
    LIBASS_CALL(, libass, ass_flush_events, track)
}
