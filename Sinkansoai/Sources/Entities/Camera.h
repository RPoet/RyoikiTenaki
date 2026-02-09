#pragma once
#include "PlaceableEntity.h"
#include "../Render/View.h"
#include "../Math/SIMDMath.h"


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

	SIMDMath::Matrix4x4 GetRotationMatrix()
	{
		float PitchRad = Transform.Rotation.x * SIMDMath::DegToRad;
		float YawRad = Transform.Rotation.y * SIMDMath::DegToRad;

		return SIMDMath::Matrix4x4::RotationX(PitchRad) * SIMDMath::Matrix4x4::RotationY(YawRad);
	}

	SIMDMath::Matrix4x4 GetCameraMatrix()
	{
		auto Rotation = GetRotationMatrix();
		auto Translation = SIMDMath::Matrix4x4::Translation(Transform.Position.x, Transform.Position.y, Transform.Position.z);
		
		return Rotation * Translation;
	}

	RViewContext GetViewContext()
	{
		RViewContext Out
		{
			GetCameraMatrix(),
			FoV,
			MinZ,
			uint2{Width, Height},
			ViewTranslation
		};

		return Out;
	}

};
