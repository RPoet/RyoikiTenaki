#pragma once
#include "PlatformDefinitions.h"

enum class ESceneAssetType : uint8
{
	Obj,
	Gltf,
	Fbx,
};

struct RSceneAsset
{
	String DisplayName;
	String RootPath;
	String FileName;
	ESceneAssetType Type = ESceneAssetType::Obj;
};

const RSceneAsset& GetSelectedSceneAsset();
void SetSelectedSceneAsset(const RSceneAsset& Asset);
void SelectSceneFromConsole();
