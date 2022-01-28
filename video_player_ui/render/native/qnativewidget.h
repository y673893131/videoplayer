#ifndef QNATIVEWIDGET_H
#define QNATIVEWIDGET_H

#include <QWidget>
#include "../videoframe.h"

class QNativeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QNativeWidget(QWidget *parent = nullptr);
    virtual ~QNativeWidget() override;

signals:
    void deviceInitialized(bool);
public slots:
    void onAppendFrame(void *frame);
    virtual void onViewAdjust(bool);
protected:
    virtual bool initialize() = 0;
    void removeFrame();
    void removeFreq();
public slots:
    virtual void render() = 0;
    // Qt Events
private:
    QPaintEngine*  paintEngine() const override;
    void           showEvent(QShowEvent * event) override;
    void           paintEvent(QPaintEvent * event) override;
    void           resizeEvent(QResizeEvent * event) override;
protected:
    unsigned int m_width, m_height;
    bool m_bDeviceInitialized, m_bViewAdjust;
    _video_frame_* m_pFrame;
    float* m_freq;
    unsigned m_freqCount;
    float m_fScaleX;
    float m_fScaleY;
};

#endif // QNATIVEWIDGET_H
