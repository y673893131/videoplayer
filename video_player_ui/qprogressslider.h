#ifndef QPROGRESSSLIDER_H
#define QPROGRESSSLIDER_H

#include <QSlider>
class QProgressSlider : public QSlider
{
    Q_OBJECT

public:
    explicit QProgressSlider(Qt::Orientation orientation, QWidget* parent = nullptr);

    // QWidget interface
protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
};

#endif // QPROGRESSSLIDER_H
