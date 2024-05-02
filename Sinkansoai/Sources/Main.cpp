#include "Definitions.h"
#include "Windows.h"

auto APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrecInstace, LPWSTR lpCmdLine, int32 nCmdShow)->int
{
	String ClassName(TEXT("Sinkansoai"));
	String AppNameName(TEXT("Sinkansoai"));

	Window App{};
	App.Init(hInstance, nCmdShow, ClassName, AppNameName);

	cout << " Console log test " << endl;

	MSG Out = {};
	while (WM_QUIT != Out.message) {

		if (PeekMessage(&Out, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Out);
			DispatchMessage(&Out);
			continue;
		}
	}
	App.Exit();

}

