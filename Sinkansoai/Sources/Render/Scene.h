#pragma once
#include "View.h"

class RScene
{
private:
	// Manage Scene Primitives;


	float DeltaTime{};
	float WorldTime{};
	RLightData LightData{};

public:
	RScene() = default;
	~RScene() = default;

	
	void SetDelatTime(float DeltaTime)
	{
		this->DeltaTime = DeltaTime;
		WorldTime += DeltaTime;
	}

	float GetDeltaTime() const
	{
		return DeltaTime;
	}

	float GetWorldTime() const
	{
		return WorldTime;
	}

	void SetLightData(const RLightData& InLightData)
	{
		LightData = InLightData;
	}

	const RLightData& GetLightData() const
	{
		return LightData;
	}
};

