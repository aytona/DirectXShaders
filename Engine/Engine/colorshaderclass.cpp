#include "colorshaderclass.h"

ColorShaderClass::ColorShaderClass()
{
    m_vertexShader = 0;
	m_hullShader = 0;
	m_domainShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
	m_tessellationBuffer = 0;
}

ColorShaderClass::ColorShaderClass(const ColorShaderClass& other)
{

}

ColorShaderClass::~ColorShaderClass()
{

}

bool ColorShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;

    result = InitializeShader(device, hwnd, L"../Engine/color.vs", L"../Engine/color.hs", L"../Engine/color.ds", L"../Engine/color.ps");
    if (!result)
        return false;

    return true;
}

void ColorShaderClass::Shutdown()
{
    ShutdownShader();
    return;
}

bool ColorShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, float tessellationAmount)
{
    bool result;

    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, tessellationAmount);
    if (!result)
        return false;

    RenderShader(deviceContext, indexCount);
    return true;
}

bool ColorShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* hullShaderBuffer;
	ID3D10Blob* domainShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC tessellationBufferDesc;

    errorMessage = 0;
    vertexShaderBuffer = 0;
	hullShaderBuffer = 0;
	domainShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // Compile vertex shader
    result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
    if (FAILED(result))
    {
        if (errorMessage)
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        else
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        return false;
    }

    // Compile hull shader
	result = D3DX11CompileFromFile(hsFilename, NULL, NULL, "ColorHullShader", "hs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &hullShaderBuffer, &errorMessage, NULL);
	if (FAILED(result)) {
		if (errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, hsFilename);
		}
		else {
			MessageBox(hwnd, hsFilename, L"Missing Shader File", MB_OK);
		}
		return false;
	}

    // Compile domain shader
	result = D3DX11CompileFromFile(dsFilename, NULL, NULL, "ColorDomainShader", "ds_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &domainShaderBuffer, &errorMessage, NULL);
	if (FAILED(result)) {
		if (errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, dsFilename);
		}
		else {
			MessageBox(hwnd, dsFilename, L"Missing Shader File", MB_OK);
		}
		return false;
	}

    // Compile pixel shader
    result = D3DX11CompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
    if (FAILED(result))
    {
        if (errorMessage)
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        else
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        return false;
    }

    // Creating all shaders from the buffer
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result))
        return false;

	result = device->CreateHullShader(hullShaderBuffer->GetBufferPointer(), hullShaderBuffer->GetBufferSize(), NULL, &m_hullShader);
	if (FAILED(result))
		return false;

	result = device->CreateDomainShader(domainShaderBuffer->GetBufferPointer(), domainShaderBuffer->GetBufferSize(), NULL, &m_domainShader);
	if (FAILED(result))
		return false;

    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result))
        return false;

    // Vertex input layout description
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "COLOR";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result))
        return false;

    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    hullShaderBuffer->Release();
    hullShaderBuffer = 0;

    domainShaderBuffer->Release();
    domainShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Dynamic matrix constant buffer description in domain shader
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result))
        return false;

    // Dynamic tessellation constant buffer description in hull shader
    tessellationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    tessellationBufferDesc.ByteWidth = sizeof(TessellationBufferType);
    tessellationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    tessellationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    tessellationBufferDesc.MiscFlags = 0;
    tessellationBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&tessellationBufferDesc, NULL, &m_tessellationBuffer);
    if (FAILED(result))
        return false;

    return true;
}

void ColorShaderClass::ShutdownShader()
{
    if (m_tessellationBuffer) {
        m_tessellationBuffer->Release();
        m_tessellationBuffer = 0;
    }

    if (m_matrixBuffer)
    {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }

    if (m_layout)
    {
        m_layout->Release();
        m_layout = 0;
    }

    if (m_pixelShader)
    {
        m_pixelShader->Release();
        m_pixelShader = 0;
    }

    if (m_domainShader) {
        m_domainShader->Release();
        m_domainShader = 0;
    }

    if (m_hullShader) {
        m_hullShader->Release();
        m_hullShader = 0;
    }

    if (m_vertexShader)
    {
        m_vertexShader->Release();
        m_vertexShader = 0;
    }
    return;
}

void ColorShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long bufferSize, i;
    ofstream fout;

    compileErrors = (char*)(errorMessage->GetBufferPointer());
    bufferSize = errorMessage->GetBufferSize();
    fout.open("shader-error.txt");

    for (i = 0; i < bufferSize; i++)
        fout << compileErrors[i];

    fout.close();

    errorMessage->Release();
    errorMessage = 0;

    MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);
    return;
}

bool ColorShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, float tessellationAmount)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;
    TessellationBufferType* dataPtr2;

    D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
    D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
    D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
        return false;

    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    deviceContext->Unmap(m_matrixBuffer, 0);

    bufferNumber = 0;
    deviceContext->DSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    result = deviceContext->Map(m_tessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
        return false;

    dataPtr2 = (TessellationBufferType*)mappedResource.pData;
    dataPtr2->tessellationAmount = tessellationAmount;
    dataPtr2->padding = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    deviceContext->Unmap(m_tessellationBuffer, 0);

    // Set position of the tessellation constant buffer in the hull shader
    bufferNumber = 0;

    // Set tessellation constant buffer in hull shader with updated values
    deviceContext->HSSetConstantBuffers(bufferNumber, 1, &m_tessellationBuffer);

    return true;
}

void ColorShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // Set vertex input layout
    deviceContext->IASetInputLayout(m_layout);

    // Set shaders that will be used in render
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->HSSetShader(m_hullShader, NULL, 0);
    deviceContext->DSSetShader(m_domainShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // Render
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}