#include "qwinthumbnail.h"

#if WIN32
#include <QPixmap>
#include <QBitmap>
#include <QMetaType>
#include <Windows.h>
#include "Log/Log.h"
#include "control/videocontrol.h"
Q_GUI_EXPORT HBITMAP qt_pixmapToWinHBITMAP(const QPixmap &p, int hbitmapFormat = 0);
#pragma comment(lib, "comctl32.lib")

QWinThumbnail::QWinThumbnail(QWidget* parent)
    : QFrameLessWidget(parent)
    , himl(nullptr)
    , p_taskbl(nullptr)
{
    taskbar_wmsg = RegisterWindowMessage(TEXT("TaskbarButtonCreated"));
    if (taskbar_wmsg == 0)
    {
        Log(Log_Err, "Failed to register QWinThumbnail message");
    }
}

QWinThumbnail::~QWinThumbnail()
{
    if(himl)
        ImageList_Destroy(himl);
    if(p_taskbl)
        p_taskbl->Release();
    CoUninitialize();
}

void QWinThumbnail::init()
{
    p_taskbl = nullptr;
    himl = nullptr;

    HRESULT hr = CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED );
    if( FAILED(hr) )
        return;

    void *pv;
    hr = CoCreateInstance( CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
                           IID_ITaskbarList3, &pv);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return;
    }

    p_taskbl = (ITaskbarList3 *)pv;
    p_taskbl->HrInit();

    int iconX = GetSystemMetrics(SM_CXSMICON);
    int iconY = GetSystemMetrics(SM_CYSMICON);
    himl = ImageList_Create( iconX /*cx*/, iconY /*cy*/, ILC_COLOR32 /*flags*/,
                             4 /*cInitial*/, 0 /*cGrow*/);
    if( himl == nullptr )
    {
        p_taskbl->Release();
        p_taskbl = nullptr;
        CoUninitialize();
        return;
    }

    QPixmap img   = QPixmap(":/tool/prev_normal").scaled( iconX, iconY );
    QPixmap img2  = QPixmap(":/tool/pause_normal").scaled( iconX, iconY );
    QPixmap img3  = QPixmap(":/tool/play_normal").scaled( iconX, iconY );
    QPixmap img4  = QPixmap(":/tool/next_normal").scaled( iconX, iconY );
    QBitmap mask  = img.createMaskFromColor(Qt::transparent);
    QBitmap mask2 = img2.createMaskFromColor(Qt::transparent);
    QBitmap mask3 = img3.createMaskFromColor(Qt::transparent);
    QBitmap mask4 = img4.createMaskFromColor(Qt::transparent);

    if( -1 == ImageList_Add(himl, qt_pixmapToWinHBITMAP(img, PremultipliedAlpha), qt_pixmapToWinHBITMAP(mask)))
        Log(Log_Err, "%s ImageList_Add failed", "First" );
    if( -1 == ImageList_Add(himl, qt_pixmapToWinHBITMAP(img2, PremultipliedAlpha), qt_pixmapToWinHBITMAP(mask2)))
        Log(Log_Err, "%s ImageList_Add failed", "Second" );
    if( -1 == ImageList_Add(himl, qt_pixmapToWinHBITMAP(img3, PremultipliedAlpha), qt_pixmapToWinHBITMAP(mask3)))
        Log(Log_Err, "%s ImageList_Add failed", "Third" );
    if( -1 == ImageList_Add(himl, qt_pixmapToWinHBITMAP(img4, PremultipliedAlpha), qt_pixmapToWinHBITMAP(mask4)))
        Log(Log_Err, "%s ImageList_Add failed", "Fourth" );

    // Define an array of two buttons. These buttons provide images through an
    // image list and also provide tooltips.
    THUMBBUTTONMASK dwMask = THUMBBUTTONMASK(THB_BITMAP | THB_FLAGS);
    THUMBBUTTON thbButtons[3];

    thbButtons[0].dwMask = dwMask;
    thbButtons[0].iId = thumb_prev;
    thbButtons[0].iBitmap = 0;
    thbButtons[0].dwFlags = THBF_ENABLED;

    thbButtons[1].dwMask = dwMask;
    thbButtons[1].iId = thumb_play_or_pause;
    thbButtons[1].iBitmap = 2;
    thbButtons[1].dwFlags = THBF_ENABLED;

    thbButtons[2].dwMask = dwMask;
    thbButtons[2].iId = thumb_next;
    thbButtons[2].iBitmap = 3;
    thbButtons[2].dwFlags = THBF_ENABLED;

    hr = p_taskbl->ThumbBarSetImageList( (HWND)winId(), himl );
    if( FAILED(hr) )
        Log(Log_Err, "%s failed with error %08lx", "ThumbBarSetImageList",
                 hr );
    else
    {

        hr = p_taskbl->ThumbBarAddButtons((HWND)winId(), 3, thbButtons);
        if( FAILED(hr) )
            Log(Log_Err, "%s failed with error %08lx",
                     "ThumbBarAddButtons", hr );
    }

    auto control = VIDEO_CONTROL;
    connect(control, &QVideoControl::start, this, &QWinThumbnail::onStart);
    connect(control, &QVideoControl::continuePlay, this, &QWinThumbnail::onStart);
    connect(control, &QVideoControl::end, this, &QWinThumbnail::onEnd);
    connect(control, &QVideoControl::pausePlay, this, &QWinThumbnail::onPause);
}

