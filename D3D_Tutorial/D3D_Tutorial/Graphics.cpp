#include "Graphics.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "WICTextureLoader11.h"
#include <string>
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DCompiler.lib")

using namespace DirectX;

void __cdecl odprintf(const char* format, ...)
{
	char buf[4096], * p = buf;
	va_list args;

	va_start(args, format);
	p += _vsnprintf(p, sizeof buf - 1, format, args);
	va_end(args);

	if (p > buf && isspace(p[-1]))
	{
		*--p = '\0';
		*p++ = '\r';
		*p++ = '\n';
		*p = '\0';
	}
	OutputDebugString(buf);
}

// �������ݽ���
D3D11_INPUT_ELEMENT_DESC layout[] = { 
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA , 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

Graphics::Graphics(HWND hWnd)
	:m_pDevice(nullptr),
	m_pContext(nullptr),
	m_pSwapChain(nullptr),
	m_pRenderTargetView(nullptr),
	m_threshold(0.5)
{
	Initialize(hWnd);
}

Graphics::~Graphics()
{

}

void Graphics::Initialize(HWND hWnd)
{
	UINT flags = 0;
#ifndef NDEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// 1. �����豸��������
	// device ��Ҫ������ʾ�Ƿ�֧��ĳЩ���Ժͷ�����Դ
	// context ��Ҫ����������Ⱦ״̬������Դ�󶨵� pipeline �� ������Ⱦ����
	HRESULT hr = D3D11CreateDevice(
		NULL,							// ѡ���Կ�, NULL ΪĬ���Կ�
		D3D_DRIVER_TYPE_HARDWARE,		// ��������
		NULL,							// ��������ѡ�� ���ʱ����Ҫ����
		flags,							// flag
		nullptr,						// ֧����Щ����
		0,								// feature ����
		D3D11_SDK_VERSION,				// SDK �汾��
		&m_pDevice,						// ��ȡ�������豸
		NULL,							// ��ȡ֧�ֵĹ���
		&m_pContext);					// ������

	// 2. ���� DXGI_SWAP_CHAIN_DESC ʵ������
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	// ��������
	sd.BufferDesc.Width = 0;		// �����ȣ���Ϊ0���� CreateSwapChain ʱ����ֵ��Ϊ������ڵĿ��
	sd.BufferDesc.Height = 0;		// ����߶�
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ����洢��ʽ
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // ɨ��·��
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// ����
	// ��������
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// ������ô�
	sd.BufferCount = 1;										// �������
	sd.OutputWindow = hWnd;									// ��ȾĿ�괰�ھ��
	sd.Windowed = TRUE;										// ������ʾ
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;				
	sd.Flags = 0;

	// 3. ʹ�� IDXGIFactory ���� IDXGISwapChain ʵ��
	IDXGIDevice* dxgiDevice = nullptr;
	IDXGIAdapter* dxgiAdapter = nullptr;
	IDXGIFactory* dxgiFactory = nullptr;
	hr = m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	if(m_pDevice!= nullptr)
		dxgiFactory->CreateSwapChain(m_pDevice, &sd, &m_pSwapChain);
	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	// 4. ������ȾĿ����ͼ
	ID3D11Texture2D* backBuffer = nullptr;
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	m_pDevice->CreateRenderTargetView(backBuffer, 0, &m_pRenderTargetView);
	backBuffer->Release();
}

void Graphics::ClearBuffer(float red, float green, float blue)
{
	const FLOAT color[4] = { red, green, blue, 1.0f };
	// ʹ��ָ����ɫ�����ȾĿ����ͼ
	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
}

void Graphics::InitEffect()
{
	/************************************* ������ɫ���׶� **************************************/
	ID3DBlob* pBlob = NULL;
	D3DReadFileToBlob(L"HLSL/vs.cso", &pBlob);
	m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pVertexShader);

	m_pDevice->CreateInputLayout(layout,
		2,
		pBlob->GetBufferPointer(), // �� shader �� layout �ж���� SemanticName
		pBlob->GetBufferSize(),
		&m_inputLayout);

	/************************************* ������ɫ���׶� **************************************/
	D3DReadFileToBlob(L"HLSL/ps.cso", &pBlob);
	m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader);
}

