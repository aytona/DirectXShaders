#include "graphicsclass.h"
#include "cameraclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
    m_Camera = 0;
    m_Model = 0;
    m_ColorShader = 0;
}

GraphicsClass::GraphicsClass(const GraphicsClass& other)
{

}

GraphicsClass::~GraphicsClass()
{

}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
	m_D3D = new D3DClass;
	if (!m_D3D)
		return false;

	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

    m_Camera = new CameraClass;
    if (!m_Camera)
        return false;

    m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

    m_Model = new ModelClass;
    if (!m_Model)
        return false;

    result = m_Model->Initialize(m_D3D->GetDevice());
    if (!result)
    {
        MessageBox(hwnd, L"Coult not initialize the model object", L"Error", MB_OK);
        return false;
    }

    m_ColorShader = new ColorShaderClass;
    if (!m_ColorShader)
        return false;

    result = m_ColorShader->Initialize(m_D3D->GetDevice(), hwnd);
    if (!result)
    {
        MessageBox(hwnd, L"Coult not initialize the color shader object", L"Error", MB_OK);
        return false;
    }

	return true;
}

void GraphicsClass::Shutdown()
{
    if (m_ColorShader)
    {
        m_ColorShader->Shutdown();
        delete m_ColorShader;
        m_ColorShader = 0;
    }
    if (m_Model)
    {
        m_Model->Shutdown();
        delete m_Model;
        m_Model = 0;
    }
    if (m_Camera)
    {
        delete m_Camera;
        m_Camera = 0;
    }
	if (m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}
	return;
}

bool GraphicsClass::Frame()
{
	bool result;
	result = Render();
	if (!result)
		return false;
	return true;
}

bool GraphicsClass::Render()
{
    D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix;
    bool result;

	m_D3D->BeginScene(0.0f, 0.5f, 0.5f, 1.0f);    // Clear buffers

    m_Camera->Render();

    m_Camera->GetViewMatrix(viewMatrix);
    m_D3D->GetWorldMatrix(worldMatrix);
    m_D3D->GetProjectionMatrix(projectionMatrix);

    m_Model->Render(m_D3D->GetDeviceContext());

    result = m_ColorShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, 12.0f);
    if (!result)
        return false;

	m_D3D->EndScene();                            // Present rendered scene
	return true;
}