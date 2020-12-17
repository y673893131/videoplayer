#ifndef QDOUYUWIDGET_H
#define QDOUYUWIDGET_H

#include "framelesswidget/framelesswidget.h"
class QTimer;
class QDouyuWidget : public QFrameLessWidget
{
    Q_OBJECT
public:
    explicit QDouyuWidget(QWidget *parent = nullptr);

signals:
    void play(const QString&);
    void showIndex(int);
public slots:

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
private:
    QTimer* m_timer;
};

#endif // QDOUYUWIDGET_H
