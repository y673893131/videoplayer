#include "qplaysubtitle.h"
#include "control/videocontrol.h"
#include "framelesswidget/util.h"
#include <QLabel>
#include <QBoxLayout>
QPlaySubtitle::QPlaySubtitle(QWidget *parent)
    :QToolBase(parent, false)
{
    initUi();
    initLayout();
}

void QPlaySubtitle::initUi()
{
    m_timerDisplay = new QTimer(this);
    m_timerDisplay->setInterval(4000);
    m_timerDisplay->setSingleShot(true);
    m_label[label_main] = new QLabel(this);
    m_label[label_sub] = new QLabel(this);

    for(int n = 0; n < label_max; ++n)
    {
        m_label[n]->setWordWrap(true);
        m_label[n]->setAlignment(Qt::AlignCenter);
    }

    setObjectName("sub_title_wd");
    m_label[label_main]->setObjectName("sub_title_ch");
    m_label[label_sub]->setObjectName("sub_title_other");
}

void QPlaySubtitle::initLayout()
{
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setMargin(0);
    for(int n = 0; n < label_max; ++n)
    {
        layout->addWidget(m_label[n]);
    }

    layout->addSpacing(CALC_WIDGET_HEIGHT(nullptr, 15));
}

void QPlaySubtitle::initConnect()
{
    auto control = VIDEO_CONTROL;
    connect(m_timerDisplay, &QTimer::timeout, this, &QPlaySubtitle::onDelayClear);
    connect(control, &QVideoControl::subtitle, this, &QPlaySubtitle::onSubtitle);
}

void QPlaySubtitle::resizeEvent(QResizeEvent *event)
{
    QToolBase::resizeEvent(event);
    for(int n = 0; n < label_max; ++n)
    {
        m_label[n]->setFixedWidth(width());
    }
}

void QPlaySubtitle::onDelayClear()
{
    for(int n = 0; n < label_max; ++n)
    {
        m_label[n]->clear();
    }
}

void QPlaySubtitle::onChannelModify()
{
    m_subtitle.titls.clear();
    update();
}

void QPlaySubtitle::onSubtitle(const QString &str, unsigned int index, int type)
{
    switch (type) {
    case 2://SUBTITLE_TEXT
        m_label[label_main]->setText(str);
        m_label[label_sub]->clear();
        m_timerDisplay->stop();
        m_timerDisplay->start();
        return;
    case 3://SUBTITLE_ASS
        break;
    default:
        return;
    }
    //    qDebug() << str << index;
    enum enum_sub_title
    {
        Sub_Titl_Time_Begin = 1,
        Sub_Titl_Time_End,
        Sub_Title_Type,
        Sub_Title_Content = 9
    };

    auto list = str.split(',');
    if(list.size() <= Sub_Title_Content)
    {
        return;
    }

    auto tmBeg = UTIL->getMs(list[Sub_Titl_Time_Begin]);
    auto tmEnd = UTIL->getMs(list[Sub_Titl_Time_End]);

    if(m_subtitle.tmBeg != tmBeg || m_subtitle.tmEnd != tmEnd)
    {
        m_subtitle.tmBeg = tmBeg;
        m_subtitle.tmEnd = tmEnd;
        m_subtitle.titls.clear();
    }

    QString content;
    for (int n = Sub_Title_Content; n < list.size(); ++n)
    {
        content.append(list[n]);
        content.append(",");
    }

    if(list.size() > Sub_Title_Content)
    {
        content.remove(content.length() - 1, 1);
    }

    if(content.lastIndexOf("\r\n") >= 0)
    {
        content = content.remove(content.length() - 2, 2);
    }

    m_subtitle.titls[list[Sub_Title_Type]] = content;
    m_label[label_main]->setText(m_subtitle.titls.begin()->second);
    if(m_subtitle.titls.size() >= 2)
    {
        m_label[label_sub]->setText(m_subtitle.titls.rbegin()->second);
    }
    else
    {
        m_label[label_sub]->clear();
    }

    m_label[label_main]->setVisible(true);
    if(!m_label[label_sub]->text().isEmpty())
        m_label[label_sub]->setVisible(true);
    m_timerDisplay->stop();
    m_timerDisplay->start();
}
