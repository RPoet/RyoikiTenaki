#pragma once
#include "../SceneGraphSystem/GraphEntity.h"
#include "Components/Transform.h"

class MPlaceableEntity : public MGraphEntity
{
	CLASS_DECORATOR(MPlaceableEntity)
protected:
	MTransform Transform{};

public:

	MTransform& GetTransform() { return Transform; }
	const MTransform& GetTransform() const { return Transform; }
};
