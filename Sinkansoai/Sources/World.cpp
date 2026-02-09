#include "World.h"
#include "Tasks/TaskSystem.h"

#include "Render/Renderer.h"
#include "Module/EditorUI/EditorUI.h"
#include "SceneSelection.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

/*
* Example Usage of the render system command.
* 
	MTaskSystem::Get().AddRenderCommand(TEXT("Scene Init Command"),
	[](RGraphicsCommandList&)
	{
		cout << "Scene Init Command" << endl;
	});
*/

namespace
{
	namespace fs = std::filesystem;

	constexpr int32 kEntityDataVersion = 1;

	fs::path GetEntityDataPath(const RSceneAsset& Asset)
	{
		if (Asset.RootPath.empty())
		{
			return fs::path();
		}

		fs::path RootPath = Asset.RootPath;
		return RootPath / L"entity_data.txt";
	}

	void WriteTransform(std::wofstream& Out, const MTransform& Transform)
	{
		Out << Transform.Position.x << L" "
			<< Transform.Position.y << L" "
			<< Transform.Position.z << L" "
			<< Transform.Rotation.x << L" "
			<< Transform.Rotation.y << L" "
			<< Transform.Rotation.z << L" "
			<< Transform.Scale.x << L" "
			<< Transform.Scale.y << L" "
			<< Transform.Scale.z;
	}

	bool ReadTransform(std::wistringstream& In, MTransform& Transform)
	{
		float Px{};
		float Py{};
		float Pz{};
		float Rx{};
		float Ry{};
		float Rz{};
		float Sx{1.0f};
		float Sy{1.0f};
		float Sz{1.0f};

		if (!(In >> Px >> Py >> Pz >> Rx >> Ry >> Rz >> Sx >> Sy >> Sz))
		{
			return false;
		}

		Transform.SetPosition(Px, Py, Pz);
		Transform.SetRotation(Rx, Ry, Rz);
		Transform.SetScale(Sx, Sy, Sz);
		return true;
	}

	bool TryReadFloat(std::wistringstream& In, float& OutValue)
	{
		return static_cast<bool>(In >> OutValue);
	}

	bool TryReadBool(std::wistringstream& In, bool& OutValue)
	{
		int32 Value = 0;
		if (!(In >> Value))
		{
			return false;
		}
		OutValue = (Value != 0);
		return true;
	}
}

void MWorld::Init()
{
	cout << "World Init" << endl;

	Scene = new RScene();

	Camera.SetExternalObject(true);
	EnsureDefaultLights();
	BuildObjectGraph();
}

void MWorld::Teardown()
{
	ObjectSystem.Teardown();
	SerializableEntities.clear();
	LightEntities.clear();
	ObjectGraph.Reset();
	bEntityDataLoaded = false;

	cout << "World Teardown" << endl;

	if (Scene)
	{
		delete Scene;
	}
}


void MWorld::Tick(float DeltaTime)
{
	TryLoadEntityData();

	TickObjectGraph(DeltaTime);
	Scene->SetDelatTime(DeltaTime);
}

void MWorld::DrawViewport()
{
	float SceneDeltaTime = Scene->GetDeltaTime();

	auto& EditorUI = MEditorUI::Get();
	EditorUI.SetViewContext(Camera.GetViewContext());
	EditorUI.SetObjectGraph(&ObjectGraph);
	EditorUI.SetOnSaveEntities([this]()
		{
			Serialize();
		});
	EditorUI.BeginFrame();
	EditorUI.BuildLayout();
	EditorUI.EndFrame();

	MTaskSystem::Get().AddRenderCommand(TEXT("Update Delta Time"),
		[InScene = Scene, SceneDeltaTime](RGraphicsCommandList& CommandList)
		{
			InScene->SetDelatTime(SceneDeltaTime);
		});

	const RLightData LightData = BuildLightData();
	MTaskSystem::Get().AddRenderCommand(TEXT("Update Light Data"),
		[InScene = Scene, LightData](RGraphicsCommandList& CommandList)
		{
			InScene->SetLightData(LightData);
		});

	MTaskSystem::Get().AddRenderCommand(TEXT("Draw Viewport"),
		[InCamera = Camera, InScene = Scene](RGraphicsCommandList& CommandList)
		{
			//cout << "Draw Viweport Command body" << endl;
			RViewContext ViewContext = const_cast<MCamera&>(InCamera).GetViewContext();

			DrawViweport_RT(CommandList, *InScene, ViewContext);
		});

	// TO DO : Fix this, IMGUI makes PIX dumb. With IMGUI, Pix doesn't capture render frame It only shows how it renders IMGUI.
	MTaskSystem::Get().AddRenderCommand(TEXT("Draw Editor UI"),
		[](RGraphicsCommandList& CommandList)
		{
			MEditorUI::Get().EnqueueRender(CommandList);
		});
}

