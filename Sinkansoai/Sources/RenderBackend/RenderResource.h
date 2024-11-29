#pragma once
#include "../PlatformDefinitions.h"

class RRenderResource
{
protected:
	String Name{ TEXT("Default") };

	bool bInitialized = false;

public:
	RRenderResource(const String& Name)
		: Name(Name)
	{}

	RRenderResource(String&& Name)
		: Name(std::move(Name))
	{}

	const String& GetName() const
	{
		return Name;
	}


	virtual void AllocateResource()
	{
		bInitialized = true;
	};

	virtual void DeallocateResource()
	{
		bInitialized = false;
	};
};

