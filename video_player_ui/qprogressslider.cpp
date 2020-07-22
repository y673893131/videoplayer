#include "qprogressslider.h"
#include <QMouseEvent>

QProgressSlider::QProgressSlider(Qt::Orientation orientation, QWidget* parent)
    :QSlider(orientation, parent)
{

}

void QProgressSlider::mouseReleaseEvent(QMouseEvent *event)
{
    QSlider::mouseReleaseEvent(event);
    auto pos = event->pos();
    auto pro = pos.x() * 1.0 / width();
    int num = pro * maximum();
    setValue(num);
    emit sliderMoved(num);
}

void QProgressSlider::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);
    QSlider::enterEvent(event);
}
