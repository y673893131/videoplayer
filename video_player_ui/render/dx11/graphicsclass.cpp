////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

GraphicsClass::GraphicsClass()
{
    m_D3D = nullptr;
    m_Camera = nullptr;
    m_TextureShader = nullptr;
    m_Bitmap = nullptr;
    m_freq = nullptr;
}


GraphicsClass::GraphicsClass(const GraphicsClass& /*other*/)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(unsigned int screenWidth, unsigned int screenHeight, HWND hwnd, float wScale, float hScale)
{
	bool result;


	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
    result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR, wScale, hScale);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	
	// Create the texture shader object.
	m_TextureShader = new TextureShaderClass;
	if(!m_TextureShader)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the bitmap object.
    m_Bitmap = new BitmapClass;
    if(!m_Bitmap)
    {
        return false;
    }

    // Initialize the bitmap object.
    result = m_Bitmap->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, m_D3D->GetDeviceContext(), screenWidth, screenHeight);
    if(!result)
    {
        MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
        return false;
    }

    m_freq = new FreqClass;
    if(!m_freq)
    {
        return false;
    }

    // Initialize the bitmap object.
    result = m_freq->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, m_D3D->GetDeviceContext(), screenWidth, screenHeight);
    if(!result)
    {
        MessageBox(hwnd, L"Could not initialize the freq object.", L"Error", MB_OK);
        return false;
    }

	return true;
}


void GraphicsClass::Shutdown()
{
	// Release the bitmap object.
	if(m_Bitmap)
	{
		m_Bitmap->Shutdown();
		delete m_Bitmap;
        m_Bitmap = nullptr;
	}

    if(m_freq)
    {
        m_freq->Shutdown();
        delete m_freq;
        m_freq = nullptr;
    }

	// Release the texture shader object.
	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
        m_TextureShader = nullptr;
	}

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
        m_Camera = nullptr;
	}

	// Release the D3D object.
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
        m_D3D = nullptr;
	}

	return;
}


bool GraphicsClass::Frame(void* data)
{
	bool result;
	static float rotation = 0.0f;


	// Update the rotation variable each frame.
    rotation += static_cast<float>(D3DX_PI * 0.005);
	if(rotation > 360.0f)
	{
		rotation -= 360.0f;
	}
	
	// Render the graphics scene.
    result = Render(rotation, data, nullptr, 0);
	if(!result)
	{
		return false;
	}

    return true;
}

bool GraphicsClass::Freq(float * data, unsigned int size)
{
    bool result;
    static float rotation = 0.0f;

    // Update the rotation variable each frame.
    rotation += static_cast<float>(D3DX_PI * 0.005);
    if(rotation > 360.0f)
    {
        rotation -= 360.0f;
    }

    // Render the graphics scene.
    result = Render(rotation, nullptr, data, size);
    if(!result)
    {
        return false;
    }

    return true;
}

void GraphicsClass::ResetViewport(float fScaleX, float fScaleY)
{
    m_D3D->ResetViewport(fScaleX, fScaleY);
}

bool GraphicsClass::Render(float /*rotation*/, void* data, float* freq, unsigned int freqSize)
{
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	bool result;

	// Clear the buffers to begin the scene.
    m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    if(!data && !freq)
    {
        m_D3D->EndScene();
        return true;
    }
	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, projection, and ortho matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);

	// Turn off the Z buffer to begin all 2D rendering.
	m_D3D->TurnZBufferOff();

    if(data)
    {
        // Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
        result = m_Bitmap->Render(m_D3D->GetDeviceContext(), 0, 0);
        if(!result)
        {
            return false;
        }

        // Render the bitmap with the texture shader.
        result = m_TextureShader->Render(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_Bitmap->texture(), data);
        if(!result)
        {
            return false;
        }
    }
    else if(freq)
    {
        result = m_freq->Render(m_D3D->GetDeviceContext(), 0, 0, freq, freqSize);
        if(!result)
        {
            return false;
        }

        // Render the bitmap with the texture shader.
        result = m_TextureShader->Render(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), m_freq->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_freq, freq);
        if(!result)
        {
            return false;
        }
    }
	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_D3D->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}
