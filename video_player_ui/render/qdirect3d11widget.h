#ifndef Q_DIRECT3D11_WIDGET_H
#define Q_DIRECT3D11_WIDGET_H

#ifdef WIN32
#include "native/qnativewidget.h"

#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <D3DX10math.h>
#include "dx11/inputclass.h"
#include "dx11/graphicsclass.h"

class QDirect3D11Widget : public QNativeWidget
{
    Q_OBJECT

public:
    QDirect3D11Widget(QWidget * parent);
    virtual ~QDirect3D11Widget() override;

private:
    bool initialize() override;
    void release();
    void render() override;

public slots:
    void onViewAdjust(bool) override;
    void onVideoSizeChanged(int,int);
    void onAppendFreq(float*, unsigned int);
    void onStart();
    void onStop();
private:
    GraphicsClass* m_pGraphics;
    bool m_bStoped;
};
#endif
#endif /*Q_DIRECT3D11_WIDGET_H*/
