#pragma once
#include "Module.h"

class MLauncher : public MModuleBase
{
	MODULE_CLASS_DECORATOR(MLauncher)

private:	
	class MWorld* World{ nullptr };

protected:
	void GenerateWorld();

	void TeardownWorld();

public:

	virtual void Init() override;

	virtual void Teardown() override;

	virtual void Update() override;


	void Serialize();
};