void QWinThumbnail::onStart()
{
    modifyBtn(true);
}

void QWinThumbnail::onPause()
{
    modifyBtn(false);
}

void QWinThumbnail::onEnd()
{
    modifyBtn(false);
}

void QWinThumbnail::modifyBtn(bool bPlay)
{
    if(!p_taskbl)
        return;

    // Define an array of three buttons. These buttons provide images through an
    // image list and also provide tooltips.
    THUMBBUTTONMASK dwMask = THUMBBUTTONMASK(THB_BITMAP | THB_FLAGS);

    THUMBBUTTON thbButtons[3];
    thbButtons[0].dwMask = dwMask;
    thbButtons[0].iId = thumb_prev;
    thbButtons[0].iBitmap = 0;
    thbButtons[0].dwFlags = THBF_ENABLED;

    thbButtons[1].dwMask = dwMask;
    thbButtons[1].iId = thumb_play_or_pause;
    thbButtons[1].iBitmap = bPlay ? 1 : 2;
    thbButtons[1].dwFlags = THBF_ENABLED;

    thbButtons[2].dwMask = dwMask;
    thbButtons[2].iId = thumb_next;
    thbButtons[2].iBitmap = 3;
    thbButtons[2].dwFlags = THBF_ENABLED;

    HRESULT hr =  p_taskbl->ThumbBarUpdateButtons((HWND)winId(), 3, thbButtons);

    if(S_OK != hr)
        Log(Log_Err,  "ThumbBarUpdateButtons failed with error %08lx", hr );

    // If a video is playing, let the vout handle the thumbnail.
//    if( !videoWidget || !THEMIM->getIM()->hasVideo() )
//    {
//        hr = p_taskbl->SetThumbnailClip(WinId(this), nullptr);
//        if(S_OK != hr)
//            Log(Log_Err,  "SetThumbnailClip failed with error %08lx", hr );
//    }
}
#include <QDebug>
bool QWinThumbnail::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    auto msg = static_cast<MSG*>(message);
    if(msg->message == taskbar_wmsg)
    {
        init();
    }
    switch (msg->message) {
    case WM_COMMAND:
    if (HIWORD(msg->wParam) == THBN_CLICKED)
    {
        auto type = LOWORD(msg->wParam);
        if(type < thumb_max && type >= 0)
            emit thumb(type);
    }
    break;
    }

    return QFrameLessWidget::nativeEvent(eventType, message, result);
}

#endif
