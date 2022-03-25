#ifndef QLIBASSINTERFACE_H
#define QLIBASSINTERFACE_H

#include "video_pimpl.h"
extern "C"
{
#ifdef ASS_C_LOAD
    #include <ass/ass.h>
#else
    #include <ass/libass.h>
#endif
}

class QLibASSInterfacePrivate;
class QLibASSInterface
{
    VP_DECLARE(QLibASSInterface)
    VP_DECLARE_PRIVATE(QLibASSInterface)
public:
    QLibASSInterface();
    ~QLibASSInterface();

    void ass_set_message_cb(ASS_Library *priv, void (*msg_cb)
                            (int level, const char *fmt, va_list args, void *data),
                            void *data);
    ASS_Library *ass_library_init(void);
    void ass_library_done(ASS_Library *priv);
    ASS_Renderer *ass_renderer_init(ASS_Library *);
    void ass_renderer_done(ASS_Renderer *priv);
    void ass_set_fonts(ASS_Renderer *priv, const char *default_font,
                       const char *default_family, int dfp,
                       const char *config, int update);
    void ass_set_frame_size(ASS_Renderer *priv, int w, int h);
    void ass_free_track(ASS_Track *track);
    ASS_Track *ass_new_track(ASS_Library *);
    void ass_process_codec_private(ASS_Track *track, char *data, int size);
    void ass_process_data(ASS_Track *track, char *data, int size);
    ASS_Image *ass_render_frame(ASS_Renderer *priv, ASS_Track *track,
                                long long now, int *detect_change);
    void ass_flush_events(ASS_Track *track);
};

#endif // QLIBASSINTERFACE_H
