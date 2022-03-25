#include "qsubtitleengineer.h"

struct sub_title_time
{
    sub_title_time()
        : tmBeg(-1)
        , tmEnd(-1)
    {

    }

    void reset()
    {
        tmBeg = -1;
        tmEnd = -1;
    }

    bool set(int64_t start, int64_t end)
    {
        if(tmBeg != start || tmEnd != end)
        {
            tmBeg = start;
            tmEnd = end;
            return true;
        }

        return false;
    }
    int64_t tmBeg;
    int64_t tmEnd;
};

class QSubtitleEngineerPrivate : public VP_Data<QSubtitleEngineer>
{
    VP_DECLARE_PUBLIC(QSubtitleEngineer)
    inline QSubtitleEngineerPrivate(QSubtitleEngineer* parent)
        : VP_Data(parent)
        , m_target(nullptr)
        , m_pos(-1)
    {

    }

private:
    QWidget* m_target;
    subtitle_header m_header;
    sub_title_time m_delay;
    int m_pos;
};

QSubtitleEngineer::QSubtitleEngineer(QObject *parent)
    : QObject(parent)
    , VP_INIT(new QSubtitleEngineerPrivate(this))
{

}

QSubtitleEngineer::~QSubtitleEngineer()
{
}

void QSubtitleEngineer::setTarget(QWidget *target)
{
    VP_D(QSubtitleEngineer);
    d->m_target = target;
}

void QSubtitleEngineer::setFrameSize(QSize /*size*/)
{
}

void QSubtitleEngineer::setPos(int pos)
{
    VP_D(QSubtitleEngineer);
    d->m_pos = pos;
    if(isFlush(pos))
    {
        flush();
        clean();
    }
}

subtitle_header QSubtitleEngineer::header()
{
    VP_D(QSubtitleEngineer);
    return d->m_header;
}

void QSubtitleEngineer::setHeader(const subtitle_header &sHeader)
{
    VP_D(QSubtitleEngineer);
    d->m_header = sHeader;
}

bool QSubtitleEngineer::isFlush(int pos)
{
    VP_D(QSubtitleEngineer);
    if(d->m_delay.tmBeg < 0)
        return false;

    if((pos < d->m_delay.tmBeg && pos + 200 < d->m_delay.tmBeg) || pos > d->m_delay.tmEnd)
    {
        d->m_delay.reset();
        return true;
    }

    return false;
}

bool QSubtitleEngineer::updateDelay(int64_t start, int64_t end)
{
    VP_D(QSubtitleEngineer);
    return d->m_delay.set(start, end);
}

QWidget *QSubtitleEngineer::target()
{
    VP_D(QSubtitleEngineer);
    return d->m_target;
}
