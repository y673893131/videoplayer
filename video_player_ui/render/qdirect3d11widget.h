#ifndef Q_DIRECT3D11_WIDGET_H
#define Q_DIRECT3D11_WIDGET_H

#include "qd3d11widget.h"
#include "videoframe.h"

class QDirect3D11Widget : public QD3D11Widget
{
    Q_OBJECT

public:
    QDirect3D11Widget(QWidget * parent);
    virtual ~QDirect3D11Widget() override;

private:
    void render() override;

public slots:
    void onViewAdjust(bool);
    void onAppendFrame(void*);
    void onVideoSizeChanged(int,int);
    void onStart();
    void onStop();
private:
    _video_frame_* m_pFrame;
    bool m_bViewAdjust, m_bStoped;
};

#endif /*Q_DIRECT3D11_WIDGET_H*/
