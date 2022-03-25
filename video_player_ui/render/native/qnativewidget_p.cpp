#include "qnativewidget_p.h"
#include "config/config.h"

QNativeWidgetPrivate::QNativeWidgetPrivate(QNativeWidget* parent)
    : VP_Data(parent)
    , QRenderPrivate()
    , m_bDeviceInitialized(false)
{
}

void QNativeWidgetPrivate::initConfig()
{
    setScaleType(GET_CONFIG_DATA(Config::Data_PlaySize).toInt());
}
