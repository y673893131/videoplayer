#include "qtoolbase.h"
#include <QPainter>
#include <QStyleOption>
#include "ui/qtoolwidgets.h"

QToolBase::QToolBase(QWidget *parent, bool bAutoHide)
    : QWidget(parent)
    , m_parent(parent)
{
    if(bAutoHide)
    {
        auto toolWidget = qobject_cast<QToolWidgets*>(m_parent);
        connect(toolWidget, &QToolWidgets::hideOrShow, this, &QToolBase::onAutoVisable);
    }
}

void QToolBase::onAutoVisable(bool bHide)
{
    setVisible(!bHide);
}

void QToolBase::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}
