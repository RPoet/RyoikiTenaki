#pragma once
#include "PlaceableEntity.h"
#include "../Math/SIMDMath.h"

enum class ELightType : uint8
{
	Directional,
	Point,
	Spot
};

class MLightEntity : public MPlaceableEntity
{
	CLASS_DECORATOR(MLightEntity)

private:
	ELightType Type = ELightType::Point;
	float3 Color = { 1.0f, 1.0f, 1.0f };
	float Intensity = 1.0f;
	float Range = 500.0f;
	float InnerConeAngle = 20.0f;
	float OuterConeAngle = 40.0f;
	bool bEnabled = true;
	bool bAnimate = false;

public:
	MLightEntity() = default;
	explicit MLightEntity(ELightType InType)
		: Type(InType)
	{
	}

	virtual ~MLightEntity() = default;

	ELightType GetType() const { return Type; }
	void SetType(ELightType InType) { Type = InType; }

	float3 GetColor() const { return Color; }
	void SetColor(const float3& InColor) { Color = InColor; }

	float GetIntensity() const { return Intensity; }
	void SetIntensity(float InIntensity) { Intensity = InIntensity; }

	float GetRange() const { return Range; }
	void SetRange(float InRange) { Range = InRange; }

	float GetInnerConeAngle() const { return InnerConeAngle; }
	void SetInnerConeAngle(float InAngle) { InnerConeAngle = InAngle; }

	float GetOuterConeAngle() const { return OuterConeAngle; }
	void SetOuterConeAngle(float InAngle) { OuterConeAngle = InAngle; }

	bool IsEnabled() const { return bEnabled; }
	void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

	bool IsAnimated() const { return bAnimate; }
	void SetAnimated(bool bInAnimate) { bAnimate = bInAnimate; }

	float3 GetWorldDirection() const
	{
		const auto& T = GetTransform();
		const float Pitch = T.Rotation.x * SIMDMath::DegToRad;
		const float Yaw = T.Rotation.y * SIMDMath::DegToRad;
		const float Roll = T.Rotation.z * SIMDMath::DegToRad;
		const auto R = SIMDMath::Matrix4x4::RotationX(Pitch) *
			SIMDMath::Matrix4x4::RotationY(Yaw) *
			SIMDMath::Matrix4x4::RotationZ(Roll);
		return SIMDMath::TransformVector(float3(0, 0, 1), R);
	}

	virtual void Tick(float DeltaTime) override
	{
		if (!bAnimate)
		{
			return;
		}

		if (Type == ELightType::Point || Type == ELightType::Spot)
		{
			auto& T = GetTransform();
			float3 Position = { T.Position.x, T.Position.y, T.Position.z };
			Position.y += DeltaTime * 200.0f;
			if (Position.y > 1000.0f)
			{
				Position.y = -20.0f;
			}
			T.SetPosition(Position.x, Position.y, Position.z);
		}
	}
};
