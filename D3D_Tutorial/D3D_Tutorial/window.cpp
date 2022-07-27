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
	wc.style = 0;  // �������ͣ�
	wc.lpfnWndProc = HandleMsgSetup; // �ص�����
	wc.cbClsExtra = 0; // �����Ķ����ֽ���
	wc.cbWndExtra = 0; // ���ڷ���Ķ����ֽ���
	wc.hInstance = m_hInst; // ������Ĵ��ڹ��̵�ʵ���ľ�� ��
	wc.hIcon = LoadIcon(NULL, IDI_INFORMATION); // ����ͼ��
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); // �����״
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // ���ñ�����ɫ
	wc.lpszMenuName = TEXT("���� menu"); // �˵�����
	wc.lpszClassName = m_className.c_str(); // �������� ��������ʱ���ø����֣������ø�����

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

	HWND hWnd = CreateWindow(WindowClass::GetInstance().get()->GetName(),	// �����Ĵ�������
		TEXT("First Window"),												// ��������
		WS_OVERLAPPEDWINDOW,												// ��������
		200, 200,															// ������ʼλ��
		1920, 1080,								// ���ڿ��
		NULL,																// �����ھ��
		NULL,																// �˵����
		WindowClass::GetInstance()->GetHInstance(),							// Ҫ��ô��ڹ�����ģ��ʵ���ľ��
		this																// �� WM_CREATE ��Ϣ��ͨ�� lparam ����ص�����������
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
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG>(pWnd)); // �� this ָ�뱣���� GWLP_USERDATA
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG>(HandleMsgThunk)); // ���ص���������Ϊ HandleMsgThunk
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


