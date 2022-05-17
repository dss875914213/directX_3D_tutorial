#include <windows.h>
#include "window.h"
#include "Graphics.h"
#define  PI 3.1415926

// �ص�����  �����ȡ����Ϣ
int WINAPI WinMain(
	HINSTANCE hInstance,		// Ӧ�ó�����
	HINSTANCE hPrevInstance,	// ��һ��Ӧ�ó���ľ����Ŀǰ������ һֱΪ NULL
	LPSTR     lpCmdLine,		// �����������
	int       nShowCmd			// ���ô��ڵ���ʾ��ʽ
)
{
	Window window;
	
	MSG msg;
	BOOL ret;
	window.GetGraphics()->Create();

	while (1)
	{
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return msg.wParam;
			}
			// ��������ϼ�ת���ɿ�ʶ���
			TranslateMessage(&msg);
			// ������Ϣ
			DispatchMessage(&msg);
		}
		static float add = 0.0f;
		add += 2 * PI / 50;
		const float c = sin(add) / 2.0f + 0.5f;
		window.GetGraphics()->ClearBuffer(1.0f, 1.0f, 1.0f);
		window.GetGraphics()->DrawPicture();
		window.GetGraphics()->EndDraw();
		Sleep(33);
	}

	return 0;
}

