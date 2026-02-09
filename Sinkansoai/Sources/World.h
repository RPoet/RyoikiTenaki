#pragma once
#include "ObjectBase.h"
#include "SceneGraphSystem/SceneGraphSystem.h"
#include "Entities/Camera.h"
#include "Entities/EditorEntity.h"
#include "Entities/Light.h"
#include "ObjectGraph.h"
#include "Render/View.h"

#include <memory>

class MWorld : public MObjectBase
{
	CLASS_DECORATOR(MWorld)

private:
	class RScene* Scene{};

	// Add ECS
	MSceneGraphSystem ObjectSystem;

	MObjectGraph ObjectGraph;
	MEditorEntity PlaceholderEntity;
	vector<SharedPtr<MLightEntity>> LightEntities;

	// Wrap as viewport
	// Viewport Properties
	// Viewport looks should not be in the world.
	// It would be better that those are in the Launcher or... other wrapper class.
	MCamera Camera;

	vector<MGraphEntity*> SerializableEntities;
	String CachedSceneRoot;
	bool bEntityDataLoaded = false;

	void BuildObjectGraph();
	void TickObjectGraph(float DeltaTime);
	void EnsureDefaultLights();
	RLightData BuildLightData() const;

	void RegisterSerializableEntity(MGraphEntity* Entity);
	void TryLoadEntityData();
	void SaveEntityData();

public:
	virtual ~MWorld() = default;

	void Init();

	void Teardown();

	void Tick(float DeltaTime);

	void DrawViewport();

	virtual void Serialize();
};

