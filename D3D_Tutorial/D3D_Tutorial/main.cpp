#include <windows.h>
#include "window.h"
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
	while ((ret = GetMessage(&msg, NULL, NULL, NULL)) != 0)
	{
		// ��������ϼ�ת���ɿ�ʶ���
		TranslateMessage(&msg);
		// ������Ϣ
		DispatchMessage(&msg);
	}

	return 0;
}

