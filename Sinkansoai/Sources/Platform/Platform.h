#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_ANDROID 0
#elif defined(__ANDROID__)
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_ANDROID 1
#else
    #error "Unsupported platform"
#endif

using PlatformWindowHandle = void*;

#if PLATFORM_WINDOWS
    #include "Windows/PlatformWindows.h"
#elif PLATFORM_ANDROID
    #include "Android/PlatformAndroid.h"
#endif
