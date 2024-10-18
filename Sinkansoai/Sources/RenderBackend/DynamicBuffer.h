#pragma once
#include "RenderResource.h"

class RDynamicBuffer : public RRenderResource
{
protected:
	uint32 Size{};
	UINT8* Data{};

public:
	RDynamicBuffer(const String& Name)
		: RRenderResource(Name)
	{}

	RDynamicBuffer(String&& Name)
		: RRenderResource(std::move(Name))
	{}

	virtual ~RDynamicBuffer() = default;

	template<class T>
	void CopyData(T&& CopyParam)
	{
		assert(Data != nullptr && bInitialized && " Data must be allocated before copying to it ");
		uint32 SizeToCopy = sizeof(T);
		memcpy(Data, &CopyParam, SizeToCopy);
	}
};

