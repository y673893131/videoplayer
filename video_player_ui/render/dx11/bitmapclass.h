////////////////////////////////////////////////////////////////////////////////
// Filename: bitmapclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _BITMAPCLASS_H_
#define _BITMAPCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <D3D11.h>
#include <D3DX10math.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "textureclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: BitmapClass
////////////////////////////////////////////////////////////////////////////////
class BitmapClass
{
private:
	struct VertexType
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};

public:
	BitmapClass();
	BitmapClass(const BitmapClass&);
	~BitmapClass();

    bool Initialize(ID3D11Device*, unsigned int, unsigned int, ID3D11DeviceContext*, unsigned int, unsigned int);
	void Shutdown();
    bool Render(ID3D11DeviceContext*, int, int);

	int GetIndexCount();
    TextureClass *texture();
private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(ID3D11DeviceContext*, int, int);
	void RenderBuffers(ID3D11DeviceContext*);

    bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*);
	void ReleaseTexture();

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    unsigned int m_vertexCount, m_indexCount;
	TextureClass* m_Texture;
    unsigned int m_screenWidth, m_screenHeight;
    unsigned int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
};

#endif
