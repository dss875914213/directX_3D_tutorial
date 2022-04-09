#include <windows.h>

// 回调函数  处理获取的消息
LRESULT CALLBACK MyWinProc(HWND hwnd, // 窗口句柄
	UINT msg, // 消息类型
	WPARAM wparam,  // 辅助消息 键盘消息中，该参数代表哪个键按下
	LPARAM lparam)  // 辅助消息 鼠标信息中，该参数的低字节代表 x 坐标，高字节代表 y 坐标
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(
	HINSTANCE hInstance,		// 应用程序句柄
	HINSTANCE hPrevInstance,	// 上一个应用程序的句柄，目前已弃用 一直为 NULL
	LPSTR     lpCmdLine,		// 传入的命令行
	int       nShowCmd			// 设置窗口的显示方式
)
{
	auto className = TEXT("FirstWindowClass");
	// 0. 设置窗口属性
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = 0;  // 窗口类型？
	wc.lpfnWndProc = MyWinProc; // 回调函数
	wc.cbClsExtra = 0; // 类分配的额外字节数
	wc.cbWndExtra = 0; // 窗口分配的额外字节数
	wc.hInstance = hInstance; // 包含类的窗口过程的实例的句柄 ？
	wc.hIcon = LoadIcon(NULL, IDI_INFORMATION); // 窗口图标
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); // 光标形状
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 设置背景颜色
	wc.lpszMenuName = TEXT("窗口 menu"); // 菜单名字
	wc.lpszClassName = className; // 窗口类名 创建窗口时设置该名字，来引用该属性

	// 1. 注册窗口
	RegisterClass(&wc);

	// 2. 创建窗口
	HWND hWnd = CreateWindow(className,			// 创建的窗口类名
		TEXT("First Window"),		// 窗口名称
		WS_OVERLAPPEDWINDOW,		// 窗口类型
		200, 200,					// 窗口起始位置
		400, 300,					// 窗口宽高
		NULL,						// 父窗口句柄
		NULL,						// 菜单句柄
		hInstance,					// 要与该窗口关联的模块实例的句柄
		NULL						// 更 WM_CREATE 消息，通过 lparam 传入回调函数的数据
	);

	// 3. 显示窗口
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	// 4. 消息分发
	MSG msg;
	BOOL ret;
	while ((ret = GetMessage(&msg, NULL, NULL, NULL)) != 0)
	{
		// 将类似组合键转换成可识别的
		TranslateMessage(&msg);
		// 分配消息
		DispatchMessage(&msg);
	}

	return 0;
}

