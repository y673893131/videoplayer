#ifndef FREQCLASS_H
#define FREQCLASS_H

#include <D3D11.h>
#include <D3DX10math.h>

class FreqClass
{
private:
    struct VertexType
    {
        VertexType()
        {
            bcolor = true;
        }
        D3DXVECTOR3 position;
        D3DXVECTOR2 texture;
        D3DXVECTOR4 color;
        bool        bcolor;
    };

public:
    FreqClass();
    ~FreqClass();

    bool Initialize(ID3D11Device*, unsigned int, unsigned int, ID3D11DeviceContext*, unsigned int, unsigned int);
    void Shutdown();
    bool Render(ID3D11DeviceContext*, int, int, float*, unsigned int);

    int GetIndexCount();
private:
    bool InitializeBuffers(ID3D11Device*);
    void ShutdownBuffers();
    bool UpdateBuffers(ID3D11DeviceContext*, int, int, float*, unsigned int);
    void RenderBuffers(ID3D11DeviceContext*);

    void UpdateBuffer(VertexType* dst, float* src,unsigned int srcCount,float x,float y, float w, float h, D3DXVECTOR4* color, bool bBar = false);

private:
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    unsigned int m_count;
    unsigned int m_screenWidth, m_screenHeight;
    unsigned int m_width, m_height;
    unsigned int m_vertexCount, m_indexCount;
    int m_previousPosX, m_previousPosY;
};

#endif // FREQCLASS_H
