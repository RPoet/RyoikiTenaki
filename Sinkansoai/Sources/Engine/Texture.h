#pragma once
#include "../PlatformDefinitions.h"


struct MTexture
{
	String Name;
	vector<uint8> Pixels;
	uint32 Width, Height, Size, BitsPerPixel;
};
