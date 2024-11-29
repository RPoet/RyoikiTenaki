#pragma once
#include "PlaceableEntity.h"
#include "../Render/View.h"


class MCamera : public MPlaceableEntity
{
	CLASS_DECORATOR(MCamera)

private:
	float FoV = 103;
	float MinZ = 1;

	uint32 Width = 1920;
	uint32 Height = 1080;

	float3 ViewTranslation = { 0,0,0 };

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
			uint2{Width, Height},
			ViewTranslation
		};

		return Out;
	}

};
