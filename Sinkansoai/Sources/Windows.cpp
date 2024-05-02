#include <string>
#include <iostream>
#include "Windows.h"


LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_MOUSEWHEEL:

		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


bool Window::Init(HINSTANCE hInstance, int32 nCmdShow, String& InClassName, String& InAppName)
{
	RECT ActualDesktop;
	GetWindowRect(GetDesktopWindow(), &ActualDesktop);

	SetWidth(1920);
	SetHeight(1100);

	HandleInstance = hInstance;
	ClassName = (InClassName);
	AppName = (InAppName);
	WinClassEx.cbSize = sizeof(WNDCLASSEX);
	WinClassEx.style = CS_HREDRAW | CS_VREDRAW;
	WinClassEx.lpfnWndProc = WindowProcedure;
	WinClassEx.cbClsExtra = 0;
	WinClassEx.cbWndExtra = 0;
	WinClassEx.hInstance = HandleInstance;
	WinClassEx.hIcon = NULL;
	WinClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WinClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WinClassEx.lpszMenuName = NULL;
	WinClassEx.lpszClassName = Window::ClassName.c_str();
	WinClassEx.hIconSm = NULL;

	if (!RegisterClassEx(&WinClassEx)) return false;

	HandleWindow = CreateWindowEx(0
		, ClassName.c_str(),
		AppName.c_str(),
		WS_OVERLAPPEDWINDOW, // When full mode WS_POPUP	would be popup
		CW_USEDEFAULT,       // When full mode 0	would be popup
		CW_USEDEFAULT,       // When full mode 0  would be popup
		Width,
		Height,
		nullptr,
		nullptr,
		HandleInstance,
		nullptr);

	ShowWindow(HandleWindow, nCmdShow);

	START_CONSOLE();

	ShowCursor(true);

	return true;
}


bool Window::Exit()
{
	::DestroyWindow(GetHWND());

	STOP_CONSOLE();
	return true;
}


void Window::Update()
{


}
