#include "graphicsclass.h"
#include "cameraclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
    m_Camera = 0;
    m_Model = 0;
    m_ColorShader = 0;

    m_speed = -0.5f;
    m_maxDistance = -50.0f;
    m_minDistance = -5.0f;

    m_tessellationAmount;
    m_maxTessellation = 64.0f;
    m_currentZPos;

    m_movingBack = true;
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

    m_currentZPos = m_Camera->GetPosition().z;

    if (m_currentZPos >= m_minDistance && !m_movingBack) {
        m_movingBack = true;
        m_speed = -m_speed;
    } else if (m_currentZPos <= m_maxDistance && m_movingBack) {
        m_movingBack = false;
        m_speed = -m_speed;
    }

    m_Camera->SetPosition(0.0f, 0.0f, m_currentZPos + m_speed);

    m_tessellationAmount = m_maxTessellation - ((m_currentZPos * m_maxTessellation) / m_maxDistance);

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

    result = m_ColorShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_tessellationAmount);
    if (!result)
        return false;

	m_D3D->EndScene();                            // Present rendered scene
	return true;
}