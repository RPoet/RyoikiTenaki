#pragma once
#define WIN32_LEAN_AND_MEAN
#include <vector>
#include <thread>
#include <string>
#include <windows.h>
#include <stdio.h>

#include "Definitions.h"
#pragma warning(disable:4996)
#define START_CONSOLE() {AllocConsole();  freopen("CONOUT$", "w", stdout); freopen("CONIN$", "r", stdin);}
#define STOP_CONSOLE()  {FreeConsole();}


class Window
{
private:
	HINSTANCE		HandleInstance;
	WNDCLASSEX		WinClassEx;
	HWND			HandleWindow;
	int32			Width, Height;

	String			ClassName;
	String			AppName;

public:
	bool Init(HINSTANCE, int32, String&, String&);
	bool Exit();
	void Update();
	HWND GetHWND() { return  HandleWindow; }
	HINSTANCE GetHandleInstance() { return  HandleInstance; }
	int GetWidth() { return  Width; }
	int GetHeight() { return  Height; }
	void SetWidth(int InWidth) { Width = InWidth; }
	void SetHeight(int InHeight) { Height = InHeight; }

	friend LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

