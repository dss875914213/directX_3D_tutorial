#pragma once
#include <windows.h>
#include <string>
#include <mutex>
#include <memory>

class Window
{
public:
	Window();
	~Window();
	Window(Window&) = delete;
	Window& operator=(const Window&) = delete;
	static LRESULT CALLBACK HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	class WindowClass
	{
		typedef std::shared_ptr<WindowClass> Ptr;
	public:
		~WindowClass();
		WindowClass(WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;
		static Ptr GetInstance();

		const char* GetName();
		const HINSTANCE  GetHInstance();

	private:
		WindowClass();
	private:		
		static Ptr m_instance;
		static std::mutex m_mutex;
		const HINSTANCE m_hInst;
		const std::string m_className;
	};
};

