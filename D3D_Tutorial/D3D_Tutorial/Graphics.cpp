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
	// 1. 创建设备和上下文
	// device 主要用于显示是否支持某些特性和分配资源
	// context 主要用于设置渲染状态、将资源绑定到 pipeline 和 发出渲染命令
	HRESULT hr = D3D11CreateDevice(
		NULL,							// 选择显卡, NULL 为默认显卡
		D3D_DRIVER_TYPE_HARDWARE,		// 驱动类型
		NULL,							// 驱动类型选择 软件时，需要设置
		flags,							// flag
		nullptr,						// 支持哪些功能
		0,								// feature 个数
		D3D11_SDK_VERSION,				// SDK 版本号
		&m_pDevice,						// 获取创建的设备
		NULL,							// 获取支持的功能
		&m_pContext);					// 上下文

	// 2. 设置 DXGI_SWAP_CHAIN_DESC 实例属性
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	// 缓存设置
	sd.BufferDesc.Width = 0;		// 缓存宽度，设为0，则 CreateSwapChain 时将该值设为输出窗口的宽度
	sd.BufferDesc.Height = 0;		// 缓存高度
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 输出存储格式
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // 扫描路线
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// 缩放
	// 采样设置
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// 缓存的用处
	sd.BufferCount = 1;										// 缓存个数
	sd.OutputWindow = hWnd;									// 渲染目标窗口句柄
	sd.Windowed = TRUE;										// 窗口显示
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;				
	sd.Flags = 0;

	// 3. 使用 IDXGIFactory 创建 IDXGISwapChain 实例
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

	// 4. 创建渲染目标视图
	ID3D11Texture2D* backBuffer = nullptr;
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	m_pDevice->CreateRenderTargetView(backBuffer, 0, &m_pRenderTargetView);
	backBuffer->Release();
}

void Graphics::ClearBuffer(float red, float green, float blue)
{
	const FLOAT color[4] = { red, green, blue, 1.0f };
	// 使用指定颜色清空渲染目标视图
	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
}

void Graphics::DrawTriangle()
{
	 //创建顶点缓存
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
	verticsDesc.ByteWidth = sizeof(vertices) * 3; // 字节数
	// -DSS TEST 将 usage 设为 D3D11_USAGE_IMMUTABLE 是否可行
	verticsDesc.Usage = D3D11_USAGE_DEFAULT; // 资源的使用，gpu和cpu 的读写权限 
	verticsDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 标识如何将资源绑定到 pipeline 
	verticsDesc.CPUAccessFlags = 0; // CPU 的读写权限
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
		1,				// buffer 数量  (start slot ~ start slot + buffer number
		&verticesBuffer,// 顶点缓存
		&strider,		// 每组数据的字节数
		&offset);		// 偏移量
	
	// 输入数据解释
	D3D11_INPUT_ELEMENT_DESC layout[] = {{
		"POSITION",									// shader 中的变量名
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,				// 顶点数据格式
		0,											// 代表顶点缓存数据通过哪个 slot 传给 GPU, input slot 数字，范围从0到15
		0,											// 偏移量，告诉 GPU 从哪个位置开始拿数据
		D3D11_INPUT_PER_VERTEX_DATA ,				// 输入槽的数据类型
		0
	}};

	// 图元拓扑结构
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// shader
	// shader 编译
	ID3DBlob* pBlob = NULL;
	D3DReadFileToBlob(L"HLSL/vs.cso", &pBlob);
	ID3D11VertexShader* pVertexShader = NULL;
	m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
	m_pContext->VSSetShader(pVertexShader, nullptr, 0);

	ID3D11InputLayout* inputLayout = NULL;
	m_pDevice->CreateInputLayout(layout, 1, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &inputLayout);
	m_pContext->IASetInputLayout(inputLayout);

	D3DReadFileToBlob(L"HLSL/ps.cso", &pBlob);
	ID3D11PixelShader* pPixelShader = NULL;
	m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
	m_pContext->PSSetShader(pPixelShader, nullptr, 0);

	// 设置渲染目标
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);
	
	// 设置视口
	D3D11_VIEWPORT viewPort = {};
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = 300;
	viewPort.Height = 200;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &viewPort);
						
	// 开始绘制
	m_pContext->Draw(3, 0);
}

void Graphics::EndDraw()
{
	m_pSwapChain->Present(1, 0);
}

