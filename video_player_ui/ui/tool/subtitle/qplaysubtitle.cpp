#include "qplaysubtitle.h"
#include "control/videocontrol.h"
#include "framelesswidget/util.h"
#include <QLabel>
#include <QBoxLayout>
#include <QDebug>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include "ui/tool/base/qsubtitlelabel.h"
#include "ui/tool/play_control/qplaycontrol.h"
QPlaySubtitle::QPlaySubtitle(QWidget *parent)
    :QToolBase(parent, false)
{
    initUi();
    initLayout();
}

void QPlaySubtitle::initUi()
{
//    m_timerDisplay = new QTimer(this);
//    m_timerDisplay->setInterval(4000);
//    m_timerDisplay->setSingleShot(true);

//    m_label[label_main] = new QSubTitleLabel(this);
//    m_label[label_sub] = new QSubTitleLabel(this);

//    for(int n = 0; n < label_max; ++n)
//    {
//        m_label[n]->setWordWrap(true);
//        m_label[n]->setAlignment(Qt::AlignCenter);
//    }

//    setObjectName("sub_title_wd");
//    m_label[label_main]->setObjectName("sub_title_ch");
//    m_label[label_sub]->setObjectName("sub_title_other");

//    QFont font;
//    font.setBold(true);
//    font.setFamily("Tahoma");

//    font.setPointSize(18);
//    m_label[label_main]->setPen("#0080FF");
//    m_label[label_main]->setMode(false);
//    m_label[label_main]->setOutlineThickness(2);
//    m_label[label_main]->setFont(font);

//    font.setPointSize(14);
//    m_label[label_sub]->setPen("#000000");
//    m_label[label_sub]->setMode(false);
//    m_label[label_sub]->setOutlineThickness(1);
//    m_label[label_sub]->setFont(font);
}

void QPlaySubtitle::initLayout()
{
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setMargin(0);
}

void QPlaySubtitle::initConnect()
{
    auto control = VIDEO_CONTROL;
//    auto playControl = m_parent->findChild<QPlayControl*>();
//    connect(m_timerDisplay, &QTimer::timeout, this, &QPlaySubtitle::onDelayClear);
    connect(control, &QVideoControl::subtitle, this, &QPlaySubtitle::onSubtitle);
    connect(control, &QVideoControl::end, this, &QPlaySubtitle::onDelayClear);
    connect(control, &QVideoControl::setPos, this, &QPlaySubtitle::onPos);
    connect(control, &QVideoControl::subTitleHeader, this, &QPlaySubtitle::onSubTitleHeader);
}

void QPlaySubtitle::resizeEvent(QResizeEvent *event)
{
    QToolBase::resizeEvent(event);
    for(auto it : m_label)
    {
        it->setFixedWidth(width());
    }
}

void QPlaySubtitle::onDelayClear()
{
    for(auto it : m_label)
    {
        if(!it->text().isEmpty())
            it->clear();
    }
}

void QPlaySubtitle::onPos(int pos)
{
    if((pos < m_delay.tmBeg && pos + 200 < m_delay.tmBeg)|| pos > m_delay.tmEnd)
    {
//        qDebug() << pos;
        onDelayClear();
    }
}

void QPlaySubtitle::onChannelModify()
{
    for(auto it : m_label)
    {
        it->clear();
    }
    update();
}

void QPlaySubtitle::onSubTitleHeader(const subtitle_header &infos)
{
    auto layout = qobject_cast<QVBoxLayout*>(this->layout());
    for(auto it : m_label)
    {
        layout->removeWidget(it);
        it->deleteLater();
    }

    m_label.clear();
    m_nameToIndex.clear();

    int marginLeft = 0;
    int marginRight = 0;
    int marginBottom = 0;
    int index = 0;
    for(auto it : infos.infos)
    {
        auto label = new /*QSubTitleLabel*/QLabel(this);
        label->setFixedWidth(width());
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);

        layout->addWidget(label);

        m_label.push_back(label);
        m_nameToIndex.insert(std::make_pair(QString::fromUtf8(it.sName), index));
        ++index;

        QFont font;
        font.setBold(it.bold != 0);
        font.setItalic(it.italic != 0);
        font.setFamily(QString::fromUtf8(it.sName));

        font.setPointSize(it.pt);

        label->setFont(font);
//        label->setMode(false);
//        label->setOutlineThickness(it.outlinePix);

        union bgr_color
        {
            int bgr;
            unsigned char bit[4];
        };

        bgr_color color;
        color.bgr = it.color[0];

        QPalette pa;
        pa.setColor(QPalette::WindowText, QColor(color.bit[0], color.bit[1], color.bit[2]));
        pa.setColor(QPalette::Shadow, QColor(Qt::gray));
        label->setPalette(pa);

//        label->setBrush(QColor(color.bit[0], color.bit[1], color.bit[2]));

//        color.bgr = it.color[2];
//        label->setPen(QColor(color.bit[0], color.bit[1], color.bit[2]));

        marginLeft = it.marginLeft;
        marginRight = it.marginRight;
        marginBottom = it.marginBottom;
    }

    layout->setContentsMargins(marginLeft, 0, marginRight, marginBottom);

    UTIL->flush(this);
}

void QPlaySubtitle::onSubtitle(const QString &str, unsigned int /*index*/, int type, int64_t start, int64_t end)
{
    switch (type) {
    case 2://SUBTITLE_TEXT
        if(!m_label.empty()) m_label[0]->setText(str);
        for(int i = 1; i < m_label.size(); ++i)
            m_label[i]->clear();
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

//    auto tmBeg = UTIL->getMs(list[Sub_Titl_Time_Begin]);
//    auto tmEnd = UTIL->getMs(list[Sub_Titl_Time_End]);

    if(m_delay.tmBeg != /*tmBeg*/start || m_delay.tmEnd != /*tmEnd*/end)
    {
        m_delay.tmBeg = start;
        m_delay.tmEnd = end;
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

    auto index = m_nameToIndex.find(list[Sub_Title_Type]);
    if(index == m_nameToIndex.end())
        return;

    auto label = m_label[static_cast<unsigned int>(index->second)];

//    qDebug() << content << start << end;
    label->setText(content);
    adjustSize();
    move(0, m_parent->height() - height());
}
