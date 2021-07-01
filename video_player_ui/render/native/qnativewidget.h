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
    void flush();
public slots:
    void onAppendFrame(void *frame);
protected:
    virtual bool initialize() = 0;
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
};

#endif // QNATIVEWIDGET_H
