#pragma once
#include "PlaceableEntity.h"

class MEditorEntity : public MPlaceableEntity
{
	CLASS_DECORATOR(MEditorEntity)

public:
	MEditorEntity() = default;
	virtual ~MEditorEntity() = default;
};
