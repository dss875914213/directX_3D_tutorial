#include <windows.h>

// �ص�����  �����ȡ����Ϣ
LRESULT CALLBACK MyWinProc(HWND hwnd, // ���ھ��
	UINT msg, // ��Ϣ����
	WPARAM wparam,  // ������Ϣ ������Ϣ�У��ò��������ĸ�������
	LPARAM lparam)  // ������Ϣ �����Ϣ�У��ò����ĵ��ֽڴ��� x ���꣬���ֽڴ��� y ����
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
	HINSTANCE hInstance,		// Ӧ�ó�����
	HINSTANCE hPrevInstance,	// ��һ��Ӧ�ó���ľ����Ŀǰ������ һֱΪ NULL
	LPSTR     lpCmdLine,		// �����������
	int       nShowCmd			// ���ô��ڵ���ʾ��ʽ
)
{
	auto className = TEXT("FirstWindowClass");
	// 0. ���ô�������
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = 0;  // �������ͣ�
	wc.lpfnWndProc = MyWinProc; // �ص�����
	wc.cbClsExtra = 0; // �����Ķ����ֽ���
	wc.cbWndExtra = 0; // ���ڷ���Ķ����ֽ���
	wc.hInstance = hInstance; // ������Ĵ��ڹ��̵�ʵ���ľ�� ��
	wc.hIcon = LoadIcon(NULL, IDI_INFORMATION); // ����ͼ��
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); // �����״
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // ���ñ�����ɫ
	wc.lpszMenuName = TEXT("���� menu"); // �˵�����
	wc.lpszClassName = className; // �������� ��������ʱ���ø����֣������ø�����

	// 1. ע�ᴰ��
	RegisterClass(&wc);

	// 2. ��������
	HWND hWnd = CreateWindow(className,			// �����Ĵ�������
		TEXT("First Window"),		// ��������
		WS_OVERLAPPEDWINDOW,		// ��������
		200, 200,					// ������ʼλ��
		400, 300,					// ���ڿ��
		NULL,						// �����ھ��
		NULL,						// �˵����
		hInstance,					// Ҫ��ô��ڹ�����ģ��ʵ���ľ��
		NULL						// �� WM_CREATE ��Ϣ��ͨ�� lparam ����ص�����������
	);

	// 3. ��ʾ����
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	// 4. ��Ϣ�ַ�
	MSG msg;
	BOOL ret;
	while ((ret = GetMessage(&msg, NULL, NULL, NULL)) != 0)
	{
		// ��������ϼ�ת���ɿ�ʶ���
		TranslateMessage(&msg);
		// ������Ϣ
		DispatchMessage(&msg);
	}

	return 0;
}

