#include "qplayprogress.h"
#include "qprogressslider.h"
#include "ui/qtoolwidgets.h"
#include "ui/tool/subtitle/qplaysubtitle.h"
#include "ui/tool/output/qoutputwidget.h"
#include "control/videocontrol.h"
#include "filter/qinputfilter.h"
#include <QBoxLayout>

QPlayProgress::QPlayProgress(QWidget* parent)
    :QToolBase(parent)
{
    initUi();
    initLayout();
}

void QPlayProgress::initUi()
{
    m_progress = new QProgressSlider(Qt::Orientation::Horizontal, this, this);
    m_progress->setObjectName("slider_pro");
    m_progress->setValue(0);
    m_progress->setDisabled(true);
}

void QPlayProgress::initLayout()
{
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_progress);
}

void QPlayProgress::initConnect()
{
    auto control = VIDEO_CONTROL;
    auto subtitle = m_parent->findChild<QPlaySubtitle*>();
    auto output = m_parent->findChild<QOutputWidget*>();

    connect(control, &QVideoControl::total, this, &QPlayProgress::onTotalTime);
    connect(control, &QVideoControl::setPos, m_progress, &QProgressSlider::setPos);
    connect(control, &QVideoControl::end,  this, &QPlayProgress::onEnd);
    connect(control, &QVideoControl::preview, m_progress, &QProgressSlider::onPreview);
    connect(control, &QVideoControl::jumpFailed, m_progress, &QProgressSlider::onJumpFailed);

    connect(m_progress, &QProgressSlider::gotoPos, control, &QVideoControl::onSeekPos);
    connect(m_progress, &QProgressSlider::jumpPos, control, &QVideoControl::onJumpPos);
    connect(m_progress, &QProgressSlider::gotoPos, subtitle, &QPlaySubtitle::onDelayClear);
    connect(m_progress, &QProgressSlider::getPreview, control, &QVideoControl::onSeekPosImg);
    connect(m_progress, &QProgressSlider::jumpStr, output, &QOutputWidget::onInfo);

    connect(QInputFilter::instance(), &QInputFilter::progressJump, m_progress, &QProgressSlider::onJump);

}

void QPlayProgress::onTotalTime(int nTotal)
{
    setVisible(nTotal < 100000000);
    m_progress->setRange(0, nTotal);
    m_progress->setEnabled(true);
}

void QPlayProgress::onSetProgress(int nProgress)
{
    m_progress->setValue(nProgress);
}

void QPlayProgress::onEnd()
{
    m_progress->setValue(0);
    m_progress->setDisabled(true);
}

void QPlayProgress::onAutoVisable(bool bHide)
{
    auto max = m_progress->maximum();
    if(max < 100000000)
    {
        QToolBase::onAutoVisable(bHide);
    }
}
