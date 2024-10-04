#include "Definitions.h"
#include "Windows.h"
#include "Engine.h"

auto APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrecInstace, LPWSTR lpCmdLine, int32 nCmdShow)->int
{
	String ClassName(TEXT("Sinkansoai"));
	String AppNameName(TEXT("Sinkansoai"));

	Window App{};
	App.Init(hInstance, nCmdShow, ClassName, AppNameName);

	cout << " Console log test " << endl;

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
	App.Exit();

}

