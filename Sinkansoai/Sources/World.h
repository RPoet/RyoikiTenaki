#pragma once
#include "ObjectBase.h"

class MWorld : public MObjectBase
{
	CLASS_DECORATOR(MWorld)

private:


	class RScene* Scene{};

	// Add ECS

public:

	void Init();

	void Teardown();

	void Tick(float DeltaTime);
};

