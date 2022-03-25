#ifndef QNATIVEWIDGET_H
#define QNATIVEWIDGET_H

#include <QWidget>
#include "../videoframe.h"
#include "video_pimpl.h"

class QNativeWidgetPrivate;
class QNativeWidget : public QWidget
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QNativeWidget)
public:
    explicit QNativeWidget(QWidget *parent = nullptr);
    QNativeWidget(QNativeWidgetPrivate*, QWidget *parent = nullptr);
    virtual ~QNativeWidget() override;

signals:
    void deviceInitialized(bool);
public slots:
    void onAppendFrame(_VideoFramePtr frame);
    void onViewAdjust(bool);
    virtual void onVideoSizeChanged(int,int);
    virtual void onAppendFreq(float*, unsigned int);
protected:
    void setup();
    bool initialize();
    virtual bool init(const QSize&, float x, float y) = 0;
    virtual void resetView(float, float) = 0;
    virtual void render(_VideoFramePtr frame, float* freq, unsigned int freqCount) = 0;
    void removeFrame();

public slots:
    void render();
    // Qt Events
private:
    QPaintEngine*  paintEngine() const override;
    void           showEvent(QShowEvent * event) override;
    void           paintEvent(QPaintEvent * event) override;
    void           resizeEvent(QResizeEvent * event) override;
protected:
    VP_DECLARE(QNativeWidget)
};

#endif // QNATIVEWIDGET_H
