#pragma once
#include <d3d11.h>
#include "event.h"
#include <windows.h>
#include <stdio.h>
#include <mutex>
#include <DirectXMath.h>
#include <wrl/client.h>

#ifndef D3DX_PI
#define D3DX_PI    (3.14159265358979323846)
#endif

void __cdecl odprintf(const char* format, ...);

struct Transformation
{
	DirectX::XMFLOAT4	clip;
	DirectX::XMFLOAT2	size;
	DirectX::XMFLOAT2	pos;
	DirectX::XMFLOAT2	scale;
	BOOL				flipH;
	BOOL				flipV;
	FLOAT				angle;
};

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace DXSpace
{
	class Graphics : public Object
	{
	public:
		Graphics(HWND hWnd);
		~Graphics();
		void Initialize(HWND hWnd); // 初始化 direct3D
		void Create();
		void ClearBuffer(float red, float green, float blue);
		void InitEffect();
		void DrawPicture();
		void EndDraw();

		void Message(int msg);
	private:
		void SetVertexBuffer();
		DirectX::XMFLOAT4X4 SetMVP();
	private:
	

		ComPtr<ID3D11VertexShader>		m_pVertexShader;
		ComPtr<ID3D11PixelShader>		m_pPixelShader;
		ComPtr<ID3D11InputLayout>		m_inputLayout;
		ComPtr<ID3D11Buffer>			m_constBuffer;

		ComPtr<ID3D11Device>			m_pDevice;
		ComPtr<ID3D11DeviceContext>		m_pContext;
		ComPtr<IDXGISwapChain>			m_pSwapChain;
		ComPtr<ID3D11RenderTargetView>	m_pRenderTargetView;
		ComPtr<ID3D11RenderTargetView>	m_pRenderTargetView2;
		ComPtr<ID3D11Texture2D>			m_backBuffer;
		ComPtr<ID3D11Texture2D>			m_pTexture;
		ComPtr<ID3D11Texture2D>			m_pDefaultTexture;
		ComPtr<ID3D11Texture2D>			m_pImmutableTexture;
		ComPtr<ID3D11Texture2D>			m_pDynamicTexture;
		ComPtr<ID3D11Texture2D>			m_pStagingTexture;
		ComPtr<ID3D11Texture2D>			m_pStagingTexture2;
		ComPtr<ID3D11Texture2D>			m_pStagingTexture3;
		ComPtr<ID3D11Texture2D>			m_pStagingTexture4;

		std::mutex			m_mutex;
		float				m_threshold;
		BOOL				m_transParent;
		Transformation		m_transformation;
		DirectX::XMMATRIX	m_model;
		DirectX::XMMATRIX	m_view;
		DirectX::XMMATRIX	m_projection;
		DirectX::XMFLOAT2	m_screenSize;
	};
}


