#include "qplayerenginner.h"

#include <QBoxLayout>
#include "ui/tool/base/qsubtitlelabel.h"
#include "framelesswidget/util.h"
#include <stdint.h>
//#define AAS_SUBTITLE_RENDER

class QPlayerEnginnerPrivate : public VP_Data<QPlayerEnginner>
{
    VP_DECLARE_PUBLIC(QPlayerEnginner)
    inline QPlayerEnginnerPrivate(QPlayerEnginner* parent)
        : VP_Data(parent)
    {
    }

    ~QPlayerEnginnerPrivate()
    {
        for(auto it : m_label)
        {
            it->deleteLater();
        }

        m_label.clear();
    }
private:

#ifdef AAS_SUBTITLE_RENDER
    std::vector<QSubTitleLabel*> m_label;
#else
    std::vector<QLabel*> m_label;
#endif

    std::map<QString, int> m_nameToIndex;
};

QPlayerEnginner::QPlayerEnginner(QObject* parent)
    : QSubtitleEngineer(parent)
    , VP_INIT(new QPlayerEnginnerPrivate(this))
{

}

QPlayerEnginner::~QPlayerEnginner()
{
}

void QPlayerEnginner::onHeader(const subtitle_header &infos)
{
    VP_D(QPlayerEnginner);
    setHeader(infos);
    auto target = this->target();
    auto layout = qobject_cast<QVBoxLayout*>(target->layout());
    for(auto it : d->m_label)
    {
        layout->removeWidget(it);
        it->deleteLater();
    }

    d->m_label.clear();
    d->m_nameToIndex.clear();

    int marginLeft = 0;
    int marginRight = 0;
    int marginBottom = 0;
    int index = 0;
    for(auto it : infos.infos)
    {
#ifdef AAS_SUBTITLE_RENDER
        auto label = new QSubTitleLabel(this);
#else
        auto label = new QLabel(target);
#endif
        label->setFixedWidth(target->width());
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);

        layout->addWidget(label);

        d->m_label.push_back(label);
        d->m_nameToIndex.insert(std::make_pair(QString::fromUtf8(it.sName), index));
        ++index;

        QFont font;
        font.setBold(it.bold != 0);
        font.setItalic(it.italic != 0);
        font.setFamily(QString::fromUtf8(it.sName));

        font.setPointSize(it.pt);

        label->setFont(font);
#ifdef AAS_SUBTITLE_RENDER
        label->setMode(false);
        label->setOutlineThickness(it.outlinePix);
#endif
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

#ifdef AAS_SUBTITLE_RENDER
        label->setBrush(QColor(color.bit[0], color.bit[1], color.bit[2]));

        color.bgr = it.color[2];
        label->setPen(QColor(color.bit[0], color.bit[1], color.bit[2]));
#endif
        marginLeft = it.marginLeft;
        marginRight = it.marginRight;
        marginBottom = it.marginBottom;
    }

    layout->setContentsMargins(marginLeft, 0, marginRight, marginBottom);

    UTIL->flush(target);
}

void QPlayerEnginner::onRender(const QString &str, unsigned int, int type, int64_t start, int64_t end)
{
    VP_D(QPlayerEnginner);
    updateDelay(start, end);
    switch (type) {
    case 2://SUBTITLE_TEXT
        if(!d->m_label.empty()) d->m_label[0]->setText(str);
        for(unsigned int i = 1; i < d->m_label.size(); ++i)
            d->m_label[i]->clear();
        return;
    case 3://SUBTITLE_ASS
//        m_engine = engine_ass;
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

    auto index = d->m_nameToIndex.find(list[Sub_Title_Type]);
    if(index == d->m_nameToIndex.end())
        return;

    auto label = d->m_label[static_cast<unsigned int>(index->second)];
    label->setText(content);

    auto target = this->target();
    target->adjustSize();
    target->move(0, target->parentWidget()->height() - target->height());
}

void QPlayerEnginner::flush()
{
    clean();
}

void QPlayerEnginner::clean()
{
    VP_D(QPlayerEnginner);
    for(auto it : d->m_label)
    {
        if(!it->text().isEmpty())
        {
            it->clear();
        }
    }
}

void QPlayerEnginner::update()
{
}
