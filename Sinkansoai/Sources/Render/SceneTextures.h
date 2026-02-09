#pragma once
#include "../PlatformDefinitions.h"
#include "../RenderBackend/Texture.h"

struct RSceneTextures
{
	SharedPtr<TextureResource> SceneDepth;
	SharedPtr<TextureResource> SceneColor;
	SharedPtr<TextureResource> BaseColor;
	SharedPtr<TextureResource> WorldNormal;
	SharedPtr<TextureResource> Material;
	SharedPtr<TextureResource> DebugTexture;

	bool IsInitialized() const
	{
		return SceneDepth && SceneColor && BaseColor && WorldNormal && Material && DebugTexture;
	}
};
