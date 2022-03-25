#include "nativeevent_uinx.h"
#ifndef Q_OS_WIN
#include <xcb/xcb.h>
#include <QDebug>

CNativeEvent_Uinx::CNativeEvent_Uinx(QFrameLessWidget *pub)
    : QFrameLessWidgetPrivate(pub)
{
}

bool CNativeEvent_Uinx::nativeEvent(const QByteArray &eventType, void *message, long *result,  QWidget* target)
{
    if(eventType != "xcb_generic_event_t")
        return false;

    auto xcb = reinterpret_cast<xcb_generic_event_t*>(message);
    auto type = xcb->response_type & ~0x80;
    if(type == XCB_GE_GENERIC)
    {
        auto event = reinterpret_cast<xcb_ge_generic_event_t*>(message);
        type = event->event_type;
    }

//    qDebug() << type;
    switch (type) {
    case XCB_BUTTON_PRESS:
        break;
    case XCB_BUTTON_RELEASE:
        break;
    case XCB_MOTION_NOTIFY:
    {
//        auto event = reinterpret_cast<xcb_motion_notify_event_t*>(message);
        checkMouseDirection(QCursor::pos());
    }break;
    case XCB_ENTER_NOTIFY:
        break;
    case XCB_LEAVE_NOTIFY:
        break;
    }
    return false;
}

bool CNativeEvent_Uinx::check(void *message, long *result)
{
    return true;
}

#endif
