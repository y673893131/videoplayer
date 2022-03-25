#ifndef Q_DIRECT3D11_WIDGET_H
#define Q_DIRECT3D11_WIDGET_H

#ifdef WIN32
#include "native/qnativewidget.h"
#include "video_pimpl.h"

class QDirect3D11WidgetPrivate;
class QDirect3D11Widget : public QNativeWidget
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QDirect3D11Widget)

public:
    explicit QDirect3D11Widget(QWidget * parent = nullptr);
    virtual ~QDirect3D11Widget() override;

private:
    bool init(const QSize& size, float x, float y) override;
    void render(_VideoFramePtr frame, float* freq, unsigned int freqCount) override;
    void resetView(float, float) override;

public slots:
    void onStart();
    void onStop();
};
#endif
#endif /*Q_DIRECT3D11_WIDGET_H*/
