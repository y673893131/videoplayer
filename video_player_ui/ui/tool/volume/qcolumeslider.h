#ifndef QCOLUMESLIDER_H
#define QCOLUMESLIDER_H

#include <QSlider>

class QColumeSlider : public QSlider
{
    Q_OBJECT
public:
    explicit QColumeSlider(Qt::Orientation orientation, QWidget* parent = nullptr);
signals:
    void jumpStr(const QString&);
public slots:
    void onJump(bool);
    void onValueChanged(int);
};

#endif // QCOLUMESLIDER_H
