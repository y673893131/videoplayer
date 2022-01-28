#include "qoutputwidget.h"
#include <QLabel>
#include <QBoxLayout>
#include <QTimer>
#include "control/videocontrol.h"

QOutputWidget::QOutputWidget(QWidget *parent)
    :QToolBase(parent, false)
{
    initUi();
    initLayout();
}

void QOutputWidget::initUi()
{
    m_label[label_info] = new QLabel(this);
    m_label[label_info]->setObjectName("output_info");
    m_label[label_info]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_timerDelay = new QTimer(this);
//    m_timerDelay->setSingleShot(true);
    m_timerDelay->setInterval(2000);
}

void QOutputWidget::initLayout()
{
    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    layout->addWidget(m_label[label_info], 0, Qt::AlignLeft | Qt::AlignVCenter);
}

void QOutputWidget::initConnect()
{
    auto control = VIDEO_CONTROL;
    connect(m_timerDelay, &QTimer::timeout, this, &QOutputWidget::onDelay);
    connect(control, &QVideoControl::tips, this, &QOutputWidget::onInfo);
}

void QOutputWidget::onDelay()
{
    m_label[label_info]->clear();
    m_timerDelay->stop();
}

void QOutputWidget::onInfo(const QString & info)
{
    m_timerDelay->stop();
    m_timerDelay->start();

    m_label[label_info]->setText(info);
}
