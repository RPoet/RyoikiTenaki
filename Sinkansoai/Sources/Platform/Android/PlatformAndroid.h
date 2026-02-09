#pragma once

#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Sinkansoai", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "Sinkansoai", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "Sinkansoai", __VA_ARGS__))

struct MPlatformStartupParams
{
    ANativeActivity* Activity;
    ANativeWindow* Window;
    AAssetManager* AssetManager;
};

inline ANativeWindow* GetNativeWindowHandle(PlatformWindowHandle Handle)
{
    return static_cast<ANativeWindow*>(Handle);
}
