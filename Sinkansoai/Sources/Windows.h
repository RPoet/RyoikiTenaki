#pragma once
#define WIN32_LEAN_AND_MEAN
#include <vector>
#include <thread>
#include <string>
#include <windows.h>
#include <stdio.h>

#include "Definitions.h"
#include "Module.h"

class MWindow : public MModuleBase
{
	MODULE_CLASS_DECORATOR(MWindow)

private:
	HINSTANCE		HandleInstance;
	WNDCLASSEX		WinClassEx;
	HWND			HandleWindow;
	int32			Width, Height;
	float			FPS;

	String			WindowName;

	bool Init(const MStartupParams&, const String&, const String&);

public:

	virtual void Init() override;
	virtual void Teardown() override;
	virtual void Update() override;

	HWND GetHWND() { return  HandleWindow; }
	HINSTANCE GetHandleInstance() { return  HandleInstance; }
	int GetWidth() { return  Width; }
	int GetHeight() { return  Height; }
	void SetWidth(int InWidth) { Width = InWidth; }
	void SetHeight(int InHeight) { Height = InHeight; }

	void SetFrame(float Frame) { FPS = Frame; }

	friend LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

