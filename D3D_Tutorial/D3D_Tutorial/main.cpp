#include <windows.h>
#include "window.h"
#define  PI 3.1415926

// 回调函数  处理获取的消息
int WINAPI WinMain(
	HINSTANCE hInstance,		// 应用程序句柄
	HINSTANCE hPrevInstance,	// 上一个应用程序的句柄，目前已弃用 一直为 NULL
	LPSTR     lpCmdLine,		// 传入的命令行
	int       nShowCmd			// 设置窗口的显示方式
)
{
	Window window;
	
	MSG msg;
	BOOL ret;
	window.GetGraphics()->Create();
	while ((ret = GetMessage(&msg, NULL, NULL, NULL)) != 0)
	{
		// 将类似组合键转换成可识别的
		TranslateMessage(&msg);
		// 分配消息
		DispatchMessage(&msg);

		//static float add = 0.0f;
		//add += 2 * PI / 50;
		//const float c = sin(add) / 2.0f + 0.5f;
		window.GetGraphics()->ClearBuffer(1.0f, 1.0f, 1.0f);
		window.GetGraphics()->DrawPicture();
		window.GetGraphics()->EndDraw();
	}

	return 0;
}

