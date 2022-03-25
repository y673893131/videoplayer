#include "qsubtitlengine.h"

#include "control/videocontrol.h"
#include "qplayerenginner.h"
#include "qsubtitleengineer.h"
#include "qassengineer.h"

class QSubtitlEnginePrivate : public VP_Data<QSubtitlEngine>
{
    VP_DECLARE_PUBLIC(QSubtitlEngine)

    inline QSubtitlEnginePrivate(QSubtitlEngine* parent)
        : VP_Data(parent)
        , m_engineIndex(-1)
        , m_engineer(nullptr)
    {

    }

    ~QSubtitlEnginePrivate()
    {
        if(m_engineer)
        {
            delete m_engineer;
            m_engineer = nullptr;
        }
    }
    enum e_engine
    {
        engine_player,
        engine_ass,

        engine_max
    };

    void init(int);
    void reinit();

    void render(const QString&, unsigned int index, int type, int64_t start, int64_t end);
    void update(QWidget*);
    void clean(QWidget*);
private:
    int m_engineIndex;
    QSubtitleEngineer* m_engineer;
};

void QSubtitlEnginePrivate::init(int type)
{
    if(type != m_engineIndex)
    {
        m_engineIndex = type;
        reinit();
    }
}

void QSubtitlEnginePrivate::reinit()
{
    VP_Q(QSubtitlEngine);
    if(m_engineIndex < 0 || m_engineIndex >= engine_max) return;
    subtitle_header header;
    QWidget* target = nullptr;
    if(m_engineer)
    {
        header = m_engineer->header();
        target = m_engineer->target();
        m_engineer->deleteLater();
        m_engineer = nullptr;
    }

    switch (m_engineIndex) {
    case engine_player:
        m_engineer = new QPlayerEnginner(q);
        break;
    case engine_ass:
        m_engineer = new QASSEngineer(q);
        break;
    }

    if(target)
    {
        m_engineer->setTarget(target);
    }

    auto control = VIDEO_CONTROL;
    if(m_engineer && header.bInit)
    {
        m_engineer->onHeader(header);
        m_engineer->setFrameSize(control->frameSize());
    }

    q->connect(control, &QVideoControl::subTitleHeader, m_engineer, &QSubtitleEngineer::onHeader);
    q->connect(control, &QVideoControl::subtitle, m_engineer, &QSubtitleEngineer::onRender);
}

void QSubtitlEnginePrivate::render(const QString &str, unsigned int index, int type, int64_t start, int64_t end)
{
    if(m_engineer)
    {
        m_engineer->onRender(str, index, type, start, end);
    }
}

QSubtitlEngine::QSubtitlEngine(QWidget* parent)
    : QObject(parent)
    , VP_INIT(new QSubtitlEnginePrivate(this))
{
    VP_D(QSubtitlEngine);
    d->init(QSubtitlEnginePrivate::engine_player);
    setTarget(parent);
}

QSubtitlEngine::~QSubtitlEngine()
{
}

void QSubtitlEngine::setTarget(QWidget *target)
{
    VP_D(QSubtitlEngine);
    d->m_engineer->setTarget(target);
}

void QSubtitlEngine::setEngine(int index)
{
    VP_D(QSubtitlEngine);
    d->init(index);
}

void QSubtitlEngine::clean()
{
    VP_D(QSubtitlEngine);
    d->m_engineer->clean();
}

void QSubtitlEngine::update()
{
    VP_D(QSubtitlEngine);
    d->m_engineer->update();
}

void QSubtitlEngine::flush()
{
    VP_D(QSubtitlEngine);
    d->m_engineer->flush();
}

void QSubtitlEngine::setPos(int pos)
{
    VP_D(QSubtitlEngine);
    d->m_engineer->setPos(pos);
}


