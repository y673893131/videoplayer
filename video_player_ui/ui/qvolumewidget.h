#ifndef QVOLUMEWIDGET_H
#define QVOLUMEWIDGET_H

#include "framelesswidget/framelesswidget.h"

class QVolumeWidgetPrivate;
class QVolumeWidget : public QFrameLessWidget
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QVolumeWidget)
public:
    QVolumeWidget(QWidget *parent = nullptr);
    void initConnect();
private slots:
    void onLoadConfig();
    void onVolChanged();
    void onSetVol();
    void onShow(bool, const QPoint&, const QSize&);
    void onTimerHide();
};

#endif // QVOLUMEWIDGET_H
