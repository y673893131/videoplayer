////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_


//////////////
// INCLUDES //
//////////////
#include <D3D11.h>
#include <D3DX11tex.h>
#include "video_player_core.h"
#include "render/videoframe.h"

#ifdef FRAME_RGB
#define USE_RGBA
#endif

#ifdef USE_RGBA
#define TEX_SIZE 1
#else
#define TEX_SIZE 3
#endif
////////////////////////////////////////////////////////////////////////////////
// Class name: TextureClass
////////////////////////////////////////////////////////////////////////////////
class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	~TextureClass();

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned int width, unsigned int height);
    void Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, _VideoFramePtr frame);
	void Shutdown();
    ID3D11Texture2D **tex();
    ID3D11ShaderResourceView** GetTexture();
private:
#ifdef USE_RGBA
    ID3D11Texture2D* m_texture[1];
    ID3D11ShaderResourceView* m_textureView[1];
#else
    ID3D11Texture2D* m_texture[3];
    ID3D11ShaderResourceView* m_textureView[3];

//    ID3D11Buffer* m_textureBuffer[3];
#endif
    unsigned int m_width;
    unsigned int m_height;
};

#endif
