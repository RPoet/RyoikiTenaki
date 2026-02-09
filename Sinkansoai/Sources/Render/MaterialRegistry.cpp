#include "MaterialRegistry.h"

void MaterialRegistry::Reset()
{
	NextId = 1;
	Materials.clear();
	IdToIndex.clear();
	IdToSlot.clear();
	TotalTextureCount = 0;
}

vector<MaterialId> MaterialRegistry::RegisterMaterials(
	vector<Material>& InMaterials,
	const std::function< SharedPtr<TextureResource>(TextureAsset&) >& CreateTexture,
	const SharedPtr<TextureResource>& DefaultTexture,
	const SharedPtr<TextureResource>& DefaultBlackTexture,
	const SharedPtr<TextureResource>& DefaultWhiteTexture,
	const SharedPtr<TextureResource>& DefaultMetalRoughTexture)
{
	vector<MaterialId> Ids;
	Ids.reserve(InMaterials.size());

	for (auto& InMat : InMaterials)
	{
		Material Mat = std::move(InMat);
		Mat.Id = NextId++;

		Mat.TexturesGPU.clear();

		if (Mat.Textures.size() < kMaterialTextureSlotCount)
		{
			Mat.Textures.resize(kMaterialTextureSlotCount);
		}

		auto CreateOrDefault = [&](TextureType Type, const SharedPtr<TextureResource>& Fallback) -> SharedPtr<TextureResource>
		{
			const auto Index = static_cast<size_t>(Type);
			if (Index < Mat.Textures.size())
			{
				const auto& Source = Mat.Textures[Index];
				if (Source.Width > 0 && Source.Height > 0)
				{
					auto Resource = CreateTexture(Mat.Textures[Index]);
					if (Resource)
					{
						return Resource;
					}
				}
			}
			return Fallback;
		};

		Mat.TexturesGPU.resize(kMaterialTextureSlotCount);
		Mat.TexturesGPU[DIFFUSE] = CreateOrDefault(DIFFUSE, DefaultTexture);
		Mat.TexturesGPU[NORMAL] = CreateOrDefault(NORMAL, DefaultBlackTexture);
		Mat.TexturesGPU[SPECULAR] = CreateOrDefault(SPECULAR, DefaultMetalRoughTexture);
		Mat.TexturesGPU[SHININESS] = CreateOrDefault(SHININESS, DefaultBlackTexture);
		Mat.TexturesGPU[AMBIENT] = CreateOrDefault(AMBIENT, DefaultWhiteTexture);
		Mat.TexturesGPU[EMISSIVE] = CreateOrDefault(EMISSIVE, DefaultBlackTexture);
		Mat.TexturesGPU[REFLECTION] = CreateOrDefault(REFLECTION, DefaultBlackTexture);

		Mat.Textures.clear();

		const uint32 Slot = static_cast<uint32>(Materials.size());
		IdToIndex[Mat.Id] = Slot;
		IdToSlot[Mat.Id] = Slot;
		Materials.emplace_back(std::move(Mat));
		TotalTextureCount += kMaterialTextureSlotCount;
		Ids.push_back(Materials.back().Id);
	}

	InMaterials.clear();
	return Ids;
}

const Material* MaterialRegistry::GetMaterial(MaterialId Id) const
{
	auto Found = IdToIndex.find(Id);
	if (Found == IdToIndex.end())
	{
		return nullptr;
	}
	return &Materials[Found->second];
}

uint32 MaterialRegistry::GetSlot(MaterialId Id) const
{
	auto Found = IdToSlot.find(Id);
	if (Found == IdToSlot.end())
	{
		return 0;
	}
	return Found->second;
}

uint32 MaterialRegistry::GetMaterialCount() const
{
	return static_cast<uint32>(Materials.size());
}

uint32 MaterialRegistry::GetTotalTextureCount() const
{
	return TotalTextureCount;
}

const vector<Material>& MaterialRegistry::GetMaterials() const
{
	return Materials;
}
