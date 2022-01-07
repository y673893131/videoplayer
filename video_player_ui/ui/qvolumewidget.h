#ifndef QVOLUMEWIDGET_H
#define QVOLUMEWIDGET_H

#include <QDialog>
#include "framelesswidget/framelesswidget.h"

class QColumeSlider;
class QVolumeWidget : public QFrameLessWidget
{
    Q_OBJECT
public:
    QVolumeWidget(QWidget *parent = nullptr);
    void initConnect();
private slots:
    void onLoadConfig();
    void onVolChanged();
    void onSetVol();
    void onShow(bool, const QPoint&, const QSize&);
    void onTimerHide();
private:
    QWidget* m_parent;
    QColumeSlider* m_volume;
    QTimer* m_timerSetVol;
    QTimer* m_timerHide;
};

#endif // QVOLUMEWIDGET_H
