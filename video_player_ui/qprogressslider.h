#ifndef QPROGRESSSLIDER_H
#define QPROGRESSSLIDER_H

#include <QSlider>
#include <QLabel>
#include <QTimer>
class QProgressSlider : public QSlider
{
    Q_OBJECT

public:
    explicit QProgressSlider(Qt::Orientation orientation, QWidget* parent = nullptr, QWidget* grandParent = nullptr);

signals:
    void getPreview(int);
public slots:
    void onPreview(void* data, int, int);
    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    QLabel* m_preview;
    QTimer* m_timer,* m_getPreview;
    int m_nPreview;
};

#endif // QPROGRESSSLIDER_H
