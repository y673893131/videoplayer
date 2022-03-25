#include "qassengineer.h"
#include <QPainter>

#include "libass/libass.h"
#include "control/videocontrol.h"

class QASSEngineerPrivate : public VP_Data<QASSEngineer>
{
    VP_DECLARE_PUBLIC(QASSEngineer)
    inline QASSEngineerPrivate(QASSEngineer* parent)
        : VP_Data(parent)
        , m_ass(new Libass)
    {
    }

    ~QASSEngineerPrivate()
    {
        if(m_ass)
        {
            delete m_ass;
            m_ass = nullptr;
        }
    }
private:
    Libass* m_ass;
};

QASSEngineer::QASSEngineer(QObject* parent)
    : QSubtitleEngineer(parent)
    , VP_INIT(new QASSEngineerPrivate(this))
{
    VP_D(QASSEngineer);
    auto control = VIDEO_CONTROL;
    connect(control, &QVideoControl::videoSizeChanged, d->m_ass, &Libass::setFrameSize);
}

QASSEngineer::~QASSEngineer()
{
}

void QASSEngineer::setFrameSize(QSize size)
{
    VP_D(QASSEngineer);
    if(d->m_ass)
    {
        d->m_ass->setFrameSize(size.width(), size.height());
    }
}

void QASSEngineer::onHeader(const subtitle_header &infos)
{
    VP_D(QASSEngineer);
    setHeader(infos);
    d->m_ass->initStreamHeader(QString::fromUtf8(infos.sHeader.c_str()));
}

void QASSEngineer::onRender(const QString &str, unsigned int, int /*type*/, int64_t start, int64_t end)
{
    VP_D(QASSEngineer);
    updateDelay(start, end);
    d->m_ass->render(str, static_cast<int>(start), static_cast<int>(end - start));
    target()->update();
}

void QASSEngineer::flush()
{
    VP_D(QASSEngineer);
    d->m_ass->flush();
}

void QASSEngineer::clean()
{
    VP_D(QASSEngineer);
    if(d->m_ass->isFlush())
    {
        target()->update();
    }
}

void QASSEngineer::update()
{
    VP_D(QASSEngineer);
    if(!target())
        return;
    auto& image = d->m_ass->image();
    QPainter p(target());
    p.drawImage(target()->rect(), image);
}

