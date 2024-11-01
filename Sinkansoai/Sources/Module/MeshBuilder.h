#pragma once
#include "../Module.h"
#include "../Engine/Mesh.h"

class MMeshBuilder : public MModuleBase
{
	MODULE_CLASS_DECORATOR(MMeshBuilder)

private:


public:

	virtual void Init() override;

	virtual void Teardown() override;

	MMesh LoadMesh(const String& Path);
};

