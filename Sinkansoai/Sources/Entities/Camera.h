#pragma once
#include "PlaceableEntity.h"

class MCamera : public MPlaceableEntity
{
	CLASS_DECORATOR(MCamera)

private:
	float Fov = 60;
	uint32 Width = 1920;
	uint32 Height = 1080;
	
	float MinZ = 0.0001f;

public:
	MCamera() = default;
	virtual ~MCamera() = default;


	virtual void Register() override;
	virtual void Destroy() override;
	virtual void Tick(float DeltaTime) override;
};
