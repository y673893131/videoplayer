#ifndef QMEDIAINFOWIDGET_H
#define QMEDIAINFOWIDGET_H

#include "qpopwidget.h"

class QMediaInfoWidgetPrivate;
class QMediaInfoWidget : public QPopWidget
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QMediaInfoWidget)
public:
    QMediaInfoWidget(QWidget *parent = nullptr);

    void initConnect(QWidget*) override;

private slots:
    void onShowCenter() override;
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // QMEDIAINFOWIDGET_H
