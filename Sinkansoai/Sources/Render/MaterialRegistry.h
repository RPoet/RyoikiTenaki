#pragma once

#include "../Singleton.h"
#include "../PlatformDefinitions.h"
#include "../Engine/Mesh.h"

#include <functional>
#include <unordered_map>

class MaterialRegistry : public Singleton<MaterialRegistry>
{
public:
	void Reset();

	vector<MaterialId> RegisterMaterials(
		vector<Material>& InMaterials,
		const std::function< SharedPtr<TextureResource>(TextureAsset&) >& CreateTexture,
		const SharedPtr<TextureResource>& DefaultTexture,
		const SharedPtr<TextureResource>& DefaultBlackTexture,
		const SharedPtr<TextureResource>& DefaultWhiteTexture,
		const SharedPtr<TextureResource>& DefaultMetalRoughTexture);

	const Material* GetMaterial(MaterialId Id) const;
	uint32 GetSlot(MaterialId Id) const;
	uint32 GetMaterialCount() const;
	uint32 GetTotalTextureCount() const;

	const vector<Material>& GetMaterials() const;

private:
	MaterialId NextId = 1;
	vector<Material> Materials;
	unordered_map<MaterialId, uint32> IdToIndex;
	unordered_map<MaterialId, uint32> IdToSlot;
	uint32 TotalTextureCount = 0;
};
