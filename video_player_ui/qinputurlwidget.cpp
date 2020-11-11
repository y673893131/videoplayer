#include "qinputurlwidget.h"
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QTimer>

QInputUrlWidget::QInputUrlWidget(QWidget *parent)
    :QFrameLessWidget(parent)
{
    setAttribute(Qt::WA_ShowModal, true);
    setObjectName("input_url_widget");
    auto widget = new QWidget(this);
    widget->setObjectName("input_url_widget");
    auto title_widget = new QWidget(widget);
    title_widget->setObjectName("input_url_title_widget");
    auto title = new QLabel(tr("input url"), title_widget);
    title->setObjectName("input_url_title_label");
    auto url = new QLineEdit(/*"rtmp://172.16.62.127/live/stream", */widget);
    url->setObjectName("input_url_edit");
    url->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    url->setPlaceholderText("like rtsp/rtmp...");
    auto btn_wdiget = new QWidget(widget);
    btn_wdiget->setObjectName("inout_url_btn_widget");
    auto sure = new QPushButton(tr("sure"), btn_wdiget);
    sure->setObjectName("input_url_btn");
    auto cancel = new QPushButton(tr("cancel"), btn_wdiget);
    cancel->setObjectName("input_url_btn");

    auto layoutThis = new QVBoxLayout(this);
    auto layout = new QVBoxLayout(widget);
    auto layoutTitle = new QHBoxLayout(title_widget);
    auto layoutBtn = new QHBoxLayout(btn_wdiget);
    layoutThis->setMargin(0);
    layoutThis->addWidget(widget);
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

    connect(sure, &QPushButton::clicked, [this, url]
    {
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
    });
}
