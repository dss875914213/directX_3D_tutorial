#include "Graphics.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "WICTextureLoader11.h"
#include <wincodec.h>
#include "ScreenGrab11.h"
#include <string>
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DCompiler.lib")

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 pos;
	XMFLOAT2 tex;
};

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
	m_threshold(0.0),
	m_transParent(TRUE),
	m_backBuffer(NULL)
{
	ZeroMemory(&m_transformation, sizeof(m_transformation));
	m_transformation.scale = { 1.0f, 1.0f };

	Initialize(hWnd);
}

Graphics::~Graphics()
{
	if(m_backBuffer)
		m_backBuffer->Release();
}

void Graphics::Initialize(HWND hWnd)
{
	RECT rect;
	::GetClientRect(hWnd, &rect);

	m_screenSize.x = rect.right - rect.left;
	m_screenSize.y = rect.bottom - rect.top;

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
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_backBuffer));
	m_pDevice->CreateRenderTargetView(m_backBuffer, 0, &m_pRenderTargetView);

	D3D11_TEXTURE2D_DESC desc;
	m_backBuffer->GetDesc(&desc);
	
	// 5. 创建纹理
	desc.Width = m_screenSize.x;
	desc.Height = m_screenSize.y;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;

	/*
		D3D11_USAGE_DEFAULT						GPU 可以读和写，所以可以作为渲染目标纹理和着色器输入纹理；因为 cpu 不能向其写入数据(初始化的时候可能也可以写入数据)，
												所以他需要先作为 pipeline 的渲染目标纹理，之后作为下一个 pipeline 的输入纹理
		D3D11_USAGE_IMMUTABLE					纹理不可变，类似 c++ 中的 const, 只能在初始化的时候赋值
		D3D11_USAGE_DYNAMIC						CPU 写入，GPU 读取。cpu 通过 map 写入数据
		D3D11_USAGE_STAGING						只能通过 CopySubresourceRegion 和 CopyResource 将另外一块纹理拷贝到 STAGING 纹理，然后通过 map 读取里面的数据
	*/
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags = 0;

	m_pDevice->CreateTexture2D(&desc, NULL, &m_pTexture);

	D3D11_RENDER_TARGET_VIEW_DESC targetViewDesc;

	m_pDevice->CreateRenderTargetView(m_pTexture, 0, &m_pRenderTargetView2);

}

void Graphics::Create()
{
	InitEffect();
	/************************************* 输入装配阶段 **************************************/
	 //创建顶点缓存
	SetVertexBuffer();

	DirectX::XMUINT3 index[] = {
		{0, 1, 2},
		{0, 2, 3}
	};

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.ByteWidth = sizeof(index) * 2; // 字节数
	// 将 usage 设为 D3D11_USAGE_IMMUTABLE  D3D11_USAGE_DEFAULT 可行
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE; // 资源的使用，gpu和cpu 的读写权限 
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // 标识如何将资源绑定到 pipeline 
	indexDesc.CPUAccessFlags = 0; // CPU 的读写权限
	indexDesc.MiscFlags = 0;
	indexDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA resourceData = {};
	resourceData.pSysMem = index;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	ID3D11Buffer* indexBuffer = NULL;
	m_pDevice->CreateBuffer(&indexDesc, &resourceData, &indexBuffer);

	// constant
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = 16;	// 必须是 16 的倍数，不然不能创建资源
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;	//  GPU (read only) and the CPU (write only)
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_constBuffer);

	m_pContext->PSSetConstantBuffers(0, 1, &m_constBuffer);

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
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView2, NULL);

	ID3D11BlendState* blendState;
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget->BlendEnable = m_transParent; // 是否开启混合
	blendDesc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA; // 将源图的 alpha 作为 src rgb 的混合因子
	blendDesc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 将源图的 1-alpha 作为 dst rgb 的混合因子
	blendDesc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD; // 进行相加操作
	blendDesc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE; // 
	blendDesc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // 可以写入的位置
	m_pDevice->CreateBlendState(&blendDesc, &blendState);

	const FLOAT BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
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
}

void Graphics::ClearBuffer(float red, float green, float blue)
{
	const FLOAT color[4] = { red, green, blue, 1.0f };
	// 使用指定颜色清空渲染目标视图
	m_pContext->ClearRenderTargetView(m_pRenderTargetView2, color);
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
	SetVertexBuffer();
	D3D11_MAPPED_SUBRESOURCE ms;
	m_pContext->Map(m_constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		memcpy_s(ms.pData, sizeof(m_threshold), &m_threshold, sizeof(m_threshold));
	}
	m_pContext->Unmap(m_constBuffer, 0);
		
	// 开始绘制
	m_pContext->DrawIndexed(6, 0, 0);

	SaveWICTextureToFile(m_pContext, m_pTexture, GUID_ContainerFormatPng, L"D://test.png");
	//SaveWICTextureToFile(m_pContext, m_backBuffer, GUID_ContainerFormatPng, L"D://test.png");
}

