#pragma once
#include "../Definitions.h"
#include "../ObjectBase.h"

class IComponent
{
private:
	int32 bVisible : 1;

public:
	
	IComponent() = default;
	virtual ~IComponent() = default;

	virtual void Init() {};
	virtual void Destroy() {};
	virtual void Tick(float Delta) {};


	void SetVisible(bool bVisible)
	{
		this->bVisible = bVisible;
	}

	bool IsVisible()
	{
		return bVisible;
	}
};
