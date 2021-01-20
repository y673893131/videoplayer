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

    void setPos(int);
signals:
    void gotoPos(int);
    void getPreview(int);
public slots:
    void onPreview(void* data, int, int);
    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    QWidget* m_timeWidget;
    QLabel* m_preview, *m_time;
    QTimer* m_getPreview;
    int m_nPreview;
    bool m_bHandup;
    QPoint m_timePt;
};

#endif // QPROGRESSSLIDER_H
