#include "Definitions.h"
#include "Windows.h"
#include "Engine.h"

#pragma warning(disable:4996)
#define START_CONSOLE() {(void)AllocConsole();  (void)freopen("CONOUT$", "w", stdout); (void)freopen("CONIN$", "r", stdin);}
#define STOP_CONSOLE()  {(void)FreeConsole();}

MStartupParams GStartupParams;

auto APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrecInstace, LPWSTR lpCmdLine, int32 nCmdShow)->int
{
	GStartupParams = { hInstance, hPrecInstace, lpCmdLine, nCmdShow };

	START_CONSOLE();

	MEngine& Engine = MEngine::Get();
	Engine.Init();

	{
		MSG Out = {};
		while (WM_QUIT != Out.message && Engine.Run())
		{
			Engine.Loop();

			if (PeekMessage(&Out, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&Out);
				DispatchMessage(&Out);
				continue;
			}
		}
	}

	Engine.Exit();

	Sleep(5000);

	STOP_CONSOLE();
}

#undef START_CONSOLE()
#undef STOP_CONSOLE()
