#include <windows.h>
#include "window.h"
#include "Graphics.h"
#include <thread>
#define  PI 3.1415926

Window window;

void Render()
{
	Graphics* m_g = new Graphics(NULL);
	while (1)
	{
		m_g->DrawPicture();
		Sleep(16);
	}
}


// �ص�����  �����ȡ����Ϣ
int WINAPI WinMain(
	HINSTANCE hInstance,		// Ӧ�ó�����
	HINSTANCE hPrevInstance,	// ��һ��Ӧ�ó���ľ����Ŀǰ������ һֱΪ NULL
	LPSTR     lpCmdLine,		// �����������
	int       nShowCmd			// ���ô��ڵ���ʾ��ʽ
)
{
	

	std::thread t(Render);
	t.join();
	//while (1)
	//{
	//	while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	//	{
	//		if (msg.message == WM_QUIT)
	//		{
	//			return msg.wParam;
	//		}
	//		// ��������ϼ�ת���ɿ�ʶ���
	//		TranslateMessage(&msg);
	//		// ������Ϣ
	//		DispatchMessage(&msg);
	//	}
	//	static float add = 0.0f;
	//	add += 2 * PI / 50;
	//	const float c = sin(add) / 2.0f + 0.5f;
	//	Sleep(10);
	//}

	return 0;
}

