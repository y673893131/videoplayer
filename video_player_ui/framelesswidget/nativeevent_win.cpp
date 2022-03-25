#include "nativeevent_win.h"

#ifdef Q_OS_WIN

#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>

CNativeEvent_Win::CNativeEvent_Win(QFrameLessWidget* pub)
    : QFrameLessWidgetPrivate(pub)
    , m_bShadow(false)
{

}

CNativeEvent_Win::~CNativeEvent_Win()
{
}

void CNativeEvent_Win::setAreo(void *wnd)
{
#ifdef Q_OS_WIN
    auto hwnd = reinterpret_cast<HWND>(wnd);
    LONG style = ::GetWindowLong(hwnd, GWL_STYLE);
    style |= WS_THICKFRAME;
    style |= WS_CAPTION;
    style |= WS_MAXIMIZEBOX;
    ::SetWindowLong(hwnd, GWL_STYLE, style);

//    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
//    SetLayeredWindowAttributes(hwnd, RGB(255, 255, 255), 254, LWA_ALPHA);
#endif
}

void CNativeEvent_Win::setShadow(void *wnd)
{
#ifdef Q_OS_WIN
    m_bShadow = true;
    auto hwnd = reinterpret_cast<HWND>(wnd);
    int w = 1;
    const MARGINS shadow = {w, w, w, w};
    DwmExtendFrameIntoClientArea(hwnd, &shadow);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
#endif
}

bool CNativeEvent_Win::check(void *message, long *result)
{
    auto msg = reinterpret_cast<MSG*>(message);
#define PADDING 10
    const LONG borderWidth = PADDING;
    RECT winrect;
    GetWindowRect(msg->hwnd, &winrect);
    long x = GET_X_LPARAM(msg->lParam);
    long y = GET_Y_LPARAM(msg->lParam);

    // bottom left
    if (x >= winrect.left && x < winrect.left + borderWidth &&
            y < winrect.bottom && y >= winrect.bottom - borderWidth)
    {
        *result = HTBOTTOMLEFT;
        return true;
    }

    // bottom right
    if (x < winrect.right && x >= winrect.right - borderWidth &&
            y < winrect.bottom && y >= winrect.bottom - borderWidth)
    {
        *result = HTBOTTOMRIGHT;
        return true;
    }

    // top left
    if (x >= winrect.left && x < winrect.left + borderWidth &&
            y >= winrect.top && y < winrect.top + borderWidth)
    {
        *result = HTTOPLEFT;
        return true;
    }

    // top right
    if (x < winrect.right && x >= winrect.right - borderWidth &&
            y >= winrect.top && y < winrect.top + borderWidth)
    {
        *result = HTTOPRIGHT;
        return true;
    }

    // left
    if (x >= winrect.left && x < winrect.left + borderWidth)
    {
        *result = HTLEFT;
        return true;
    }

    // right
    if (x < winrect.right && x >= winrect.right - borderWidth)
    {
        *result = HTRIGHT;
        return true;
    }

    // bottom
    if (y < winrect.bottom && y >= winrect.bottom - borderWidth)
    {
        *result = HTBOTTOM;
        return true;
    }

    // top
    if (y >= winrect.top && y < winrect.top + borderWidth)
    {
        *result = HTTOP;
        return true;
    }

    return false;
}

bool CNativeEvent_Win::nativeEvent(const QByteArray &/*eventType*/, void *message, long *result, QWidget* widget)
{
#ifdef Q_OS_WIN
    auto msg = reinterpret_cast<MSG*>(message);
    switch (msg->message) {
    case WM_NCHITTEST:
    {
        bool bCheck = false;
        if(m_bResizeabe && !widget->isMaximized() && !widget->isFullScreen())
        {
            bCheck = check(message, result);
        }

        if(!bCheck && isCaption())
        {
            *result = HTCAPTION;
            return true;
        }

        return bCheck;
    }
    case WM_NCCALCSIZE:
    {
        *result = 0;
        return true;
    }
    case WM_GETMINMAXINFO:
    {
//        if(!m_bShadow)
//        {
//            break;
//        }

        if (::IsZoomed(msg->hwnd)) {
            RECT frame = { 0, 0, 0, 0 };
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
            frame.left = abs(frame.left);
            frame.top = abs(frame.bottom);
            widget->setContentsMargins(frame.left, frame.top, frame.right, frame.bottom);
        }
        else {
            int w = 0;
            widget->setContentsMargins(w, w, w, w);
        }

        *result = ::DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        return true;
    }
    }
#endif
    return false;
}

bool CNativeEvent_Win::isCaption()
{
    return false;
}

#endif
