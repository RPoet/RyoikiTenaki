#include "SceneSelection.h"

#include <filesystem>
#include <iostream>
#include <cwctype>
#include <algorithm>

namespace fs = std::filesystem;

static RSceneAsset GSelectedSceneAsset{};

const RSceneAsset& GetSelectedSceneAsset()
{
	return GSelectedSceneAsset;
}

void SetSelectedSceneAsset(const RSceneAsset& Asset)
{
	GSelectedSceneAsset = Asset;
}

static bool IsGltfExtension(const fs::path& Path)
{
	auto Ext = Path.extension().wstring();
	for (auto& Ch : Ext)
	{
		Ch = static_cast<wchar_t>(towlower(Ch));
	}
	return Ext == L".gltf" || Ext == L".glb";
}

static bool IsFbxExtension(const fs::path& Path)
{
	auto Ext = Path.extension().wstring();
	for (auto& Ch : Ext)
	{
		Ch = static_cast<wchar_t>(towlower(Ch));
	}
	return Ext == L".fbx";
}

static RSceneAsset BuildAssetFromFile(const fs::path& FilePath)
{
	RSceneAsset Asset{};
	Asset.RootPath = FilePath.parent_path().wstring();
	if (!Asset.RootPath.empty() && Asset.RootPath.back() != L'/' && Asset.RootPath.back() != L'\\')
	{
		Asset.RootPath.push_back(L'/');
	}

	Asset.FileName = FilePath.filename().wstring();
	Asset.DisplayName = FilePath.stem().wstring();
	if (IsGltfExtension(FilePath))
	{
		Asset.Type = ESceneAssetType::Gltf;
	}
	else if (IsFbxExtension(FilePath))
	{
		Asset.Type = ESceneAssetType::Fbx;
	}
	else
	{
		Asset.Type = ESceneAssetType::Obj;
	}
	return Asset;
}

static void AddSceneCandidates(vector<RSceneAsset>& OutAssets, const fs::path& RootPath)
{
	if (!fs::exists(RootPath))
	{
		return;
	}

	for (const auto& Entry : fs::recursive_directory_iterator(RootPath))
	{
		if (Entry.is_regular_file())
		{
			if (IsGltfExtension(Entry.path()) || IsFbxExtension(Entry.path()))
			{
				OutAssets.push_back(BuildAssetFromFile(Entry.path()));
			}
		}
	}
}

void SelectSceneFromConsole()
{
	const fs::path SceneRoot = L"C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Resources/TestScenes";

	vector<RSceneAsset> Candidates;
	Candidates.push_back(RSceneAsset{
		L"Sponza",
		L"C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Resources/sponza/",
		L"sponza.obj",
		ESceneAssetType::Obj
	});
	AddSceneCandidates(Candidates, SceneRoot);

	std::sort(Candidates.begin(), Candidates.end(), [](const RSceneAsset& A, const RSceneAsset& B)
	{
		if (A.DisplayName == B.DisplayName && A.RootPath == B.RootPath && A.FileName == B.FileName)
		{
			return false;
		}
		if (A.DisplayName == L"Sponza" && B.DisplayName == L"Sponza")
		{
			return A.FileName < B.FileName;
		}
		if (A.DisplayName == L"Sponza")
		{
			return true;
		}
		if (B.DisplayName == L"Sponza")
		{
			return false;
		}
		if (A.DisplayName == B.DisplayName)
		{
			if (A.RootPath == B.RootPath)
			{
				return A.FileName < B.FileName;
			}
			return A.RootPath < B.RootPath;
		}
		return A.DisplayName < B.DisplayName;
	});

	cout << "Select scene index:" << endl;
	for (size_t i = 0; i < Candidates.size(); ++i)
	{
		CharString Name(Candidates[i].DisplayName.begin(), Candidates[i].DisplayName.end());
		cout << "  [" << i << "] " << Name << endl;
	}

	cout << "> ";
	int32 SelectedIndex = 0;
	if (!(std::cin >> SelectedIndex))
	{
		SelectedIndex = 0;
	}

	if (SelectedIndex < 0 || static_cast<size_t>(SelectedIndex) >= Candidates.size())
	{
		SelectedIndex = 0;
	}

	SetSelectedSceneAsset(Candidates[SelectedIndex]);
}
