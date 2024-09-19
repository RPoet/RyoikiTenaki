#pragma once
#include <vector>
#include "Singleton.h"
#include "Definitions.h"

class MModuleBase;

class MEngine : public Singleton<MEngine>
{
private:
	vector<MModuleBase*> Modules;

	float DeltaTime;

public:

	void Init();
	void Exit();
	void Loop();


	void RegisterModule(MModuleBase* Module);
};
