#ifndef QNATIVEWIDGET_P_H
#define QNATIVEWIDGET_P_H

#include "video_pimpl.h"
#include "../qrenderprivate.h"

class QNativeWidget;
class QNativeWidgetPrivate : public VP_Data<QNativeWidget>, public QRenderPrivate
{
    VP_DECLARE_PUBLIC(QNativeWidget)
protected:
    explicit QNativeWidgetPrivate(QNativeWidget* parent);
    void initConfig();
private:
    bool m_bDeviceInitialized;
};

#endif // QNATIVEWIDGET_P_H
