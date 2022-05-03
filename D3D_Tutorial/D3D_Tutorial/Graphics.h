#pragma once
#include <d3d11.h>
#include "event.h"
#include <windows.h>
#include <stdio.h>
#include <mutex>

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
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11InputLayout* m_inputLayout;
	ID3D11Buffer* m_constBuffer;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;

	std::mutex	m_mutex;
	float		m_threshold;
};


