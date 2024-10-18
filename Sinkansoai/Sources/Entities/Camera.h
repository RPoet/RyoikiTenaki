#pragma once
#include "PlaceableEntity.h"
#include "../Render/View.h"


class MCamera : public MPlaceableEntity
{
	CLASS_DECORATOR(MCamera)

private:
	float FoV = 60;
	float MinZ = 0.0001f;

	uint32 Width = 1920;
	uint32 Height = 1080;

public:
	MCamera() = default;
	virtual ~MCamera() = default;

	void SetFov(float FoV)
	{
		this->FoV = FoV;
	}

	float GetFoV() const { return FoV; }


	virtual void Register() override;
	virtual void Destroy() override;
	virtual void Tick(float DeltaTime) override;

	RViewContext GetViewContext()
	{
		RViewContext Out
		{
			GetTransform().ToMatrix(),
			FoV,
			MinZ,
			uint2{Width, Height}
		};

		return Out;
	}

};
