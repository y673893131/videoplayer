////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textureclass.h"
#include "render/videoframe.h"
#include <QDebug>
TextureClass::TextureClass()
{
    m_width = 0;
    m_height = 0;
    memset(m_texture,0x00,sizeof(m_texture));
    memset(m_textureView,0x00,sizeof(m_textureView));
}

TextureClass::TextureClass(const TextureClass& /*other*/)
{
}

TextureClass::~TextureClass()
{
}

bool TextureClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;

    D3D11_TEXTURE2D_DESC textureDesc;
    HRESULT hResult;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    // Setup the description of the texture.
    textureDesc.Height = height;
    textureDesc.Width = width;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
#ifdef USE_RGBA
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
#else
    textureDesc.Format = DXGI_FORMAT_R8_UNORM;
#endif

    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
//#define USE_DYNAMIC
#ifdef USE_DYNAMIC
    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;
#else
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
#endif
    // Setup the shader resource view description.
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

    unsigned int widths[] = {width, width / 2, width / 2};
    unsigned int heights[] = {height, height / 2, height / 2};

    for(int i = 0; i < TEX_SIZE; ++i)
    {
        textureDesc.Width = widths[i];
        textureDesc.Height = heights[i];

        // Create the empty texture.
        hResult = device->CreateTexture2D(&textureDesc, nullptr, &m_texture[i]);
        if(FAILED(hResult))
        {
            qDebug() << __FUNCTION__ << __LINE__ << m_width << m_height << "CreateTexture2D";
            return false;
        }

        // Create the shader resource view for the texture.
        hResult = device->CreateShaderResourceView(m_texture[i], &srvDesc, &m_textureView[i]);
        if(FAILED(hResult))
        {
            qDebug() << __FUNCTION__ << __LINE__ << m_width << m_height << "CreateShaderResourceView";
            return false;
        }
#ifndef USE_DYNAMIC
        // Generate mipmaps for this texture.
        deviceContext->GenerateMips(m_textureView[i]);
#endif
    }


    return true;
}

void TextureClass::Render(ID3D11Device* device, ID3D11DeviceContext *deviceContext, void* data)
{
    if(!data)
        return;
    auto frame = reinterpret_cast<_video_frame_*>(data);
    if(frame->w != m_width || frame->h != m_height)
    {
        Shutdown();
        Initialize(device, deviceContext, frame->w, frame->h);
    }

    unsigned int w = 0,h = 0;
    for (int i = 0; i < TEX_SIZE; ++i)
    {
        auto pData = frame->data(i, w, h);
#ifdef USE_RGBA
        w *= 4;
#endif
#ifndef USE_DYNAMIC
        deviceContext->UpdateSubresource(m_texture[i], 0, nullptr, pData, w, 0);
#else
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        auto result = deviceContext->Map(m_texture[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if(FAILED(result))
        {
            break;
        }

        CopyMemory(mappedResource.pData, pData, w * h);

        deviceContext->Unmap(m_texture[i], 0);

#endif
    }
}

void TextureClass::Shutdown()
{
    // Release the texture view resource.
    for(int i = 0; i < TEX_SIZE;++i)
    {
        if(m_textureView[i])
        {
            m_textureView[i]->Release();
            m_textureView[i] = nullptr;
        }

        // Release the texture resource.
        if(m_texture[i])
        {
            m_texture[i]->Release();
            m_texture[i] = nullptr;
        }
    }
}

ID3D11Texture2D ** TextureClass::tex()
{
    return m_texture;
}


ID3D11ShaderResourceView** TextureClass::GetTexture()
{
    return m_textureView;
}
