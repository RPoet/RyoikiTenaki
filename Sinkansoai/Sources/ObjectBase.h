#pragma once
#include "Definitions.h"

class MObjectBase
{
protected:
	using Base = MObjectBase;

private:
	int32 bVisible : 1;
	int32 bRegistered : 1;

protected:

	void SetRegistered(bool bRegistered)
	{
		this->bRegistered = bRegistered;
	}

public:

	MObjectBase();
	virtual ~MObjectBase() = default;
	
	bool IsVisible() const { return bVisible; }
	bool IsRegistered() { return bRegistered; }

	void SetVisible(bool bVisible)
	{
		this->bVisible = bVisible;
	}

	// need to serialize
	virtual void Serialize() {};
};
