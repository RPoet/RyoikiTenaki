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


	MEngine RyoikiTenkai;
	RyoikiTenkai.Init();

	{
		MSG Out = {};
		while (WM_QUIT != Out.message)
		{
			RyoikiTenkai.Loop();

			if (PeekMessage(&Out, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&Out);
				DispatchMessage(&Out);
				continue;
			}
		}
	}

	RyoikiTenkai.Exit();
	App.Exit();

}

