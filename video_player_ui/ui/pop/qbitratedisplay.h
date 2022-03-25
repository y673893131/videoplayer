#ifndef QBITRATEDISPLAY_H
#define QBITRATEDISPLAY_H

#include <QWidget>
#include <video_pimpl.h>

class QBitRateDisplayPrivate;
class QBitRateDisplay : public QWidget
{
    Q_OBJECT
    VP_DECLARE(QBitRateDisplay)
    VP_DECLARE_PRIVATE(QBitRateDisplay)
public:
    explicit QBitRateDisplay(QWidget *parent = nullptr);
    void append(int64_t);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // QBITRATEDISPLAY_H
