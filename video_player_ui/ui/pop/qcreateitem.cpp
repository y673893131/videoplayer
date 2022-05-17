#include "qcreateitem.h"
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include "framelesswidget/util.h"
#include <QDebug>
QCreateItem::QCreateItem()
    : m_title(nullptr)
{
}

QWidget *QCreateItem::createTitle(const QString & sText, QWidget *parent)
{
    auto title_widget = new QWidget(parent);
    m_title = title_widget;
    title_widget->setObjectName("input_url_title_widget");
    auto title = new QLabel(sText, title_widget);
    title->setObjectName("input_url_title_label");

    auto close = new QPushButton(parent);
    close->setObjectName("btn_close");

    auto layoutTitle = new QHBoxLayout(title_widget);
    layoutTitle->addWidget(title);
    layoutTitle->addStretch();
    layoutTitle->addWidget(close);

    CALC_WIDGET_HEIGHT(title_widget, 40);

    parent->connect(close, &QPushButton::clicked, parent, &QWidget::hide);
    return title_widget;
}

bool QCreateItem::isTitleCapture()
{
    return m_title->underMouse();
}
