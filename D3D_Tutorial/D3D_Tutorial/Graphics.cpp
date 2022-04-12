#include "Graphics.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DCompiler.lib")

Graphics::Graphics(HWND hWnd)
	:m_pDevice(nullptr),
	m_pContext(nullptr),
	m_pSwapChain(nullptr),
	m_pRenderTargetView(nullptr)
{
	Initialize(hWnd);
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

void Graphics::DrawTriangle()
{
	/************************************* ����װ��׶� **************************************/	
	 //�������㻺��
	struct SimpleVertex
	{
		DirectX::XMFLOAT3 pos;
	};
	SimpleVertex vertices[] =
	{
		DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f),
		DirectX::XMFLOAT3(0.5f,-0.5f, 0.5f),
		DirectX::XMFLOAT3(-0.5f,-0.5, 0.5f)
	};

	D3D11_BUFFER_DESC verticsDesc = {};
	verticsDesc.ByteWidth = sizeof(vertices) * 3; // �ֽ���
	// �� usage ��Ϊ D3D11_USAGE_IMMUTABLE  D3D11_USAGE_DEFAULT ����
	verticsDesc.Usage = D3D11_USAGE_DEFAULT; // ��Դ��ʹ�ã�gpu��cpu �Ķ�дȨ�� 
	verticsDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ��ʶ��ν���Դ�󶨵� pipeline 
	verticsDesc.CPUAccessFlags = 0; // CPU �Ķ�дȨ��
	verticsDesc.MiscFlags = 0;
	verticsDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA resourceData = {};
	resourceData.pSysMem = vertices;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;
	ID3D11Buffer* verticesBuffer;
	m_pDevice->CreateBuffer(&verticsDesc, &resourceData, &verticesBuffer);


	UINT strider = sizeof(SimpleVertex);
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(
		0,				// start slot
		1,				// buffer ����  (start slot ~ start slot + buffer number
		&verticesBuffer,// ���㻺��
		&strider,		// ÿ�����ݵ��ֽ���
		&offset);		// ƫ����
	
	// �������ݽ���
	D3D11_INPUT_ELEMENT_DESC layout[] = {{
		"POSITION",									// shader �еı�����
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,				// �������ݸ�ʽ
		0,											// �����㻺������ͨ���ĸ� slot ���� GPU, input slot ���֣���Χ��0��15
		0,											// ƫ���������� GPU ���ĸ�λ�ÿ�ʼ������
		D3D11_INPUT_PER_VERTEX_DATA ,				// ����۵���������
		0
	}};

	/************************************* ������ɫ���׶� **************************************/
	ID3DBlob* pBlob = NULL;
	D3DReadFileToBlob(L"HLSL/vs.cso", &pBlob);
	ID3D11VertexShader* pVertexShader = NULL;
	m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
	m_pContext->VSSetShader(pVertexShader, nullptr, 0);

	ID3D11InputLayout* inputLayout = NULL;
	m_pDevice->CreateInputLayout(layout, 
		1, 
		pBlob->GetBufferPointer(), // �� shader �� layout �ж���� SemanticName
		pBlob->GetBufferSize(), 
		&inputLayout);
	m_pContext->IASetInputLayout(inputLayout);

	// ͼԪ���˽ṹ
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/************************************* ������ɫ���׶� **************************************/
	D3DReadFileToBlob(L"HLSL/ps.cso", &pBlob);
	ID3D11PixelShader* pPixelShader = NULL;
	m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
	m_pContext->PSSetShader(pPixelShader, nullptr, 0);

	
	/************************************* ����׶� **************************************/
	// ������ȾĿ��
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);
	
	// �����ӿ�
	D3D11_VIEWPORT viewPort = {};
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = 300;
	viewPort.Height = 200;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &viewPort);
						
	// ��ʼ����
	m_pContext->Draw(3, 0);
}

void Graphics::EndDraw()
{
	m_pSwapChain->Present(1, 0);
}