void Graphics::EndDraw()
{
	m_pSwapChain->Present(1, 0);
}

void Graphics::Message(int msg)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	odprintf("msg: %d ", msg);
	switch (msg)
	{
	case VK_UP:
		m_threshold += 0.1;
		break;
	case VK_DOWN:
		m_threshold -= 0.1;
		break;
	case 'H':
		m_transformation.flipH = !m_transformation.flipH;
		break;
	case 'V':
		m_transformation.flipV = !m_transformation.flipV;
		break;
	case 'R':
		m_transformation.angle -= 10;
		break;
	case 'L':
		m_transformation.angle += 10;
		break;
	case 'W':
		m_transformation.pos.y += 1;
		break;
	case 'S':
		m_transformation.pos.y -= 1;
		break;
	case 'A':
		m_transformation.pos.x -= 1;
		break;
	case 'D':
		m_transformation.pos.x += 1;
		break;
	case 'U':
		m_transformation.scale.x += 0.1;
		break;
	case 'I':
		m_transformation.scale.x -= 0.1;
		break;
	case 'O':
		m_transformation.scale.y += 0.1;
		break;
	case 'P':
		m_transformation.scale.y -= 0.1;
		break;
	default:
		break;
	}
	m_threshold = (std::max)((std::min)(1.0f, m_threshold), 0.0f);
	odprintf("m_threshold %f ", m_threshold);
}



void Graphics::SetVertexBuffer()
{
	XMFLOAT4X4 pos = SetMVP();

	SimpleVertex vertices[] =
	{
		{XMFLOAT3(pos._11 / pos._14, pos._12 / pos._14, pos._13 / pos._14), XMFLOAT2(m_transformation.flipH ? 1.0 : 0.3f, m_transformation.flipV ? 0.0f : 0.7f)},
		{XMFLOAT3(pos._21 / pos._24, pos._22 / pos._24, pos._23 / pos._24),	XMFLOAT2(m_transformation.flipH ? 1.0 : 0.3f, m_transformation.flipV ? 1.0f : 0.3f)},
		{XMFLOAT3(pos._31 / pos._34, pos._32 / pos._34, pos._33 / pos._34),	XMFLOAT2(m_transformation.flipH ? 0.0 : 0.7f, m_transformation.flipV ? 1.0f : 0.3f)},
		{XMFLOAT3(pos._41 / pos._44, pos._42 / pos._44, pos._43 / pos._44),	XMFLOAT2(m_transformation.flipH ? 0.0 : 0.7f, m_transformation.flipV ? 0.0f : 0.7f)},
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
}

DirectX::XMFLOAT4X4 Graphics::SetMVP()
{
	//XMMATRIX rotate = DirectX::XMMatrixRotationX(m_transformation.angle * D3DX_PI / 180);
	//XMMATRIX rotate = DirectX::XMMatrixRotationY(m_transformation.angle * D3DX_PI / 180);
	XMMATRIX rotate = DirectX::XMMatrixRotationZ(m_transformation.angle * D3DX_PI / 180);

	XMMATRIX scale = DirectX::XMMatrixScaling(m_transformation.scale.x * 265.0, m_transformation.scale.y*235, 1.0f);
	XMMATRIX translate = DirectX::XMMatrixTranslation(m_transformation.pos.x, m_transformation.pos.y, 0.0f);
	//XMMATRIX shear = DirectX::XMMatrix;
	m_model = scale * rotate* translate;

	// 由于 directX 是左手坐标系，有些东西和课里面不一样
	m_view = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f), 
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	FLOAT fov = 90 * D3DX_PI / 180;
	FLOAT aspectRatio = 800.0 / 600;
	odprintf("m_transformation.scale.x %f ", m_transformation.scale.x);
	//FLOAT aspectRatio = 1.0;
	//m_projection = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, 1.0f, 100.0f);
	m_projection = DirectX::XMMatrixOrthographicLH(800.0, 600.0, 1.0f, 1000.0f);
	XMMATRIX matrix = m_projection * m_view * m_model;

	XMMATRIX pos = DirectX::XMMatrixSet(
		-1.0f, -1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 1.0f);

	pos = pos * m_model;
	pos = pos * m_view;
	pos = pos * m_projection;
	XMFLOAT4X4 pDestination;
	DirectX::XMStoreFloat4x4(&pDestination, pos);
	return pDestination;
}

