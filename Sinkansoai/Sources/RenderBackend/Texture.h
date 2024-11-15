#pragma once
#include "RenderResource.h"

class RTexture : public RRenderResource
{
protected:
	
	uint32 Width = 0;
	uint32 Height = 0;
	uint32 PixelSizeInBytes = 0;

public:
	RTexture(const String& Name)
		: RRenderResource(Name)
	{}

	RTexture(String&& Name)
		: RRenderResource(std::move(Name))
	{}

	virtual ~RTexture() = default;


};

