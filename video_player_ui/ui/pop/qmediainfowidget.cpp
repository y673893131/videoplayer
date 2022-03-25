#include "qmediainfowidget.h"
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QDebug>

#include "qpopwidgetprivate.h"
#include "control/videocontrol.h"

#include "qbitratedisplay.h"

class QMediaInfoWidgetPrivate : public QPopWidgetPrivate
{
    VP_DECLARE_PUBLIC(QMediaInfoWidget)
    inline QMediaInfoWidgetPrivate(QMediaInfoWidget* parent)
        : QPopWidgetPrivate(parent)
    {
        setKey("media_info");
    }

    void init();
    void stop();
    void updatePath(const QString&);
    void updateBitRate(int64_t);
    void updateMediaSize(int, int);
    void updateFps(int);

    enum elabel
    {
        elabel_path,
        elabel_video_size,
        elabel_bit_rate,
        elabel_fps
    };

    QVector<QPair<QLabel*, QLabel*>> labels;
    QBitRateDisplay* bitrate;
};

void QMediaInfoWidgetPrivate::init()
{
    VP_Q(QMediaInfoWidget);
    setResizeable(true);
    q->setObjectName("media_info_widget");

    auto size = CALC_WIDGET_SIZE(nullptr, 500, 300);
    q->resize(size);

#define TO_PAIR(x) qMakePair(new QLabel(x + " : ", q), new QLabel(q))
    labels << TO_PAIR(QMediaInfoWidget::tr("file path"));
    labels << TO_PAIR(QMediaInfoWidget::tr("video size"));
    labels << TO_PAIR(QMediaInfoWidget::tr("bit rate"));
    labels << TO_PAIR(QString("fps"));

    auto title = createTitle(QMediaInfoWidget::tr("media info"), q);
    bitrate = new QBitRateDisplay(q);
    auto layout = new QVBoxLayout(q);
    auto layoutContent = new QGridLayout;
    layout->setMargin(0);
    layout->addWidget(title);
    layout->addLayout(layoutContent);

    for(int i = 0; i < labels.size(); ++i)
    {
        labels[i].first->setObjectName("media_info_title");
        labels[i].second->setObjectName("media_info_value");
        labels[i].second->setWordWrap(true);
        labels[i].second->setTextInteractionFlags(Qt::TextSelectableByMouse);

        layoutContent->addWidget(labels[i].first, i, 0, Qt::AlignRight | Qt::AlignTop);
        layoutContent->addWidget(labels[i].second, i, 1);
    }

    layoutContent->setAlignment(Qt::AlignTop);
    layoutContent->setMargin(5);
    layoutContent->setColumnStretch(0, 1);
    layoutContent->setColumnStretch(1, 10);

    auto layoutBitRate = new QVBoxLayout;
    layoutBitRate->setMargin(5);
    layoutBitRate->addWidget(bitrate);
    layout->addLayout(layoutBitRate, 10);
}

void QMediaInfoWidgetPrivate::stop()
{
    for(auto&& it : qAsConst(labels))
        it.second->clear();
}

void QMediaInfoWidgetPrivate::updatePath(const QString & path)
{
    labels[elabel_path].second->setText(path);
}

void QMediaInfoWidgetPrivate::updateBitRate(int64_t size)
{
    QString sUint("Kbps");
    auto value = size * 8 / 1024.0;
    if(value > 1024.0)
    {
        value /= 1024.0;
        sUint = "Mbps";
        if(value > 1024.0)
        {
            value /= 1024.0;
            sUint = "Gbps";
        }
    }

    labels[elabel_bit_rate].second->setText(QString::asprintf("%.3f %s", value, sUint.toLocal8Bit().data()));
    bitrate->append(size);
}

void QMediaInfoWidgetPrivate::updateMediaSize(int w, int h)
{
    labels[elabel_video_size].second->setText(QString("%1*%2").arg(w).arg(h));
}

void QMediaInfoWidgetPrivate::updateFps(int count)
{
    labels[elabel_fps].second->setText(QString("%1").arg(count));
}

QMediaInfoWidget::QMediaInfoWidget(QWidget *parent)
    : QPopWidget(new QMediaInfoWidgetPrivate(this), parent)
{
    VP_D(QMediaInfoWidget);
    d->init();
}

void QMediaInfoWidget::initConnect(QWidget* /*parent*/)
{
    VP_D(QMediaInfoWidget);
    auto control = VIDEO_CONTROL;
    connect(control, &QVideoControl::end, this, [=]{ d->stop();});
    connect(control, &QVideoControl::play, this, [=](const QString& path){ d->updatePath(path);});
    connect(control, &QVideoControl::frameRate, this, [=](int fps){ d->updateFps(fps);});
    connect(control, &QVideoControl::bitRateChanged, this, [=](int64_t rate){ d->updateBitRate(rate);});
    connect(control, &QVideoControl::videoSizeChanged, this, [=](int w, int h){ d->updateMediaSize(w, h);});
}

void QMediaInfoWidget::onShowCenter()
{
    QPopWidget::onShowCenter();
    VP_D(QMediaInfoWidget);
    d->bitrate->clear();
}

void QMediaInfoWidget::resizeEvent(QResizeEvent *event)
{
    QPopWidget::resizeEvent(event);
}
