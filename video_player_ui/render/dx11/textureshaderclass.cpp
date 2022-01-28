////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textureshaderclass.h"
#include "textureclass.h"
#include <QFile>
#include <QDebug>

TextureShaderClass::TextureShaderClass()
{
    m_vertexShader = nullptr;
    m_pixelShader = nullptr;
    m_layout = nullptr;
    m_matrixBuffer = nullptr;
    m_sampleState = nullptr;
}


TextureShaderClass::TextureShaderClass(const TextureShaderClass& /*other*/)
{
}


TextureShaderClass::~TextureShaderClass()
{
}


bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
    result = InitializeShader(device, hwnd);
	if(!result)
	{
		return false;
	}

	return true;
}


void TextureShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}


bool TextureShaderClass::Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX& worldMatrix, D3DXMATRIX& viewMatrix,
                                D3DXMATRIX& projectionMatrix, TextureClass* texture, void* data)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
    result = SetShaderParameters(device, deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, data);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

    return true;
}

bool TextureShaderClass::Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX& worldMatrix, D3DXMATRIX& viewMatrix,
                                D3DXMATRIX& projectionMatrix, FreqClass* texture, float* data)
{
    bool result;
    // Set the shader parameters that it will use for rendering.
    result = SetShaderParameters(device, deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, data);
    if(!result)
    {
        return false;
    }

    // Now render the prepared buffers with the shader.
    RenderShader(deviceContext, indexCount);

    return true;
}


bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND /*hwnd*/)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[4];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;


	// Initialize the pointers this function will use to null.
    errorMessage = nullptr;
    vertexShaderBuffer = nullptr;
    pixelShaderBuffer = nullptr;

    QFile vs(":/shader/dx11_vs");
    vs.open(QFile::ReadOnly);
    auto vsData = vs.readAll();
    vs.close();
    // Compile the vertex shader code.
    result = D3DX11CompileFromMemory(vsData, static_cast<unsigned int>(vsData.length()), nullptr, nullptr, nullptr,
                            "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr,
                            &vertexShaderBuffer, &errorMessage, nullptr);
//    result = D3DX11CompileFromFile(vsFilename, nullptr, nullptr, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr,
//                                   &vertexShaderBuffer, &errorMessage, nullptr);
	if(FAILED(result))
	{
        qDebug() << __FUNCTION__ << __LINE__ << reinterpret_cast<char*>(errorMessage->GetBufferPointer());
        errorMessage->Release();
		return false;
	}

    QFile ps(":/shader/dx11_ps");
    ps.open(QFile::ReadOnly);
    auto psData = ps.readAll();
    ps.close();

    // Compile the pixel shader code.
    result = D3DX11CompileFromMemory(psData, static_cast<unsigned int>(psData.length()), nullptr, nullptr, nullptr,
                            "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr,
                            &pixelShaderBuffer, &errorMessage, nullptr);
//	result = D3DX11CompileFromFile(psFilename, nullptr, nullptr, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr,
//								   &pixelShaderBuffer, &errorMessage, nullptr);
	if(FAILED(result))
	{
        qDebug() << __FUNCTION__ << __LINE__ << reinterpret_cast<char*>(errorMessage->GetBufferPointer());
        errorMessage->Release();
		return false;
	}

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader);
	if(FAILED(result))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader);
	if(FAILED(result))
	{
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
    int ii = 0;
    polygonLayout[ii].SemanticName = "POSITION";
    polygonLayout[ii].SemanticIndex = 0;
    polygonLayout[ii].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[ii].InputSlot = 0;
    polygonLayout[ii].AlignedByteOffset = 0;
    polygonLayout[ii].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[ii].InstanceDataStepRate = 0;

    ++ii;
    polygonLayout[ii].SemanticName = "TEXCOORD";
    polygonLayout[ii].SemanticIndex = 0;
    polygonLayout[ii].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[ii].InputSlot = 0;
    polygonLayout[ii].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[ii].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[ii].InstanceDataStepRate = 0;

    ++ii;
    polygonLayout[ii].SemanticName = "COLOR";
    polygonLayout[ii].SemanticIndex = 0;
    polygonLayout[ii].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[ii].InputSlot = 0;
    polygonLayout[ii].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[ii].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[ii].InstanceDataStepRate = 0;

    ++ii;
    polygonLayout[ii].SemanticName = "bool";
    polygonLayout[ii].SemanticIndex = 0;
    polygonLayout[ii].Format = DXGI_FORMAT_R8_UNORM;
    polygonLayout[ii].InputSlot = 0;
    polygonLayout[ii].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[ii].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[ii].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                               &m_layout);
	if(FAILED(result))
	{
		return false;
	}


	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
    vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
    pixelShaderBuffer = nullptr;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&matrixBufferDesc, nullptr, &m_matrixBuffer);
	if(FAILED(result))
	{
		return false;
	}

    // Create a texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if(FAILED(result))
    {
        return false;
    }

	return true;
}


void TextureShaderClass::ShutdownShader()
{
	// Release the sampler state.
	if(m_sampleState)
	{
		m_sampleState->Release();
        m_sampleState = nullptr;
	}

	// Release the matrix constant buffer.
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
        m_matrixBuffer = nullptr;
	}

	// Release the layout.
	if(m_layout)
	{
		m_layout->Release();
        m_layout = nullptr;
	}

	// Release the pixel shader.
	if(m_pixelShader)
	{
		m_pixelShader->Release();
        m_pixelShader = nullptr;
	}

	// Release the vertex shader.
	if(m_vertexShader)
	{
		m_vertexShader->Release();
        m_vertexShader = nullptr;
	}

	return;
}


void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
    compileErrors = reinterpret_cast<char*>(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
    errorMessage = nullptr;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}


bool TextureShaderClass::SetShaderParameters(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX& worldMatrix, D3DXMATRIX& viewMatrix,
                                             D3DXMATRIX& projectionMatrix, TextureClass* texture, void* data)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
    dataPtr = reinterpret_cast<MatrixBufferType*>(mappedResource.pData);

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

    // Texture render
    texture->Render(device, deviceContext, data);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Set shader texture resource in the pixel shader.
    deviceContext->PSSetShaderResources(0, TEX_SIZE, texture->GetTexture());

    return true;
}

bool TextureShaderClass::SetShaderParameters(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX& worldMatrix, D3DXMATRIX& viewMatrix,
                                             D3DXMATRIX& projectionMatrix, FreqClass* texture, float* data)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;

    // Transpose the matrices to prepare them for the shader.
    D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
    D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
    D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if(FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr = reinterpret_cast<MatrixBufferType*>(mappedResource.pData);

    // Copy the matrices into the constant buffer.
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

    // Texture render
//    texture->Render(deviceContext, 0, 0, data, 128);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // Set shader texture resource in the pixel shader.
//    deviceContext->PSSetShaderResources(0, TEX_SIZE, texture->GetTexture());

    return true;
}


void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, unsigned int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
    deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
    deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}
