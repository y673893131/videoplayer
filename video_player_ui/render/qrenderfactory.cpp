#include "qrenderfactory.h"
#include "render/qdirect3d11widget.h"
#include "render/qglvideowidget.h"
#include "config/config.h"
#include "control/videocontrol.h"
#include "Log/Log.h"
#include "filter/qinputfilter.h"
#include "qlabelvideowidget.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>
#include <thread>

QRenderFactory::QRenderFactory(QWidget *parent)
    : QObject(parent)
    , m_bUpdate(false)
{
    auto render = GET_CONFIG_DATA(Config::Data_Render);
    qDebug() << "[render]" << render.toString();
#ifdef Q_OS_WIN
    if(render == "opengl")
    {
        m_renderWidget = new QGLVideoWidget(parent);
    }
    else
    {
        m_renderWidget = new QDirect3D11Widget(parent);
        m_bUpdate = true;
    }
#else
    m_renderWidget = new QGLVideoWidget(parent);
#endif
}

bool QRenderFactory::isUpdate()
{
    return m_bUpdate;
}

QWidget *QRenderFactory::renderWidget()
{
    return m_renderWidget;
}
#include <QMetaObject>
void QRenderFactory::onCap()
{
    if(m_renderWidget)
    {
        std::thread([=]{
            auto sFile = QString("vPlay_") + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss") + ".jpg";
            auto path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + sFile;
            auto img = VIDEO_CONTROL->frame()->toImage();
            auto bOk = img.save(path, "jpg");
#if WIN32
            auto error = GetLastError();
#else
            int error = 0;
#endif
            LogB(Log_Info, "video capture: %s[%d, lasterror=%d]", path.toLocal8Bit().data(), bOk, error);
            if(!bOk)
            {
                path += QString(" error: %1").arg(error);
            }
            emit QInputFilter::instance()->error(path);
        }).detach();
    }
}
