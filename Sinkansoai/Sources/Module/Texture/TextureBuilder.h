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

	vector<uint8> GenerateDefaultTexture(const uint32 Width, const uint32 Height, const uint32 PixelSizeInBytes);

	vector<uint8> GenerateDefaultColoredTexture(const uint32 Width, const uint32 Height, const uint32 PixelSizeInBytes, uint32 Color);

	MTexture LoadTexture(const String& Path, const String& TextureName);

};

