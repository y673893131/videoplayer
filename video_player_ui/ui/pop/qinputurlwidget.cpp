#include "qinputurlwidget.h"
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QDebug>

#include "qpopwidgetprivate.h"

class QInputUrlWidgetPrivate : public QPopWidgetPrivate
{
    VP_DECLARE_PUBLIC(QInputUrlWidget)
    inline QInputUrlWidgetPrivate(QInputUrlWidget* parent)
        : QPopWidgetPrivate(parent)
        , url(nullptr)
        , sure(nullptr)
        , cancel(nullptr)
    {
        setKey("url");
        qDebug() << __FUNCTION__ << __LINE__ << this;
    }

    void init();
    void onSure();


    QLineEdit* url;
    QPushButton* sure;
    QPushButton* cancel;
};

void QInputUrlWidgetPrivate::init()
{
    VP_Q(QInputUrlWidget);
    q->setObjectName("input_url_widget");
    auto widget = q;
    auto title_widget = createTitle(QInputUrlWidget::tr("input url"), widget);
    url = new QLineEdit(widget);
    url->setObjectName("input_url_edit");
    url->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    url->setPlaceholderText("like rtsp/rtmp...");
    auto btn_wdiget = new QWidget(widget);
    btn_wdiget->setObjectName("inout_url_btn_widget");
    sure = new QPushButton(QInputUrlWidget::tr("sure"), btn_wdiget);
    sure->setObjectName("input_url_btn");
    cancel = new QPushButton(QInputUrlWidget::tr("cancel"), btn_wdiget);
    cancel->setObjectName("input_url_btn");
    auto layout = new QVBoxLayout(widget);
    auto layoutBtn = new QHBoxLayout(btn_wdiget);
    layout->setMargin(0);
    layout->addWidget(title_widget);
    layout->addStretch();
    layout->addWidget(url);
    layout->addStretch();
    layout->addWidget(btn_wdiget);

    layoutBtn->addStretch();
    layoutBtn->addWidget(sure);
    layoutBtn->addSpacing(20);
    layoutBtn->addWidget(cancel);
    layoutBtn->addStretch();

    CALC_WIDGET_SIZE(q, 280, 150);
    CALC_WIDGET_SIZE(sure, 100, 30);
    CALC_WIDGET_SIZE(cancel, 100, 30);
    qDebug() << __FUNCTION__ << __LINE__;
}

void QInputUrlWidgetPrivate::onSure()
{
    VP_Q(QInputUrlWidget);
    if(url->text().isEmpty())
        return ;
    Q_EMIT q->inputUrl(QStringList(url->text()));
}

QInputUrlWidget::QInputUrlWidget(QWidget *parent)
    : QPopWidget(new QInputUrlWidgetPrivate(this), parent)
{
    VP_D(QInputUrlWidget);
    d->init();
}

void QInputUrlWidget::initConnect(QWidget* parent)
{
    VP_D(QInputUrlWidget);
    connect(this, SIGNAL(inputUrl(const QStringList&)), parent, SIGNAL(load(const QStringList&)));
    connect(d->sure, &QPushButton::clicked, this, [d]
    {
        d->onSure();
    });

    connect(d->cancel, &QPushButton::clicked, this, &QInputUrlWidget::hide);

    connect(this, &QInputUrlWidget::showInit, this, [=]
    {
        QTimer::singleShot(0, this, [=]
        {
            d->url->selectAll();
            d->url->setFocus();
        });
    });
}
