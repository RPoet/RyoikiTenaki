#pragma once
#include "../PlatformDefinitions.h"


enum TextureType
{
	DIFFUSE,
	NORMAL,
	SPECULAR,
	SHININESS,
	AMBIENT,
	EMISSIVE,
	REFLECTION,
	COUNT
};

constexpr uint32 kMaterialTextureSlotCount = static_cast<uint32>(TextureType::COUNT);

constexpr bool IsSRGBTextureType(TextureType Type)
{
	switch (Type)
	{
	case DIFFUSE:
	case EMISSIVE:
	case REFLECTION:
		return true;
	default:
		return false;
	}
}

struct MTexture
{
	String Name;
	vector<uint8> Pixels;
	uint32 Width, Height, Size, BitsPerPixel;
	bool bSRGB = false;
};

// Role-based naming alias (CPU asset)
using TextureAsset = MTexture;