void Graphics::DrawPicture()
{
	InitEffect();
	/************************************* ����װ��׶� **************************************/	
	 //�������㻺��
	struct SimpleVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};
	SimpleVertex vertices[] =
	{
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f)},
		{DirectX::XMFLOAT3(-0.5f,  0.5f, 0.5f),	XMFLOAT2(0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f,  0.5f, 0.5f),	XMFLOAT2(1.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f),	XMFLOAT2(1.0f, 1.0f)},
	};

	D3D11_BUFFER_DESC verticsDesc = {};
	verticsDesc.ByteWidth = sizeof(vertices) * 5; // �ֽ���
	// �� usage ��Ϊ D3D11_USAGE_IMMUTABLE  D3D11_USAGE_DEFAULT ����
	verticsDesc.Usage = D3D11_USAGE_IMMUTABLE; // ��Դ��ʹ�ã�gpu��cpu �Ķ�дȨ�� 
	verticsDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ��ʶ��ν���Դ�󶨵� pipeline 
	verticsDesc.CPUAccessFlags = 0; // CPU �Ķ�дȨ��
	verticsDesc.MiscFlags = 0;
	verticsDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA resourceData = {};
	resourceData.pSysMem = vertices;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;
	ID3D11Buffer* verticesBuffer = NULL;
	m_pDevice->CreateBuffer(&verticsDesc, &resourceData, &verticesBuffer);


	UINT strider = sizeof(SimpleVertex);
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(
		0,				// start slot
		1,				// buffer ����  (start slot ~ start slot + buffer number
		&verticesBuffer,// ���㻺��
		&strider,		// ÿ�����ݵ��ֽ���
		&offset);		// ƫ����


	DirectX::XMUINT3 index[] = {
		{0, 1, 2},
		{0, 2, 3}};

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.ByteWidth = sizeof(index) * 2; // �ֽ���
	// �� usage ��Ϊ D3D11_USAGE_IMMUTABLE  D3D11_USAGE_DEFAULT ����
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE; // ��Դ��ʹ�ã�gpu��cpu �Ķ�дȨ�� 
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // ��ʶ��ν���Դ�󶨵� pipeline 
	indexDesc.CPUAccessFlags = 0; // CPU �Ķ�дȨ��
	indexDesc.MiscFlags = 0;
	indexDesc.StructureByteStride = 0;

	resourceData.pSysMem = index;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	ID3D11Buffer* indexBuffer = NULL;
	m_pDevice->CreateBuffer(&indexDesc, &resourceData, &indexBuffer);

	// ������������
	m_pContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	// ���벼��
	m_pContext->IASetInputLayout(m_inputLayout);

	// ͼԪ���˽ṹ
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/************************************* ������ɫ���׶� **************************************/
	m_pContext->VSSetShader(m_pVertexShader, nullptr, 0);

	/************************************* ������ɫ���׶� **************************************/
	m_pContext->PSSetShader(m_pPixelShader, nullptr, 0);

	ID3D11Resource* inputResource = NULL;
	ID3D11ShaderResourceView* shaderResourceView = NULL;
	std::wstring path = L"res\\image\\tiger.png";
	CreateWICTextureFromFile(m_pDevice,
		path.c_str(),
		&inputResource,
		&shaderResourceView);

	m_pContext->PSSetShaderResources(0, 1, &shaderResourceView);

	ID3D11SamplerState* sampler;
	D3D11_SAMPLER_DESC sampleDesc = {};
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // ƽ��������
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampleDesc.MinLOD = 0;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_pDevice->CreateSamplerState(&sampleDesc, &sampler);

	m_pContext->PSSetSamplers(0, 1, &sampler);

	
	/************************************* ����׶� **************************************/
	// ������ȾĿ��
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	ID3D11BlendState* blendState;
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget->BlendEnable = TRUE; // �Ƿ������
	blendDesc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA; // ��Դͼ�� alpha ��Ϊ src rgb �Ļ������
	blendDesc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // ��Դͼ�� 1-alpha ��Ϊ dst rgb �Ļ������
	blendDesc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD; // ������Ӳ���
	blendDesc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE; // 
	blendDesc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // ����д���λ��
	m_pDevice->CreateBlendState(&blendDesc, &blendState);

	const FLOAT BlendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_pContext->OMSetBlendState(blendState, BlendFactor, 0xffffffff);
	
	// �����ӿ�
	D3D11_VIEWPORT viewPort = {};
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = 800;
	viewPort.Height = 600;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &viewPort);
				
	// ��ʼ����
	m_pContext->DrawIndexed(6, 0, 0);
}

void Graphics::EndDraw()
{
	m_pSwapChain->Present(1, 0);
}

void Graphics::Message(int msg)
{
	switch (msg)
	{
	case VK_UP:
		m_threshold += 0.1;
		break;
	case VK_DOWN:
		m_threshold -= 0.1;
		break;
	default:
		break;
	}
	m_threshold = (std::max)((std::min)(1.0f, m_threshold), 0.0f);
	odprintf("aaaa %f ", m_threshold);
}

