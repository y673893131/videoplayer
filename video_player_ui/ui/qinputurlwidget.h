#ifndef QINPUTURLWIDGET_H
#define QINPUTURLWIDGET_H

#include <QDialog>
#include "framelesswidget/framelesswidget.h"

class QInputUrlWidget : public QFrameLessWidget
{
    Q_OBJECT
public:
    QInputUrlWidget(QWidget *parent = nullptr);
signals:
    void inputUrl(const QStringList&);
    void showInit();
};

#endif // QINPUTURLWIDGET_H
