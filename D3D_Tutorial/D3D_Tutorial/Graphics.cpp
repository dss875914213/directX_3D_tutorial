#include "Graphics.h"

#pragma comment(lib, "D3D11.lib")

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

void Graphics::EndDraw()
{
	m_pSwapChain->Present(1, 0);
}

