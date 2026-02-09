#pragma once
#include "RenderResource.h"

class RTexture : public RRenderResource
{
protected:
	
	uint32 Width = 0;
	uint32 Height = 0;
	uint32 NumMips = 0;
	uint32 PixelSizeInBytes = 0;

public:
	RTexture(const String& Name)
		: RRenderResource(Name)
	{}

	RTexture(String&& Name)
		: RRenderResource(std::move(Name))
	{}

	virtual ~RTexture() = default;


	uint32 GetWidth() const
	{
		return Width;
	}

	uint32 GetHeight() const
	{
		return Height;
	}

	uint32 GetNumMips() const
	{
		return NumMips;
	}

	uint32 GetPixelSizeInBytes() const
	{
		return PixelSizeInBytes;
	}


	// Temporaly used for Mip level 0.
	virtual void StreamTexture( void* pData ) {}
};

// Role-based naming aliases (render resource)
using TextureResource = RTexture;

