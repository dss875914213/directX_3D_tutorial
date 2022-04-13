#pragma once
#include <d3d11.h>

class Graphics
{
public:
	Graphics(HWND hWnd);
	~Graphics();
	void Initialize(HWND hWnd); // ≥ı ºªØ direct3D
	void ClearBuffer(float red, float green, float blue);
	void InitEffect();
	void DrawTriangle();
	void EndDraw();
private:
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11InputLayout* m_inputLayout;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;
};

