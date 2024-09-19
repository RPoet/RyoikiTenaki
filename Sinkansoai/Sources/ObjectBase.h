#pragma once
#include "Definitions.h"

class MObjectBase
{
protected:
	using Base = MObjectBase;

private:
	int32 bVisible : 1;
	int32 bRegistered : 1;

public:

	MObjectBase();
	
	bool IsVisible() const { return bVisible; }

	bool IsRegistered() { return bRegistered; }


	void SetVisible(bool bVisible)
	{
		this->bVisible = bVisible;
	}

	void SetRegistered(bool bRegistered)
	{
		this->bRegistered = bRegistered;
	}

	virtual void RegisterToWorld() {};

	virtual void Destroy() {};
};
