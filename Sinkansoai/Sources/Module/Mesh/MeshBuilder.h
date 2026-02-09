#pragma once
#include "../../Module.h"
#include "../../Engine/Mesh.h"

class MMeshBuilder : public MModuleBase
{
	MODULE_CLASS_DECORATOR(MMeshBuilder)

private:
public:

	virtual void Init() override;

	virtual void Teardown() override;

	vector<MMesh> LoadMesh(const String& Path, const String& ModelName, vector<Material>& OutMaterials);
	vector<MMesh> LoadMeshGLTF(const String& Path, const String& ModelName, vector<Material>& OutMaterials);
	vector<MMesh> LoadMeshFBX(const String& Path, const String& ModelName, vector<Material>& OutMaterials);
};

