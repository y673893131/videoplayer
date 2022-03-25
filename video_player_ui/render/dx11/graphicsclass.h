////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "textureshaderclass.h"
#include "bitmapclass.h"
#include "freqclass.h"
#include "render/videoframe.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

    bool Initialize(unsigned int, unsigned int, HWND, float, float);
	void Shutdown();
    bool Frame(_VideoFramePtr);
    bool Freq(float*, unsigned int);
    void ResetViewport(float, float);
private:
    bool Render(float, _VideoFramePtr,float*, unsigned int);

private:
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	TextureShaderClass* m_TextureShader;
	BitmapClass* m_Bitmap;
    FreqClass* m_freq;
};

#endif
