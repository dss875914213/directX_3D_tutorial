#include "Graphics.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "WICTextureLoader11.h"
#include <wincodec.h>
#include "ScreenGrab11.h"
#include <string>
#include "timer.h"
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
namespace DXSpace
{

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

}

void Graphics::Initialize(HWND hWnd)
{
	RECT rect;
	::GetClientRect(hWnd, &rect);

	m_screenSize.x = rect.right - rect.left;
	m_screenSize.y = rect.bottom - rect.top;

	UINT flags = 0;
//#ifndef NDEBUG
//	flags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
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

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = 1920;
	desc.Height = 1080;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_NV12;
	desc.SampleDesc.Count = 1;
	//desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	desc.Usage = D3D11_USAGE_DEFAULT;
	//desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	m_pDevice->CreateTexture2D(&desc, NULL, &m_pDefaultTexture);

	D3D11_TEXTURE2D_DESC desc1;
	ZeroMemory(&desc1, sizeof(desc1));
	desc1.MipLevels = 1;
	desc1.ArraySize = 1;
	desc1.SampleDesc.Count = 1;
	desc1.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc1.Usage = D3D11_USAGE_STAGING;
	desc1.Format = DXGI_FORMAT_NV12;
	desc1.Width = 1920;
	desc1.Height = 1080;

	HRESULT hr1 = m_pDevice->CreateTexture2D(&desc1, NULL, &m_pStagingTexture);
	hr1 = m_pDevice->CreateTexture2D(&desc1, NULL, &m_pStagingTexture2);
	hr1 = m_pDevice->CreateTexture2D(&desc1, NULL, &m_pStagingTexture3);
	hr1 = m_pDevice->CreateTexture2D(&desc1, NULL, &m_pStagingTexture4);

}

static int add = 0;

void Graphics::DrawPicture()
{
	add++;
	if (add >= 4)
		add = 0;
	long lBeginTime = MyGetTickCount();
	D3D11_MAPPED_SUBRESOURCE ms;

	//if (add == 0)
	{
		m_pContext->Map(m_pStagingTexture.Get(), 0, D3D11_MAP_READ, 0, &ms);
		m_pContext->Unmap(m_pStagingTexture.Get(), 0);
		m_pContext->CopyResource(m_pStagingTexture4.Get(), m_pDefaultTexture.Get());
		m_pContext->Flush();
	}
	//else if(add == 1)
	//{
	//	m_pContext->Map(m_pStagingTexture2.Get(), 0, D3D11_MAP_READ, 0, &ms);
	//	m_pContext->Unmap(m_pStagingTexture2.Get(), 0);
	//	m_pContext->CopyResource(m_pStagingTexture.Get(), m_pDefaultTexture.Get());
	//	m_pContext->Flush();
	//}
	//else if (add == 2)
	//{
	//	m_pContext->Map(m_pStagingTexture3.Get(), 0, D3D11_MAP_READ, 0, &ms);
	//	m_pContext->Unmap(m_pStagingTexture3.Get(), 0);
	//	m_pContext->CopyResource(m_pStagingTexture2.Get(), m_pDefaultTexture.Get());
	//	m_pContext->Flush();
	//}
	//else if (add == 3)
	//{
	//	m_pContext->Map(m_pStagingTexture4.Get(), 0, D3D11_MAP_READ, 0, &ms);
	//	m_pContext->Unmap(m_pStagingTexture4.Get(), 0);
	//	m_pContext->CopyResource(m_pStagingTexture3.Get(), m_pDefaultTexture.Get());
	//	m_pContext->Flush();
	//}


	
	long lTime = GetTickCountDIFF(lBeginTime);
	odprintf("lTime %ld ", lTime);
	//Sleep(200);
}

void Graphics::Create()
{
}

void Graphics::ClearBuffer(float red, float green, float blue)
{
	const FLOAT color[4] = { red, green, blue, 1.0f };
	// 使用指定颜色清空渲染目标视图
	//m_pContext->ClearRenderTargetView(m_pRenderTargetView2, color);
}

void Graphics::InitEffect()
{
	/************************************* 顶点着色器阶段 **************************************/
	ComPtr<ID3DBlob> pBlob = NULL;
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

void Graphics::EndDraw()
{
	//m_pSwapChain->Present(1, 0);
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
}

