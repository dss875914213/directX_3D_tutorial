#include "window.h"
#include "Graphics.h"


Window::WindowClass::Ptr Window::WindowClass::m_instance = nullptr;

std::mutex Window::WindowClass::m_mutex;

Window::WindowClass::~WindowClass()
{

}

Window::WindowClass::Ptr Window::WindowClass::GetInstance()
{
	m_instance = std::shared_ptr<WindowClass>(new WindowClass);
	return m_instance;
}

const char* Window::WindowClass::GetName()
{
	return m_className.c_str();
}

const HINSTANCE Window::WindowClass::GetHInstance()
{
	return m_hInst;
}

Window::WindowClass::WindowClass()
	:m_hInst(GetModuleHandle(NULL)),
	m_className("0_51WindowClass")
{
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = 0;  // 窗口类型？
	wc.lpfnWndProc = HandleMsgSetup; // 回调函数
	wc.cbClsExtra = 0; // 类分配的额外字节数
	wc.cbWndExtra = 0; // 窗口分配的额外字节数
	wc.hInstance = m_hInst; // 包含类的窗口过程的实例的句柄 ？
	wc.hIcon = LoadIcon(NULL, IDI_INFORMATION); // 窗口图标
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); // 光标形状
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 设置背景颜色
	wc.lpszMenuName = TEXT("窗口 menu"); // 菜单名字
	wc.lpszClassName = m_className.c_str(); // 窗口类名 创建窗口时设置该名字，来引用该属性

	RegisterClass(&wc);
}

Window::Window()
	:m_g(nullptr)
{
	INT width = 1920;
	INT height = 1080;
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindow(WindowClass::GetInstance().get()->GetName(),	// 创建的窗口类名
		TEXT("First Window"),												// 窗口名称
		WS_OVERLAPPEDWINDOW,												// 窗口类型
		200, 200,															// 窗口起始位置
		1920, 1080,								// 窗口宽高
		NULL,																// 父窗口句柄
		NULL,																// 菜单句柄
		WindowClass::GetInstance()->GetHInstance(),							// 要与该窗口关联的模块实例的句柄
		this																// 更 WM_CREATE 消息，通过 lparam 传入回调函数的数据
	);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	
	m_g = new Graphics(hWnd);

	m_winEvent.associate(m_g, &Graphics::Message);
}

Graphics* Window::GetGraphics()
{
	return m_g;
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_CREATE)
	{
		const CREATESTRUCT* const pCreate = reinterpret_cast<CREATESTRUCT*>(lparam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG>(pWnd)); // 将 this 指针保存在 GWLP_USERDATA
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG>(HandleMsgThunk)); // 将回调函数设置为 HandleMsgThunk
		return pWnd->HandleMsg(hwnd, msg, wparam, lparam);
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK Window::HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LONG param = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	Window* const pWnd = reinterpret_cast<Window*>(param);
	return pWnd->HandleMsg(hwnd, msg, wparam, lparam);
}

LRESULT Window::HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		m_winEvent.sendEvent(wparam);
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

Window::~Window()
{
	m_winEvent.disAssociate(m_g, &Graphics::Message);
	delete m_g;
}


