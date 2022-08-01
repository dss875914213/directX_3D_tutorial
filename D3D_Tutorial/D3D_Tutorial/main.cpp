//#include <windows.h>
//#include "window.h"
//#include "Graphics.h"
//#include <thread>
//#define  PI 3.1415926

#include <windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#pragma comment(lib, "D3D11.lib")
#include <stdio.h>

//Window window;
//
//void Render()
//{
//	Graphics* m_g = new Graphics(NULL);
//	while (1)
//	{
//		m_g->DrawPicture();
//		Sleep(10);
//	}
//}

void __cdecl odprintf1(const char* format, ...)
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


long MyGetTickCount1()
{
	static BOOL init = FALSE;
	static BOOL hires = FALSE;
	static _int64 freq = 1;
	if (!init)
	{
		hires = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		if (!hires)
			freq = 1000;
		init = TRUE;
	}
	_int64 now;
	if (hires)
		QueryPerformanceCounter((LARGE_INTEGER*)&now);
	else
		now = GetTickCount();
	return (long)(1000.0f * (double)now / (double)freq);
}

long GetTickCountDIFF1(long lBegin, long lNow)
{
	if (0 == lBegin)
		return 0;
	if (0 == lNow)
		lNow = MyGetTickCount1();
	long lDiff = lNow - lBegin;
	if (lDiff < 0)
		lDiff = 0xFFFFFFFF - lBegin + lNow + 1;
	return lDiff;
}


// 回调函数  处理获取的消息
int WINAPI WinMain(
	HINSTANCE hInstance,		// 应用程序句柄
	HINSTANCE hPrevInstance,	// 上一个应用程序的句柄，目前已弃用 一直为 NULL
	LPSTR     lpCmdLine,		// 传入的命令行
	int       nShowCmd			// 设置窗口的显示方式
)
{
	ID3D11Texture2D* m_pDefaultTexture;
	ID3D11Texture2D* m_pImmutableTexture;
	ID3D11Texture2D* m_pDynamicTexture;
	ID3D11Texture2D* m_pStagingTexture;
	ID3D11Texture2D* m_pStagingTexture2;
	ID3D11Texture2D* m_pStagingTexture3;
	ID3D11Texture2D* m_pStagingTexture4;

	ID3D11Device*  m_pDevice;
	ID3D11DeviceContext*  m_pContext;

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


	static int add = 0;
	HRESULT hr1 = m_pDevice->CreateTexture2D(&desc1, NULL, &m_pStagingTexture);
	hr1 = m_pDevice->CreateTexture2D(&desc1, NULL, &m_pStagingTexture2);
	hr1 = m_pDevice->CreateTexture2D(&desc1, NULL, &m_pStagingTexture3);
	hr1 = m_pDevice->CreateTexture2D(&desc1, NULL, &m_pStagingTexture4);

	while (1)
	{
		add++;
		if (add >= 2)
			add = 0;
		long lBeginTime = MyGetTickCount1();
		D3D11_MAPPED_SUBRESOURCE ms;

		if (add == 0)
		{
			m_pContext->Map(m_pStagingTexture, 0, D3D11_MAP_READ, 0, &ms);
			m_pContext->Unmap(m_pStagingTexture, 0);
			m_pContext->CopyResource(m_pStagingTexture2, m_pDefaultTexture);
			m_pContext->Flush();
		}
		else if (add == 1)
		{
			m_pContext->Map(m_pStagingTexture2, 0, D3D11_MAP_READ, 0, &ms);
			m_pContext->Unmap(m_pStagingTexture2, 0);
			m_pContext->CopyResource(m_pStagingTexture, m_pDefaultTexture);
			m_pContext->Flush();
		}

		long lTime = GetTickCountDIFF1(lBeginTime, 0);
		odprintf1("lTime %ld ", lTime);
		Sleep(8);
	}
	

	

	//std::thread t(Render);
	//t.join();
	//while (1)
	//{
	//	while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	//	{
	//		if (msg.message == WM_QUIT)
	//		{
	//			return msg.wParam;
	//		}
	//		// 将类似组合键转换成可识别的
	//		TranslateMessage(&msg);
	//		// 分配消息
	//		DispatchMessage(&msg);
	//	}
	//	static float add = 0.0f;
	//	add += 2 * PI / 50;
	//	const float c = sin(add) / 2.0f + 0.5f;
	//	Sleep(10);
	//}

	return 0;
}

