#pragma once
#include <d3d11.h>
#include "event.h"
#include <windows.h>
#include <stdio.h>
#include <mutex>
#include <DirectXMath.h>

#ifndef D3DX_PI
#define D3DX_PI    (3.14159265358979323846)
#endif

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
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11InputLayout* m_inputLayout;
	ID3D11Buffer* m_constBuffer;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;

	std::mutex			m_mutex;
	float				m_threshold;
	BOOL				m_transParent;
	Transformation		m_transformation;
	DirectX::XMMATRIX	m_model;
	DirectX::XMMATRIX	m_view;
	DirectX::XMMATRIX	m_projection;
};


