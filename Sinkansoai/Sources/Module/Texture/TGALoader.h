#pragma once
#include "../../Module.h"
#include "../../Engine/Texture.h"

class MTextureBuilder : public MModuleBase
{
	MODULE_CLASS_DECORATOR(MTextureBuilder)

private:


public:

	virtual void Init() override;

	virtual void Teardown() override;

	MTexture LoadTexture(const String& Path, const String& TextureName);
};