void MWorld::Serialize()
{
	SaveEntityData();
}

void MWorld::BuildObjectGraph()
{
	ObjectGraph.Reset();
	SerializableEntities.clear();

	const int32 RootIndex = ObjectGraph.AddNode(TEXT("World"), EObjectNodeType::Root, nullptr, -1);
	ObjectGraph.RootIndex = RootIndex;

	ObjectGraph.AddNode(TEXT("EditorCamera"), EObjectNodeType::Camera, &Camera, RootIndex);
	RegisterSerializableEntity(&Camera);

	int32 DirectionalIndex = 0;
	int32 PointIndex = 0;
	int32 SpotIndex = 0;
	for (const auto& Light : LightEntities)
	{
		if (!Light)
		{
			continue;
		}

		String Name = TEXT("Light");
		switch (Light->GetType())
		{
		case ELightType::Directional:
			Name = TEXT("DirectionalLight_") + std::to_wstring(DirectionalIndex++);
			break;
		case ELightType::Point:
			Name = TEXT("PointLight_") + std::to_wstring(PointIndex++);
			break;
		case ELightType::Spot:
			Name = TEXT("SpotLight_") + std::to_wstring(SpotIndex++);
			break;
		}

		ObjectGraph.AddNode(Name, EObjectNodeType::Light, Light.get(), RootIndex);
		RegisterSerializableEntity(Light.get());
	}

	PlaceholderEntity.SetExternalObject(true);
	ObjectGraph.AddNode(TEXT("Entity_0"), EObjectNodeType::Entity, &PlaceholderEntity, RootIndex);
	RegisterSerializableEntity(&PlaceholderEntity);
}

void MWorld::TickObjectGraph(float DeltaTime)
{
	for (const auto& Node : ObjectGraph.Nodes)
	{
		if (!Node.Entity || !Node.Entity->IsTickable())
		{
			continue;
		}

		Node.Entity->Tick(DeltaTime);
	}
}

void MWorld::EnsureDefaultLights()
{
	if (!LightEntities.empty())
	{
		return;
	}

	LightEntities.reserve(5);

	{
		auto Light = std::make_shared<MLightEntity>(ELightType::Directional);
		Light->SetColor({ 1.0f, 1.0f, 1.0f });
		Light->SetIntensity(1.0f);
		Light->GetTransform().SetRotation(55.0f, 0.0f, 0.0f);
		Light->GetTransform().SetPosition(0.0f, 200.0f, 0.0f);
		LightEntities.push_back(Light);
	}

	{
		auto Light = std::make_shared<MLightEntity>(ELightType::Point);
		Light->SetColor({ 1.0f, 0.85f, 0.7f });
		Light->SetIntensity(1.5f);
		Light->SetRange(600.0f);
		Light->GetTransform().SetPosition(300.0f, 200.0f, 0.0f);
		LightEntities.push_back(Light);
	}

	{
		auto Light = std::make_shared<MLightEntity>(ELightType::Point);
		Light->SetColor({ 0.6f, 0.75f, 1.0f });
		Light->SetIntensity(1.2f);
		Light->SetRange(550.0f);
		Light->GetTransform().SetPosition(-300.0f, 200.0f, 100.0f);
		LightEntities.push_back(Light);
	}

	{
		auto Light = std::make_shared<MLightEntity>(ELightType::Point);
		Light->SetColor({ 0.8f, 1.0f, 0.8f });
		Light->SetIntensity(1.0f);
		Light->SetRange(500.0f);
		Light->GetTransform().SetPosition(0.0f, 150.0f, -300.0f);
		LightEntities.push_back(Light);
	}

	{
		auto Light = std::make_shared<MLightEntity>(ELightType::Spot);
		Light->SetColor({ 1.0f, 1.0f, 1.0f });
		Light->SetIntensity(2.0f);
		Light->SetRange(800.0f);
		Light->SetInnerConeAngle(15.0f);
		Light->SetOuterConeAngle(30.0f);
		Light->GetTransform().SetPosition(0.0f, 400.0f, -400.0f);
		Light->GetTransform().SetRotation(-35.0f, 0.0f, 0.0f);
		LightEntities.push_back(Light);
	}
}

