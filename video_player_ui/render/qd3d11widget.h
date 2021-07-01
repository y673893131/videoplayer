#ifndef Q_D3D11_WIDGET_H
#define Q_D3D11_WIDGET_H

#include <stdexcept>
#include <QtGlobal>
#include <QWidget>
#include <QTimer>

#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <D3DX10math.h>
#include "dx11/inputclass.h"
#include "dx11/graphicsclass.h"

class QD3D11Widget : public QWidget
{
    Q_OBJECT

public:
    QD3D11Widget(QWidget * parent);
    virtual ~QD3D11Widget() override;

    void setSize(int w, int h);
    void release();
protected:
    virtual void render();
protected:
    bool initialize();

    // Qt Events
private:
    QPaintEngine*  paintEngine() const override;
    void           showEvent(QShowEvent * event) override;
    void           paintEvent(QPaintEvent * event) override;
    void           resizeEvent(QResizeEvent * event) override;
signals:
    void deviceInitialized(bool success);

    void eventHandled();
    void widgetResized();

    void rendered();

    void keyPressed(QKeyEvent *);
    void mouseMoved(QMouseEvent *);
    void mouseClicked(QMouseEvent *);
    void mouseReleased(QMouseEvent *);
    void flush();
protected slots:
    void onFrame();

    // Getters / Setters
public:
    HWND const & nativeHandle() const { return m_hWnd; }
//    D3DCOLORVALUE * BackColor() { return &m_BackColor; }

protected:
    HWND m_hWnd;
    unsigned int m_width, m_height;
    bool m_bDeviceInitialized;
    GraphicsClass* m_pGraphics;
};

// ############################################################################
// ############################## Utils #######################################
// ############################################################################
#define ReleaseObject(object)                                                                 \
    if ((object) != Q_NULLPTR)                                                                \
    {                                                                                         \
        object->Release();                                                                    \
        object = Q_NULLPTR;                                                                   \
    }
#define ReleaseHandle(object)                                                                 \
    if ((object) != Q_NULLPTR)                                                                \
    {                                                                                         \
        CloseHandle(object);                                                                  \
        object = Q_NULLPTR;                                                                   \
    }

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr)
        : std::runtime_error(HrToString(hr))
        , m_hr(hr)
    {
    }
    HRESULT Error() const { return m_hr; }

private:
    const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr)) { throw HrException(hr); }
}

#define DXCall(func) ThrowIfFailed(func)

#endif /*Q_D3D11_WIDGET_H*/
