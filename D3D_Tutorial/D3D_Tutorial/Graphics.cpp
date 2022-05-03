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

// 输入数据解释
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

void Graphics::InitEffect()
{
	/************************************* 顶点着色器阶段 **************************************/
	ID3DBlob* pBlob = NULL;
	D3DReadFileToBlob(L"HLSL/vs.cso", &pBlob);
	m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pVertexShader);

	m_pDevice->CreateInputLayout(layout,
		2,
		pBlob->GetBufferPointer(), // 该 shader 有 layout 中定义的 SemanticName
		pBlob->GetBufferSize(),
		&m_inputLayout);

	/************************************* 像素着色器阶段 **************************************/
	D3DReadFileToBlob(L"HLSL/ps.cso", &pBlob);
	m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader);
}

void Graphics::DrawPicture()
{
	InitEffect();
	/************************************* 输入装配阶段 **************************************/	
	 //创建顶点缓存
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
	verticsDesc.ByteWidth = sizeof(vertices) * 5; // 字节数
	// 将 usage 设为 D3D11_USAGE_IMMUTABLE  D3D11_USAGE_DEFAULT 可行
	verticsDesc.Usage = D3D11_USAGE_IMMUTABLE; // 资源的使用，gpu和cpu 的读写权限 
	verticsDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 标识如何将资源绑定到 pipeline 
	verticsDesc.CPUAccessFlags = 0; // CPU 的读写权限
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
		1,				// buffer 数量  (start slot ~ start slot + buffer number
		&verticesBuffer,// 顶点缓存
		&strider,		// 每组数据的字节数
		&offset);		// 偏移量


	DirectX::XMUINT3 index[] = {
		{0, 1, 2},
		{0, 2, 3}};

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.ByteWidth = sizeof(index) * 2; // 字节数
	// 将 usage 设为 D3D11_USAGE_IMMUTABLE  D3D11_USAGE_DEFAULT 可行
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE; // 资源的使用，gpu和cpu 的读写权限 
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // 标识如何将资源绑定到 pipeline 
	indexDesc.CPUAccessFlags = 0; // CPU 的读写权限
	indexDesc.MiscFlags = 0;
	indexDesc.StructureByteStride = 0;

	resourceData.pSysMem = index;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	ID3D11Buffer* indexBuffer = NULL;
	m_pDevice->CreateBuffer(&indexDesc, &resourceData, &indexBuffer);

	// 设置所引缓存
	m_pContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	// 输入布局
	m_pContext->IASetInputLayout(m_inputLayout);

	// 图元拓扑结构
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/************************************* 顶点着色器阶段 **************************************/
	m_pContext->VSSetShader(m_pVertexShader, nullptr, 0);

	/************************************* 像素着色器阶段 **************************************/
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
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 平铺整数个
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampleDesc.MinLOD = 0;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_pDevice->CreateSamplerState(&sampleDesc, &sampler);

	m_pContext->PSSetSamplers(0, 1, &sampler);

	
	/************************************* 输出阶段 **************************************/
	// 设置渲染目标
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	ID3D11BlendState* blendState;
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget->BlendEnable = TRUE; // 是否开启混合
	blendDesc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA; // 将源图的 alpha 作为 src rgb 的混合因子
	blendDesc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 将源图的 1-alpha 作为 dst rgb 的混合因子
	blendDesc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD; // 进行相加操作
	blendDesc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE; // 
	blendDesc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // 可以写入的位置
	m_pDevice->CreateBlendState(&blendDesc, &blendState);

	const FLOAT BlendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_pContext->OMSetBlendState(blendState, BlendFactor, 0xffffffff);
	
	// 设置视口
	D3D11_VIEWPORT viewPort = {};
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = 800;
	viewPort.Height = 600;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &viewPort);
				
	// 开始绘制
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

