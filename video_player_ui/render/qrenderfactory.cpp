#include "qrenderfactory.h"
#include "render/qdirect3d11widget.h"
#include "render/qglvideowidget.h"
#include "config/config.h"
#include <QDebug>

QRenderFactory::QRenderFactory(QWidget *parent)
    : QObject(parent)
    , m_bUpdate(false)
{
    auto render = GET_CONFIG_DATA(Config::Data_Render);
    qDebug() << "[render]" << render.toString();
    if(render == "opengl")
    {
        m_renderWidget = new QGLVideoWidget(parent);
    }
    else
    {
        m_renderWidget = new QDirect3D11Widget(parent);
        m_bUpdate = true;
    }
}

bool QRenderFactory::isUpdate()
{
    return m_bUpdate;
}

QWidget *QRenderFactory::renderWidget()
{
    return m_renderWidget;
}
