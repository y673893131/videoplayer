#include "qliveplatform.h"
#include "platform/platform/qliveplatformmanager.h"
#include "platform/platform/qdouyuwidget.h"
#include "control/videocontrol.h"
#include "ui/qtoolwidgets.h"
#include "ui/tool/fileview/qfileview.h"
#include <QPushButton>
#include <QBoxLayout>

QLivePlatform::QLivePlatform(QWidget *parent)
    :QToolBase(parent)
{
    initUi();
    initLayout();
}

void QLivePlatform::initUi()
{
    m_button[button_close] = new QPushButton(this);
    m_platformManager = new QLivePlatformManager(this);
    m_dy = new QDouyuWidget(this);
    setObjectName("live_platform");
    m_button[button_close]->setObjectName("btn_close");
    hide();
}

void QLivePlatform::initLayout()
{
    auto group = m_platformManager->group();
    auto layout = new QVBoxLayout(this);
    auto layoutBtn = new QVBoxLayout;
    layout->addWidget(m_button[button_close], 0, Qt::AlignRight | Qt::AlignTop);
    layout->addStretch();
    layout->addLayout(layoutBtn);
    layout->addStretch();

    layoutBtn->setAlignment(Qt::AlignCenter);
    for(auto it : group->buttons())
        layoutBtn->addWidget(it);
}

void QLivePlatform::initConnect()
{
    auto group = m_platformManager->group();
    auto control = VIDEO_CONTROL;
    auto toolWidget = qobject_cast<QToolWidgets*>(m_parent);
    auto filelist = m_parent->findChild<QFileView*>();
    connect(m_button[button_close], &QPushButton::clicked, this, &QWidget::hide);

#if (QT_VERSION < QT_VERSION_CHECK(5,15,0))
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &QWidget::hide);
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), m_dy, &QDouyuWidget::showIndex);
#else
    connect(group, &QButtonGroup::idClicked, this, &QWidget::hide);
    connect(group, &QButtonGroup::idClicked, m_dy, &QDouyuWidget::showIndex);
#endif

    connect(m_dy, &QDouyuWidget::play, filelist, &QFileView::onHandleStop);
    connect(m_dy, &QDouyuWidget::play, control, &QVideoControl::onStart);

    connect(toolWidget, &QToolWidgets::moveShowPlatform, this, &QLivePlatform::onMoveShow);
}

void QLivePlatform::onAutoVisable(bool bHide)
{
    if(bHide)
    {
        setVisible(false);
    }
}

void QLivePlatform::onMoveShow()
{
    if(m_dy->isHidden())
        setVisible(true);
}
