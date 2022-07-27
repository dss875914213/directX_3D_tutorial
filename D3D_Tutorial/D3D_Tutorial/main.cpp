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


// 回调函数  处理获取的消息
int WINAPI WinMain(
	HINSTANCE hInstance,		// 应用程序句柄
	HINSTANCE hPrevInstance,	// 上一个应用程序的句柄，目前已弃用 一直为 NULL
	LPSTR     lpCmdLine,		// 传入的命令行
	int       nShowCmd			// 设置窗口的显示方式
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

