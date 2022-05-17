#ifndef QOPENLOCALFILEWIDGET_H
#define QOPENLOCALFILEWIDGET_H

#include "qpopwidget.h"

class QOpenLocalFileWidgetPrivate;
class QOpenLocalFileWidget : public QPopWidget
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QOpenLocalFileWidget)

signals:
    void openLoaleFile();
public:
    QOpenLocalFileWidget(QWidget *parent = nullptr);
    void initConnect(QWidget*) override;

private:
    void onShowCenter() override;
};

#endif // QOPENLOCALFILEWIDGET_H
