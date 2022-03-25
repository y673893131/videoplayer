#include "qplaysubtitle.h"
#include "control/videocontrol.h"
#include "framelesswidget/util.h"
#include <QLabel>
#include <QBoxLayout>
#include <QDebug>
#include <QScreen>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include "ui/qtoolwidgets.h"
#include "ui/tool/base/qsubtitlelabel.h"
#include "ui/tool/play_control/qplaycontrol.h"
#include "ui/tool/output/qoutputwidget.h"

#include "./engine/qsubtitlengine.h"

class QPlaySubtitlePrivate : public VP_Data<QPlaySubtitle>
{
    VP_DECLARE_PUBLIC(QPlaySubtitle)
    inline QPlaySubtitlePrivate(QPlaySubtitle* parent)
        : VP_Data(parent)
        , m_engine(new QSubtitlEngine(parent))
    {
    }

private:
    QSubtitlEngine* m_engine;
};

QPlaySubtitle::QPlaySubtitle(QWidget *parent)
    :QToolBase(parent, false)
    ,VP_INIT(new QPlaySubtitlePrivate(this))
{
    initUi();
    initLayout();
}

QPlaySubtitle::~QPlaySubtitle()
{
}

void QPlaySubtitle::initUi()
{
#ifdef AAS_SUBTITLE_RENDER
#endif
}

void QPlaySubtitle::initLayout()
{
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    layout->setMargin(0);
}

void QPlaySubtitle::initConnect()
{
    VP_D(QPlaySubtitle);

    auto control = VIDEO_CONTROL;
    auto output = m_parent->findChild<QOutputWidget*>();
    connect(control, &QVideoControl::end, this, &QPlaySubtitle::onDelayClear);
    connect(control, &QVideoControl::setPos, this, &QPlaySubtitle::onPos);
    connect(d->m_engine, &QSubtitlEngine::reportError, output, &QOutputWidget::onError);
}

void QPlaySubtitle::paintEvent(QPaintEvent *event)
{
    VP_D(QPlaySubtitle);
    QToolBase::paintEvent(event);
    d->m_engine->update();
}

void QPlaySubtitle::onGotoPos()
{
    VP_D(QPlaySubtitle);
    d->m_engine->flush();
    onDelayClear();
}

void QPlaySubtitle::onDelayClear()
{
    VP_D(QPlaySubtitle);
    d->m_engine->clean();
}

void QPlaySubtitle::onPos(int pos)
{
    VP_D(QPlaySubtitle);
    d->m_engine->setPos(pos);
}

void QPlaySubtitle::onChannelModify()
{
    onDelayClear();
}

void QPlaySubtitle::onSetEngine(int index)
{
    VP_D(QPlaySubtitle);
//    d->m_engine->setEngine(index);
}


