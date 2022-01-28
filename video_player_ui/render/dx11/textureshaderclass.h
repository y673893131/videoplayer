////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURESHADERCLASS_H_
#define _TEXTURESHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <D3D11.h>
#include <D3DX10math.h>
#include <D3DX11async.h>
#include <fstream>
#include "textureclass.h"
#include "freqclass.h"
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Class name: TextureShaderClass
////////////////////////////////////////////////////////////////////////////////
class TextureShaderClass
{
private:
	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

public:
	TextureShaderClass();
	TextureShaderClass(const TextureShaderClass&);
	~TextureShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
    bool Render(ID3D11Device* device, ID3D11DeviceContext*, int, D3DXMATRIX&, D3DXMATRIX&, D3DXMATRIX&, TextureClass*, void* data);
    bool Render(ID3D11Device* device, ID3D11DeviceContext*, int, D3DXMATRIX&, D3DXMATRIX&, D3DXMATRIX&, FreqClass*, float* data);

private:
    bool InitializeShader(ID3D11Device*, HWND);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

    bool SetShaderParameters(ID3D11Device* device, ID3D11DeviceContext*, D3DXMATRIX&, D3DXMATRIX&, D3DXMATRIX&, TextureClass*, void* data);
    bool SetShaderParameters(ID3D11Device* device, ID3D11DeviceContext*, D3DXMATRIX&, D3DXMATRIX&, D3DXMATRIX&, FreqClass*, float* data);
    void RenderShader(ID3D11DeviceContext*, unsigned int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
};

#endif
