#pragma once

class RScene
{
private:
	// Manage Scene Primitives;


	float DeltaTime{};
	float WorldTime{};

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
};

