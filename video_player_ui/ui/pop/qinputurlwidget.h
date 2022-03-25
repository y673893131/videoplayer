#ifndef QINPUTURLWIDGET_H
#define QINPUTURLWIDGET_H

#include "qpopwidget.h"

class QInputUrlWidgetPrivate;
class QInputUrlWidget : public QPopWidget
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QInputUrlWidget)
public:
    QInputUrlWidget(QWidget *parent = nullptr);
    void initConnect(QWidget*) override;
};

#endif // QINPUTURLWIDGET_H