RLightData MWorld::BuildLightData() const
{
	RLightData Out{};
	Out.NumPointLights = 0;

	const MLightEntity* DirectionalLight = nullptr;
	for (const auto& Light : LightEntities)
	{
		if (!Light || !Light->IsEnabled())
		{
			continue;
		}

		if (Light->GetType() == ELightType::Directional)
		{
			DirectionalLight = Light.get();
			break;
		}
	}

	if (DirectionalLight)
	{
		const auto Direction = DirectionalLight->GetWorldDirection();
		const float3 Color = DirectionalLight->GetColor();
		const float Intensity = DirectionalLight->GetIntensity();

		Out.DirectionalLight.Direction = float4(Direction.x, Direction.y, Direction.z, 1.0f);
		Out.DirectionalLight.Diffuse = float4(Color.x * Intensity, Color.y * Intensity, Color.z * Intensity, 1.0f);
		Out.DirectionalLight.Specular = float4(Color.x * Intensity, Color.y * Intensity, Color.z * Intensity, 1.0f);
		Out.DirectionalLight.Ambient = float4(Color.x * 0.2f, Color.y * 0.2f, Color.z * 0.2f, 1.0f);
	}

	for (const auto& Light : LightEntities)
	{
		if (!Light || !Light->IsEnabled())
		{
			continue;
		}

		if (Light->GetType() != ELightType::Point)
		{
			continue;
		}

		if (Out.NumPointLights >= 500)
		{
			break;
		}

		auto& Target = Out.PointLights[Out.NumPointLights++];
		const auto& Transform = Light->GetTransform();
		const float3 Color = Light->GetColor();
		const float Intensity = Light->GetIntensity();
		const float Range = Light->GetRange();

		Target.WorldPositionAndIntensity = float4(Transform.Position.x, Transform.Position.y, Transform.Position.z, Range);
		Target.Color = float4(Color.x * Intensity, Color.y * Intensity, Color.z * Intensity, 1.0f);
	}

	return Out;
}

void MWorld::RegisterSerializableEntity(MGraphEntity* Entity)
{
	if (!Entity)
	{
		return;
	}

	auto It = std::find(SerializableEntities.begin(), SerializableEntities.end(), Entity);
	if (It == SerializableEntities.end())
	{
		SerializableEntities.push_back(Entity);
	}
}

