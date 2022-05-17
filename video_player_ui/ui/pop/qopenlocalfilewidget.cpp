#include "qopenlocalfilewidget.h"
#include <QDebug>

#include "qpopwidgetprivate.h"

class QOpenLocalFileWidgetPrivate : public QPopWidgetPrivate
{
    VP_DECLARE_PUBLIC(QOpenLocalFileWidget)
    inline QOpenLocalFileWidgetPrivate(QOpenLocalFileWidget* parent)
        : QPopWidgetPrivate(parent)
    {
        setKey("local_file");
    }
};

QOpenLocalFileWidget::QOpenLocalFileWidget(QWidget *parent)
    : QPopWidget(new QOpenLocalFileWidgetPrivate(this), parent)
{
}

void QOpenLocalFileWidget::initConnect(QWidget* parent)
{
    connect(this, SIGNAL(openLoaleFile()), parent, SIGNAL(loadFile()));
}

void QOpenLocalFileWidget::onShowCenter()
{
    emit openLoaleFile();
}
