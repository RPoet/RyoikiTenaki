#include <string>
#include <iostream>
#include "Windows.h"

IMPLEMENT_MODULE(MWindow)

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


bool MWindow::Init(const MStartupParams& StartupParams, const String& InClassName, const String& InAppName)
{
	Super::Init();

	RECT ActualDesktop;
	GetWindowRect(GetDesktopWindow(), &ActualDesktop);

	SetWidth(1920);
	SetHeight(1100);

	HandleInstance = StartupParams.hInstance;

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
	WinClassEx.lpszClassName = InClassName.c_str();
	WinClassEx.hIconSm = NULL;

	if (!RegisterClassEx(&WinClassEx)) return false;

	HandleWindow = CreateWindowEx(0
		, InClassName.c_str(),
		InAppName.c_str(),
		WS_OVERLAPPEDWINDOW, // When full mode WS_POPUP	would be popup
		CW_USEDEFAULT,       // When full mode 0	would be popup
		CW_USEDEFAULT,       // When full mode 0  would be popup
		Width,
		Height,
		nullptr,
		nullptr,
		HandleInstance,
		nullptr);

	ShowWindow(HandleWindow, StartupParams.nCmdShow);

	ShowCursor(true);

	return true;
}

void MWindow::Init()
{
	const static String ClassName(TEXT("Sinkansoai"));
	const static String AppNameName(TEXT("Sinkansoai"));

	Init(GStartupParams, ClassName, AppNameName);

	cout << "Windows module Init" << endl;
}


void MWindow::Teardown()
{
	::DestroyWindow(GetHWND());
}


void MWindow::Update()
{
	// nothing to do...
}