void MWorld::TryLoadEntityData()
{
	const auto& Asset = GetSelectedSceneAsset();
	if (Asset.RootPath.empty())
	{
		return;
	}

	if (bEntityDataLoaded && CachedSceneRoot == Asset.RootPath)
	{
		return;
	}

	CachedSceneRoot = Asset.RootPath;
	bEntityDataLoaded = true;

	const fs::path DataPath = GetEntityDataPath(Asset);
	if (DataPath.empty() || !fs::exists(DataPath))
	{
		return;
	}

	std::wifstream InFile(DataPath);
	if (!InFile.is_open())
	{
		return;
	}

	std::wstring Line;
	while (std::getline(InFile, Line))
	{
		if (Line.empty() || Line[0] == L'#')
		{
			continue;
		}

		std::wistringstream LineStream(Line);
		std::wstring Token;
		LineStream >> Token;

		if (Token == L"version")
		{
			int32 Version = 0;
			LineStream >> Version;
			if (Version != kEntityDataVersion)
			{
				return;
			}
		}
		else if (Token == L"entity")
		{
			int32 EntityIndex = -1;
			std::wstring Type;
			LineStream >> EntityIndex >> Type;
			if (EntityIndex < 0 || static_cast<size_t>(EntityIndex) >= SerializableEntities.size())
			{
				continue;
			}

			MGraphEntity* Entity = SerializableEntities[EntityIndex];
			auto* Placeable = dynamic_cast<MPlaceableEntity*>(Entity);
			if (!Placeable)
			{
				continue;
			}

			if (!ReadTransform(LineStream, Placeable->GetTransform()))
			{
				continue;
			}

			if (Type == L"Camera")
			{
				auto* CameraEntity = dynamic_cast<MCamera*>(Entity);
				if (CameraEntity)
				{
					float Fov = CameraEntity->GetFoV();
					if (LineStream >> Fov)
					{
						CameraEntity->SetFov(Fov);
					}
				}
			}
			else if (Type == L"LightDir" || Type == L"LightPoint" || Type == L"LightSpot")
			{
				auto* LightEntity = dynamic_cast<MLightEntity*>(Entity);
				if (!LightEntity)
				{
					continue;
				}

				if (Type == L"LightDir")
				{
					LightEntity->SetType(ELightType::Directional);
				}
				else if (Type == L"LightPoint")
				{
					LightEntity->SetType(ELightType::Point);
				}
				else
				{
					LightEntity->SetType(ELightType::Spot);
				}

				float3 Color = LightEntity->GetColor();
				float Intensity = LightEntity->GetIntensity();
				float Range = LightEntity->GetRange();
				float Inner = LightEntity->GetInnerConeAngle();
				float Outer = LightEntity->GetOuterConeAngle();
				bool bEnabled = LightEntity->IsEnabled();
				bool bAnimate = LightEntity->IsAnimated();

				if (TryReadFloat(LineStream, Color.x) &&
					TryReadFloat(LineStream, Color.y) &&
					TryReadFloat(LineStream, Color.z))
				{
					LightEntity->SetColor(Color);
				}

				if (TryReadFloat(LineStream, Intensity))
				{
					LightEntity->SetIntensity(Intensity);
				}

				if (TryReadFloat(LineStream, Range))
				{
					LightEntity->SetRange(Range);
				}

				if (TryReadFloat(LineStream, Inner))
				{
					LightEntity->SetInnerConeAngle(Inner);
				}

				if (TryReadFloat(LineStream, Outer))
				{
					LightEntity->SetOuterConeAngle(Outer);
				}

				if (TryReadBool(LineStream, bEnabled))
				{
					LightEntity->SetEnabled(bEnabled);
				}

				if (TryReadBool(LineStream, bAnimate))
				{
					LightEntity->SetAnimated(bAnimate);
				}
			}
		}
	}
}

void MWorld::SaveEntityData()
{
	const auto& Asset = GetSelectedSceneAsset();
	if (Asset.RootPath.empty())
	{
		return;
	}

	const fs::path DataPath = GetEntityDataPath(Asset);
	if (DataPath.empty())
	{
		return;
	}

	std::wofstream OutFile(DataPath);
	if (!OutFile.is_open())
	{
		return;
	}

	OutFile << L"# Sinkansoai Entity Data\n";
	OutFile << L"version " << kEntityDataVersion << L"\n";

	for (size_t Index = 0; Index < SerializableEntities.size(); ++Index)
	{
		MGraphEntity* Entity = SerializableEntities[Index];
		if (!Entity)
		{
			continue;
		}

		auto* Placeable = dynamic_cast<MPlaceableEntity*>(Entity);
		if (!Placeable)
		{
			continue;
		}

		std::wstring Type = L"Entity";
		if (dynamic_cast<MCamera*>(Entity))
		{
			Type = L"Camera";
		}
		else if (auto* LightEntity = dynamic_cast<MLightEntity*>(Entity))
		{
			switch (LightEntity->GetType())
			{
			case ELightType::Directional:
				Type = L"LightDir";
				break;
			case ELightType::Point:
				Type = L"LightPoint";
				break;
			case ELightType::Spot:
				Type = L"LightSpot";
				break;
			}
		}

		OutFile << L"entity " << static_cast<int32>(Index) << L" " << Type << L" ";
		WriteTransform(OutFile, Placeable->GetTransform());

		if (Type == L"Camera")
		{
			auto* CameraEntity = static_cast<MCamera*>(Entity);
			OutFile << L" " << CameraEntity->GetFoV();
		}
		else if (Type == L"LightDir" || Type == L"LightPoint" || Type == L"LightSpot")
		{
			auto* LightEntity = static_cast<MLightEntity*>(Entity);
			const float3 Color = LightEntity->GetColor();
			OutFile << L" " << Color.x << L" " << Color.y << L" " << Color.z
				<< L" " << LightEntity->GetIntensity()
				<< L" " << LightEntity->GetRange()
				<< L" " << LightEntity->GetInnerConeAngle()
				<< L" " << LightEntity->GetOuterConeAngle()
				<< L" " << (LightEntity->IsEnabled() ? 1 : 0)
				<< L" " << (LightEntity->IsAnimated() ? 1 : 0);
		}

		OutFile << L"\n";
	}
}
