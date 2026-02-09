#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

struct MPlatformStartupParams
{
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPWSTR lpCmdLine;
    int nCmdShow;
};

inline HWND GetNativeWindowHandle(PlatformWindowHandle Handle)
{
    return static_cast<HWND>(Handle);
}
