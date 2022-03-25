#ifndef QPOPWIDGET_H
#define QPOPWIDGET_H

#include "framelesswidget/framelesswidget.h"

class QPopWidgetPrivate;
class QPopWidget : public QFrameLessWidget
{
    Q_OBJECT
protected:
    VP_DECLARE_PRIVATE(QPopWidget)
public:
    QPopWidget(QWidget *parent = nullptr);
    QPopWidget(QPopWidgetPrivate* pri, QWidget *parent = nullptr);

    virtual void initConnect(QWidget*);
signals:
    void inputUrl(const QStringList&);
    void showInit();
protected slots:
    void onShow(const QString&);
    virtual void onShowCenter();

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // QPOPWIDGET_H
