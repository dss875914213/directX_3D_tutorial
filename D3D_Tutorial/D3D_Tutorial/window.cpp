#include "window.h"

Window::WindowClass::Ptr Window::WindowClass::m_instance = nullptr;

std::mutex Window::WindowClass::m_mutex;

Window::WindowClass::~WindowClass()
{

}

Window::WindowClass::Ptr Window::WindowClass::GetInstance()
{
	if (m_instance == nullptr)
	{
		std::lock_guard<std::mutex> autolock(m_mutex);
		if (m_instance == nullptr)
		{
			m_instance = std::shared_ptr<WindowClass>(new WindowClass);
		}
	}
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
{
	HWND hWnd = CreateWindow(WindowClass::GetInstance().get()->GetName(),	// �����Ĵ�������
		TEXT("First Window"),												// ��������
		WS_OVERLAPPEDWINDOW,												// ��������
		200, 200,															// ������ʼλ��
		400, 300,															// ���ڿ��
		NULL,																// �����ھ��
		NULL,																// �˵����
		WindowClass::GetInstance()->GetHInstance(),							// Ҫ��ô��ڹ�����ģ��ʵ���ľ��
		this																// �� WM_CREATE ��Ϣ��ͨ�� lparam ����ص�����������
	);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// -DSS TEST ����ָ WM_NCCREATE ���� WM_CREATE
	if (msg == WM_CREATE)
	{
		const CREATESTRUCT* const pCreate = reinterpret_cast<CREATESTRUCT*>(lparam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG>(pWnd));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG>(HandleMsgThunk));
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
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

Window::~Window()
{

}


