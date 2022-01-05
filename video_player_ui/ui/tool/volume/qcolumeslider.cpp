#include "qcolumeslider.h"
#include <QKeyEvent>

QColumeSlider::QColumeSlider(Qt::Orientation orientation, QWidget* parent)
    : QSlider(orientation, parent)
{
    setObjectName("slider_voice");
    setRange(0, 200);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    connect(this, &QColumeSlider::valueChanged, this, &QColumeSlider::onValueChanged);
}

void QColumeSlider::onJump(bool bBigger)
{
    int step = value() + (bBigger ? 5 : -5);
    if(step < 0)
        step = 0;
    else if(step > maximum())
        step = maximum();
    setValue(step);
}

void QColumeSlider::onValueChanged(int value)
{
    QString sValue = QString("%1: %2").arg(tr("volume")).arg(value);
    emit jumpStr(sValue);
}
