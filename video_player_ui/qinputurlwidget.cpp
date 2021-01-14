#include "qinputurlwidget.h"
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QDebug>

QInputUrlWidget::QInputUrlWidget(QWidget *parent)
    :QFrameLessWidget(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowModal, true);

    setDragSelf(true);
    auto hwnd = reinterpret_cast<HWND>(this->winId());
    setAreo(hwnd);
    setShadow(hwnd);
    setResizeable(false);

    setObjectName("input_url_widget");
    auto widget = this;
    auto title_widget = new QWidget(widget);
    title_widget->setObjectName("input_url_title_widget");
    auto title = new QLabel(tr("input url"), title_widget);
    title->setObjectName("input_url_title_label");
    auto url = new QLineEdit(widget);
    url->setObjectName("input_url_edit");
    url->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    url->setPlaceholderText("like rtsp/rtmp...");
    auto btn_wdiget = new QWidget(widget);
    btn_wdiget->setObjectName("inout_url_btn_widget");
    auto sure = new QPushButton(tr("sure"), btn_wdiget);
    sure->setObjectName("input_url_btn");
    auto cancel = new QPushButton(tr("cancel"), btn_wdiget);
    cancel->setObjectName("input_url_btn");

    auto layout = new QVBoxLayout(widget);
    auto layoutTitle = new QHBoxLayout(title_widget);
    auto layoutBtn = new QHBoxLayout(btn_wdiget);
    layout->setMargin(0);
    layout->addWidget(title_widget);
    layout->addStretch();
    layout->addWidget(url);
    layout->addStretch();
    layout->addWidget(btn_wdiget);

    layoutTitle->addWidget(title);
    layoutBtn->addStretch();
    layoutBtn->addWidget(sure);
    layoutBtn->addSpacing(20);
    layoutBtn->addWidget(cancel);
    layoutBtn->addStretch();

    CALC_WIDGET_HEIGHT(title_widget, 0.04f);
    CALC_WIDGET_SIZE(sure, 0.05f, 0.03f);
    CALC_WIDGET_SIZE(cancel, 0.05f, 0.03f);

    connect(sure, &QPushButton::clicked, [this, url]
    {
        if(url->text().isEmpty())
            return ;
        emit inputUrl(url->text());
        close();
    });

    connect(cancel, &QPushButton::clicked, [this]
    {
        close();
    });

    connect(this, &QInputUrlWidget::showInit, [=]
    {
        QTimer::singleShot(0, [=]
        {
            url->selectAll();
            url->setFocus();
        });

        show();
        CENTER_WIDGET(this);
    });
}

void QInputUrlWidget::showEvent(QShowEvent *event)
{
    CALC_WIDGET_SIZE(this, 0.18f, 0.15f);
    QFrameLessWidget::showEvent(event);
}
