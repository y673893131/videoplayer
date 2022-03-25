#include "qpopwidget.h"
#include "qpopwidgetprivate.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>

QPopWidget::QPopWidget(QWidget *parent)
    :QFrameLessWidget(new QPopWidgetPrivate(this), qApp->desktop())
{
    VP_D(QPopWidget);
    d->init(parent);
}

QPopWidget::QPopWidget(QPopWidgetPrivate *pri, QWidget *parent)
    :QFrameLessWidget(pri, qApp->desktop())
{
    VP_D(QPopWidget);
    d->init(parent);
}

void QPopWidget::initConnect(QWidget*)
{
}

void QPopWidget::onShow(const QString & sKey)
{
    VP_D(QPopWidget);

    if(sKey == d->m_sKey)
    {
        qDebug() << __FUNCTION__ << sKey;
        emit showInit();
    }
}

void QPopWidget::onShowCenter()
{
    this->show();
    this->raise();
    CENTER_WIDGET(this);
}

void QPopWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QFrameLessWidget::paintEvent(event);
}

