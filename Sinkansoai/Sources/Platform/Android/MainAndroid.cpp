#include "../../Definitions.h"
#include "../../Engine.h"
#include "PlatformAndroid.h"

#include <android_native_app_glue.h>

MStartupParams GStartupParams;

static void HandleAppCommand(android_app* app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
            if (app->window != nullptr)
            {
                GStartupParams.Window = app->window;
                GStartupParams.Activity = app->activity;
                GStartupParams.AssetManager = app->activity->assetManager;
                
                LOGI("Window initialized");
                
                MEngine& Engine = MEngine::Get();
                Engine.Init();
            }
            break;
            
        case APP_CMD_TERM_WINDOW:
            LOGI("Window terminated");
            {
                MEngine& Engine = MEngine::Get();
                Engine.Exit();
            }
            break;
            
        case APP_CMD_GAINED_FOCUS:
            LOGI("App gained focus");
            break;
            
        case APP_CMD_LOST_FOCUS:
            LOGI("App lost focus");
            break;
    }
}

static int32_t HandleInputEvent(android_app* app, AInputEvent* event)
{
    return 0;
}

void android_main(android_app* app)
{
    LOGI("Sinkansoai Android starting...");
    
    app->onAppCmd = HandleAppCommand;
    app->onInputEvent = HandleInputEvent;
    
    MEngine& Engine = MEngine::Get();
    
    while (true)
    {
        int events;
        android_poll_source* source;
        
        while (ALooper_pollAll(Engine.Run() ? 0 : -1, nullptr, &events, (void**)&source) >= 0)
        {
            if (source != nullptr)
            {
                source->process(app, source);
            }
            
            if (app->destroyRequested)
            {
                LOGI("Destroy requested, exiting...");
                return;
            }
        }
        
        if (Engine.Run() && app->window != nullptr)
        {
            Engine.Loop();
        }
    }
}
