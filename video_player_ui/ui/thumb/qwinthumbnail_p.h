#ifndef QWINTHUMBNAILPRIVATE_H
#define QWINTHUMBNAILPRIVATE_H

#if WIN32
#include "qwintaskbarmenu.h"
#include "qwinthumbnailtoolbar.h"
#include "framelesswidget/nativeevent_p.h"
#include "qwinthumbnail.h"

class QWinThumbnail;
class QWinThumbnailPrivate : public CNativeEvent_p
{
    VP_DECLARE_PUBLIC(QWinThumbnail)
public:
    inline QWinThumbnailPrivate(QWinThumbnail* parent)
        : CNativeEvent_p(parent)
        , m_taskbarMenu(new QWinTaskbarMenu)
        , m_taskbar(nullptr)
    {
    }

    ~QWinThumbnailPrivate()
    {
    }

    void init();
    void modifyBtn(bool bPlay);
private:
    QWinTaskbarMenu* m_taskbarMenu;
    QWinThumbnailToolBar* m_taskbar;
};

#endif
#endif // QWINTHUMBNAILPRIVATE_H
