#pragma once
#include "ObjectBase.h"
#include "SceneGraphSystem/SceneGraphSystem.h"
#include "Entities/Camera.h"

class MWorld : public MObjectBase
{
	CLASS_DECORATOR(MWorld)

private:
	class RScene* Scene{};

	// Add ECS
	MSceneGraphSystem ObjectSystem;

	// Wrap as viewport
	// Viewport Properties
	// Viewport looks should not be in the world.
	// This would be in the Launcher or... other wrapper class.
	MCamera Camera;

public:
	virtual ~MWorld() = default;

	void Init();

	void Teardown();

	void Tick(float DeltaTime);

	virtual void Serialize();
};

