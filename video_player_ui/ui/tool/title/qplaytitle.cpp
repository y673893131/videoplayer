#include "qplaytitle.h"
#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>
#include <QApplication>
#include "control/videocontrol.h"
#include "ui/qtoolwidgets.h"

QPlayTitle::QPlayTitle(QWidget *parent)
    :QToolBase(parent)
{
    initUi();
    initLayout();
}

void QPlayTitle::initUi()
{
    m_title = new QLabel(qApp->applicationName(), this);
    m_button[button_minimize] = new QPushButton(this);
    m_button[button_maximize] = new QPushButton(this);
    m_button[button_close] = new QPushButton(this);

    setObjectName("wd_title");
    m_title->setObjectName("label_title");
    m_button[button_minimize]->setObjectName("btn_min");
    m_button[button_maximize]->setObjectName("btn_max");
    m_button[button_close]->setObjectName("btn_close");

    m_button[button_minimize]->setToolTip(tr("minimize"));
    m_button[button_maximize]->setToolTip(tr("maximize"));
    m_button[button_close]->setToolTip(tr("close"));

    CALC_WIDGET_WIDTH(m_title, 800);
}

void QPlayTitle::initLayout()
{
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addSpacing(10);
    layout->addWidget(m_title);
    layout->addStretch();
    for(int n = 0; n < button_max; ++n)
    {
        layout->addWidget(m_button[n]);
    }
}

void QPlayTitle::initConnect()
{
    auto control = VIDEO_CONTROL;
    auto toolWidget = qobject_cast<QToolWidgets*>(m_parent);
    connect(m_button[button_minimize], &QPushButton::clicked, toolWidget, &QToolWidgets::showMin);
    connect(m_button[button_maximize], &QPushButton::clicked, toolWidget, &QToolWidgets::onMax);
    connect(m_button[button_close], &QPushButton::clicked, toolWidget, &QToolWidgets::exit);

    connect(control, &QVideoControl::play, this, &QPlayTitle::onPlay);
}

void QPlayTitle::onPlay(const QString &sUrl)
{
    auto sName = sUrl.mid(sUrl.lastIndexOf('/') + 1);
    m_title->setToolTip(sName);
    auto font = m_title->fontMetrics();
    auto sText = font.elidedText(sName, Qt::ElideRight, m_title->width());
    m_title->setText(sText);
}
