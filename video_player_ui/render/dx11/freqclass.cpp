#include "freqclass.h"
#include "framelesswidget/util.h"

FreqClass::FreqClass()
{
    m_count = 128 * 3;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}

FreqClass::~FreqClass()
{

}

bool FreqClass::Initialize(ID3D11Device* device, unsigned int screenWidth, unsigned int screenHeight,
                             ID3D11DeviceContext* /*deviceContext*/, unsigned int bitmapWidth, unsigned int bitmapHeight)
{
    bool result;

    // Store the screen size.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Store the size in pixels that this bitmap should be rendered at.
    m_width = bitmapWidth;
    m_height = bitmapHeight;

    // Initialize the previous rendering position to negative one.
    m_previousPosX = -1;
    m_previousPosY = -1;

    // Initialize the vertex and index buffers.
    result = InitializeBuffers(device);
    if(!result)
    {
        return false;
    }

    return true;
}

void FreqClass::Shutdown()
{
    // Shutdown the vertex and index buffers.
    ShutdownBuffers();

    return;
}

bool FreqClass::Render(ID3D11DeviceContext *deviceContext, int positionX, int positionY, float *data, unsigned int size)
{
    bool result;

    // Re-build the dynamic vertex buffer for rendering to possibly a different location on the screen.
    result = UpdateBuffers(deviceContext, positionX, positionY, data, size);
    if(!result)
    {
        return false;
    }

    // Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
    RenderBuffers(deviceContext);

    return true;
}

int FreqClass::GetIndexCount()
{
    return static_cast<int>(m_indexCount);
}

bool FreqClass::InitializeBuffers(ID3D11Device * device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    unsigned int i;

    // Set the number of vertices in the vertex array.
    m_vertexCount = 6 * m_count;

    // Set the number of indices in the index array.
    m_indexCount = m_vertexCount;

    // Create the vertex array.
    vertices = new VertexType[m_vertexCount];
    if(!vertices)
    {
        return false;
    }

    // Create the index array.
    indices = new unsigned long[m_indexCount];
    if(!indices)
    {
        return false;
    }

    // Initialize vertex array to zeros at first.
    memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

    // Load the index array with data.
    for(i=0; i<m_indexCount; i++)
    {
        indices[i] = i;
    }

    // Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if(FAILED(result))
    {
        return false;
    }

    // Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if(FAILED(result))
    {
        return false;
    }

    // Release the arrays now that the vertex and index buffers have been created and loaded.
    delete [] vertices;
    vertices = nullptr;

    delete [] indices;
    indices = nullptr;

    return true;
}

void FreqClass::ShutdownBuffers()
{
    // Release the index buffer.
    if(m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = nullptr;
    }

    // Release the vertex buffer.
    if(m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }

    return;
}

bool FreqClass::UpdateBuffers(
        ID3D11DeviceContext* deviceContext,
        int positionX,
        int positionY,
        float* data,
        unsigned int count)
{
    float left, right, top, bottom;
    static VertexType* vertices = new VertexType[m_vertexCount];
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* verticesPtr;
    HRESULT result;

    // If it has changed then update the position it is being rendered to.
    m_previousPosX = positionX;
    m_previousPosY = positionY;

    // Calculate the screen coordinates of the left side of the bitmap.
    left =((m_screenWidth / 2.0f) * -1) + 1.0f * positionX;

    // Calculate the screen coordinates of the right side of the bitmap.
    right = left + 1.0f * m_width;

    // Calculate the screen coordinates of the top of the bitmap.
    top = m_screenHeight / 2.0f - 1.0f * positionY;

    // Calculate the screen coordinates of the bottom of the bitmap.
    bottom = top - 1.0f * m_height;

//    // Create the vertex array.
//    vertices = new VertexType[m_vertexCount];
//    if(!vertices)
//    {
//        return false;
//    }

//    unsigned int count = 128;
    auto size = CALC_WIDGET_SIZE(nullptr, count / 64 * 400, 200);
    float maxHeight = size.height();
    float maxWidth = size.width();
    float xStart = -maxWidth / 2.0f;
    float yStart = -maxHeight / 2.0f;
    float xWidth = maxWidth * 4.0f / (count * 5.0f);

    auto color = D3DXVECTOR4(1.0f,0.5f,0.5f,0.5f);
    static float* last = new float[count];
    UpdateBuffer(vertices, last, count, xStart, yStart, xWidth, maxHeight, &color, true);
    memcpy(last, data, sizeof(float) * count);

    color = D3DXVECTOR4(1.0f, 0.36f, 0.22f, 1.0f);
    UpdateBuffer(vertices + count * 6, data, count, xStart, yStart, xWidth, maxHeight, &color);

    color = D3DXVECTOR4(0.9f,0.2f,0.2f,0.5f);
    UpdateBuffer(vertices + 2 * count * 6, data, count, xStart, yStart, xWidth, -maxHeight / 2, &color);

    // Lock the vertex buffer so it can be written to.
    result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if(FAILED(result))
    {
//        delete[] vertices;
        return false;
    }

    // Get a pointer to the data in the vertex buffer.
    verticesPtr = reinterpret_cast<VertexType*>(mappedResource.pData);

    // Copy the data into the vertex buffer.
    memcpy(verticesPtr, vertices, (sizeof(VertexType) * m_vertexCount));

    // Unlock the vertex buffer.
    deviceContext->Unmap(m_vertexBuffer, 0);

    // Release the vertex array as it is no longer needed.
//    delete [] vertices;
//    vertices = nullptr;

    return true;
}

void FreqClass::RenderBuffers(ID3D11DeviceContext *deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset.
    stride = sizeof(VertexType);
    offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

void FreqClass::UpdateBuffer(
        FreqClass::VertexType *dst, float *src, unsigned int srcCount,
        float x, float y, float w, float h,
        D3DXVECTOR4* pColor, bool bBar)
{
    auto color = *pColor;
    float fHeight = 0.0f;
    float fHeightUp = 0.0f;
    float fHeightDown = 0.0f;
    float fBar = h * 0.02f;
    for (unsigned int i = 0; i < srcCount; ++i)
    {
        unsigned int index = i * 6;
        fHeight = src[i] * h;
        if(h > 0)
            fHeightUp = fHeight;
        else
            fHeightDown = fHeight;
        if(bBar)
        {
            if(fHeight <= fBar)
            {
                fHeightUp = 0.0f;
                fHeightUp = 0.0f;
            }
            else
            {
                fHeightUp = src[i] * h;
                fHeightDown = fHeightUp - fBar;
            }
        }
        dst[index].position = D3DXVECTOR3(x, y + fHeightDown, 0.0f);  // left bottom
        dst[index].color = color;

        dst[index + 1].position = D3DXVECTOR3(x, y + fHeightUp, 0.0f);  // left top
        dst[index + 1].color = color;

        dst[index + 2].position = D3DXVECTOR3(x + w, y + fHeightUp, 0.0f);  // right top
        dst[index + 2].color = color;

        dst[index + 3].position = D3DXVECTOR3(x, y + fHeightDown, 0.0f);  // left bottom
        dst[index + 3].color = color;

        dst[index + 4].position = D3DXVECTOR3(x + w, y + fHeightUp, 0.0f);  // right top
        dst[index + 4].color = color;

        dst[index + 5].position = D3DXVECTOR3(x + w, y + fHeightDown, 0.0f);  // right bottom
        dst[index + 5].color = color;

        x += w + w / 4;
    }
}
