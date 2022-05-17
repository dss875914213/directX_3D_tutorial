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
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_backBuffer));
	m_pDevice->CreateRenderTargetView(m_backBuffer, 0, &m_pRenderTargetView);

	D3D11_TEXTURE2D_DESC desc;
	m_backBuffer->GetDesc(&desc);
	
	// 5. ��������
	desc.Width = m_screenSize.x;
	desc.Height = m_screenSize.y;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;

	/*
		D3D11_USAGE_DEFAULT						GPU ���Զ���д�����Կ�����Ϊ��ȾĿ���������ɫ������������Ϊ cpu ��������д������(��ʼ����ʱ�����Ҳ����д������)��
												��������Ҫ����Ϊ pipeline ����ȾĿ������֮����Ϊ��һ�� pipeline ����������
		D3D11_USAGE_IMMUTABLE					�����ɱ䣬���� c++ �е� const, ֻ���ڳ�ʼ����ʱ��ֵ
		D3D11_USAGE_DYNAMIC						CPU д�룬GPU ��ȡ��cpu ͨ�� map д������
		D3D11_USAGE_STAGING						ֻ��ͨ�� CopySubresourceRegion �� CopyResource ������һ���������� STAGING ����Ȼ��ͨ�� map ��ȡ���������
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
	/************************************* ����װ��׶� **************************************/
	 //�������㻺��
	SetVertexBuffer();

	DirectX::XMUINT3 index[] = {
		{0, 1, 2},
		{0, 2, 3}
	};

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.ByteWidth = sizeof(index) * 2; // �ֽ���
	// �� usage ��Ϊ D3D11_USAGE_IMMUTABLE  D3D11_USAGE_DEFAULT ����
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE; // ��Դ��ʹ�ã�gpu��cpu �Ķ�дȨ�� 
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // ��ʶ��ν���Դ�󶨵� pipeline 
	indexDesc.CPUAccessFlags = 0; // CPU �Ķ�дȨ��
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
	bufferDesc.ByteWidth = 16;	// ������ 16 �ı�������Ȼ���ܴ�����Դ
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;	//  GPU (read only) and the CPU (write only)
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_constBuffer);

	m_pContext->PSSetConstantBuffers(0, 1, &m_constBuffer);

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
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView2, NULL);

	ID3D11BlendState* blendState;
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget->BlendEnable = m_transParent; // �Ƿ������
	blendDesc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA; // ��Դͼ�� alpha ��Ϊ src rgb �Ļ������
	blendDesc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // ��Դͼ�� 1-alpha ��Ϊ dst rgb �Ļ������
	blendDesc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD; // ������Ӳ���
	blendDesc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE; // 
	blendDesc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // ����д���λ��
	m_pDevice->CreateBlendState(&blendDesc, &blendState);

	const FLOAT BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
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
}

void Graphics::ClearBuffer(float red, float green, float blue)
{
	const FLOAT color[4] = { red, green, blue, 1.0f };
	// ʹ��ָ����ɫ�����ȾĿ����ͼ
	m_pContext->ClearRenderTargetView(m_pRenderTargetView2, color);
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
	SetVertexBuffer();
	D3D11_MAPPED_SUBRESOURCE ms;
	m_pContext->Map(m_constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		memcpy_s(ms.pData, sizeof(m_threshold), &m_threshold, sizeof(m_threshold));
	}
	m_pContext->Unmap(m_constBuffer, 0);
		
	// ��ʼ����
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

	// ���� directX ����������ϵ����Щ�����Ϳ����治һ��
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

