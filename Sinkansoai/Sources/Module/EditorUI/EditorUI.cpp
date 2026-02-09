#include "EditorUI.h"

#include "../../Windows.h"
#include "../../Tasks/TaskSystem.h"
#include "../../RenderBackend/RenderBackendCommon.h"
#include "../../RenderBackend/RenderBackendD3D12/RenderBackendD3D12.h"
#include "../../RenderBackend/RenderBackendD3D12/GraphicsCommandListD3D12.h"
#include "../../SceneSelection.h"
#include "../../Math/SIMDMath.h"
#include "../../ObjectGraph.h"
#include "../../Entities/Camera.h"
#include "../../Entities/EditorEntity.h"
#include "../../Entities/Light.h"
#include "../../Entities/PlaceableEntity.h"

#include "../../ThirdParty/ImGui/imgui.h"
#include "../../ThirdParty/ImGui/backends/imgui_impl_dx12.h"
#include "../../ThirdParty/ImGui/backends/imgui_impl_win32.h"

#include <string>
#include <algorithm>

IMPLEMENT_MODULE(MEditorUI)

static bool ProjectWorldToScreen(const SIMDMath::Matrix4x4& WorldToClip, const uint2& ViewRect, const float3& World, ImVec2& Out)
{
	if (ViewRect.x == 0 || ViewRect.y == 0)
	{
		return false;
	}

	const SIMDMath::Vector4 Clip = SIMDMath::Transform(SIMDMath::Vector4(World.x, World.y, World.z, 1.0f), WorldToClip);
	if (Clip.w == 0.0f)
	{
		return false;
	}

	const float InvW = 1.0f / Clip.w;
	const float NdcX = Clip.x * InvW;
	const float NdcY = Clip.y * InvW;

	const float ScreenX = (NdcX * 0.5f + 0.5f) * static_cast<float>(ViewRect.x);
	const float ScreenY = (1.0f - (NdcY * 0.5f + 0.5f)) * static_cast<float>(ViewRect.y);
	Out = ImVec2(ScreenX, ScreenY);
	return true;
}

void MEditorUI::Init()
{
	Super::Init();
}

void MEditorUI::Teardown()
{
	if (bInitialized)
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		ImGuiSrvHeap.Reset();
		bInitialized = false;
	}

	Super::Teardown();
}

void MEditorUI::InitializeImGui()
{
	if (bInitialized || !GBackend)
	{
		return;
	}

	if (GBackend->GetBackendName() != TEXT("D3D12"))
	{
		return;
	}

	auto* Backend = &RRenderBackendD3D12::Get();
	ID3D12Device* Device = Backend->GetDevice();
	if (!Device)
	{
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	IO.Fonts->AddFontDefault();
	IO.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);

	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HeapDesc.NumDescriptors = 1;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&ImGuiSrvHeap));

	ImGui_ImplWin32_Init(MWindow::Get().GetHWND());
	ImGui_ImplDX12_Init(Device, NumBackBuffers, DXGI_FORMAT_R8G8B8A8_UNORM,
		ImGuiSrvHeap.Get(),
		ImGuiSrvHeap->GetCPUDescriptorHandleForHeapStart(),
		ImGuiSrvHeap->GetGPUDescriptorHandleForHeapStart());

	bInitialized = true;
}

void MEditorUI::BeginFrame()
{
	InitializeImGui();
	if (!bInitialized)
	{
		return;
	}

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void MEditorUI::SetViewContext(const RViewContext& ViewContext)
{
	CachedViewContext = ViewContext;
	bHasViewContext = true;
}

void MEditorUI::SetObjectGraph(MObjectGraph* Graph)
{
	if (CachedObjectGraph != Graph)
	{
		CachedObjectGraph = Graph;
		SelectedNode = -1;
	}
}

void MEditorUI::SetOnSaveEntities(std::function<void()> Callback)
{
	OnSaveEntities = Callback;
}

void MEditorUI::BuildLayout()
{
	if (!bInitialized)
	{
		return;
	}

	auto NarrowString = [](const String& InText)
	{
		return std::string(InText.begin(), InText.end());
	};

	ImGui::Begin("Editor");
	ImGui::Text("Editor Mode");

	const auto& SceneAsset = GetSelectedSceneAsset();
	if (!SceneAsset.DisplayName.empty())
	{
		const auto Name = NarrowString(SceneAsset.DisplayName);
		ImGui::Text("Scene: %s", Name.c_str());
	}
	ImGui::Separator();
	if (ImGui::Button("Save Entities") && OnSaveEntities)
	{
		OnSaveEntities();
	}
	ImGui::Text("UI layout test");
	ImGui::End();

	ImGui::Begin("Outliner");
	if (!CachedObjectGraph || CachedObjectGraph->RootIndex < 0)
	{
		ImGui::Text("No Object Graph");
	}
	else
	{
		auto GetIcon = [&](const MObjectNode& Node)
		{
			switch (Node.Type)
			{
			case EObjectNodeType::Root: return "[W]";
			case EObjectNodeType::Camera: return "[C]";
			case EObjectNodeType::Entity: return "[E]";
			case EObjectNodeType::Light:
			{
				auto* Light = dynamic_cast<MLightEntity*>(Node.Entity);
				if (!Light)
				{
					return "[L]";
				}
				switch (Light->GetType())
				{
				case ELightType::Directional: return "[LD]";
				case ELightType::Point: return "[LP]";
				case ELightType::Spot: return "[LS]";
				}
				return "[L]";
			}
			}
			return "[?]";
		};

		auto DrawNode = [&](auto&& Self, int32 NodeIndex) -> void
		{
			const MObjectNode* Node = CachedObjectGraph->GetNode(NodeIndex);
			if (!Node)
			{
				return;
			}

			const std::string Label = std::string(GetIcon(*Node)) + " " + NarrowString(Node->Name);
			const bool bSelected = (SelectedNode == NodeIndex);
			ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow;
			if (bSelected)
			{
				Flags |= ImGuiTreeNodeFlags_Selected;
			}
			if (Node->Children.empty())
			{
				Flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			}

			const bool bOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(NodeIndex)), Flags, "%s", Label.c_str());
			if (ImGui::IsItemClicked())
			{
				SelectedNode = NodeIndex;
			}

			if (bOpen && !Node->Children.empty())
			{
				for (const int32 ChildIndex : Node->Children)
				{
					Self(Self, ChildIndex);
				}
				ImGui::TreePop();
			}
		};

		DrawNode(DrawNode, CachedObjectGraph->RootIndex);
	}
	ImGui::End();

	ImGui::Begin("Details");
	const MObjectNode* Selected = CachedObjectGraph ? CachedObjectGraph->GetNode(SelectedNode) : nullptr;
	if (!Selected || !Selected->Entity)
	{
		ImGui::Text("Selected: (none)");
		ImGui::End();
		DrawGizmos();
		return;
	}

	const std::string SelectedName = NarrowString(Selected->Name);
	ImGui::Text("Selected: %s", SelectedName.c_str());
	ImGui::Separator();

	auto DrawTransform = [&](MTransform& Transform) -> bool
	{
		bool bChanged = false;
		float3 Position = { Transform.Position.x, Transform.Position.y, Transform.Position.z };
		float3 Rotation = { Transform.Rotation.x, Transform.Rotation.y, Transform.Rotation.z };
		float3 Scale = { Transform.Scale.x, Transform.Scale.y, Transform.Scale.z };

		if (ImGui::DragFloat3("Position", &Position.x, 1.0f))
		{
			Transform.SetPosition(Position.x, Position.y, Position.z);
			bChanged = true;
		}
		if (ImGui::DragFloat3("Rotation", &Rotation.x, 1.0f))
		{
			Transform.SetRotation(Rotation.x, Rotation.y, Rotation.z);
			bChanged = true;
		}
		if (ImGui::DragFloat3("Scale", &Scale.x, 0.1f))
		{
			Transform.SetScale(Scale.x, Scale.y, Scale.z);
			bChanged = true;
		}
		return bChanged;
	};

	if (auto* CameraEntity = dynamic_cast<MCamera*>(Selected->Entity))
	{
		const bool bTransformChanged = DrawTransform(CameraEntity->GetTransform());
		bool bFovChanged = false;
		float Fov = CameraEntity->GetFoV();
		if (ImGui::DragFloat("Fov", &Fov, 1.0f, 10.0f, 170.0f))
		{
			CameraEntity->SetFov(Fov);
			bFovChanged = true;
		}
		if (bTransformChanged || bFovChanged)
		{
			SetViewContext(CameraEntity->GetViewContext());
		}
	}
	else if (auto* LightEntity = dynamic_cast<MLightEntity*>(Selected->Entity))
	{
		auto& Transform = LightEntity->GetTransform();
		float3 Position = { Transform.Position.x, Transform.Position.y, Transform.Position.z };
		if (ImGui::DragFloat3("Position", &Position.x, 1.0f))
		{
			Transform.SetPosition(Position.x, Position.y, Position.z);
		}

		const char* TypeLabels[] = { "Directional", "Point", "Spot" };
		int32 TypeIndex = static_cast<int32>(LightEntity->GetType());
		if (ImGui::Combo("Type", &TypeIndex, TypeLabels, IM_ARRAYSIZE(TypeLabels)))
		{
			LightEntity->SetType(static_cast<ELightType>(TypeIndex));
		}

		if (LightEntity->GetType() == ELightType::Directional)
		{
			float3 WorldRotation = { Transform.Rotation.x, Transform.Rotation.y, Transform.Rotation.z };
			if (ImGui::DragFloat3("World Rotation", &WorldRotation.x, 5.0f))
			{
				Transform.SetRotation(WorldRotation.x, WorldRotation.y, WorldRotation.z);
			}

			const float3 Direction = float3::Normalize(LightEntity->GetWorldDirection());
			ImGui::Text("Direction: %.3f %.3f %.3f", Direction.x, Direction.y, Direction.z);
		}
		else
		{
			float3 Rotation = { Transform.Rotation.x, Transform.Rotation.y, Transform.Rotation.z };
			if (ImGui::DragFloat3("Rotation", &Rotation.x, 5.0f))
			{
				Transform.SetRotation(Rotation.x, Rotation.y, Rotation.z);
			}
		}

		float3 Scale = { Transform.Scale.x, Transform.Scale.y, Transform.Scale.z };
		if (ImGui::DragFloat3("Scale", &Scale.x, 0.1f))
		{
			Transform.SetScale(Scale.x, Scale.y, Scale.z);
		}

		float3 Color = LightEntity->GetColor();
		if (ImGui::ColorEdit3("Color", &Color.x))
		{
			LightEntity->SetColor(Color);
		}

		float Intensity = LightEntity->GetIntensity();
		if (ImGui::DragFloat("Intensity", &Intensity, 0.1f, 0.0f, 100.0f))
		{
			LightEntity->SetIntensity(Intensity);
		}

		if (LightEntity->GetType() == ELightType::Point || LightEntity->GetType() == ELightType::Spot)
		{
			float Range = LightEntity->GetRange();
			if (ImGui::DragFloat("Range", &Range, 1.0f, 0.0f, 5000.0f))
			{
				LightEntity->SetRange(Range);
			}
		}

		if (LightEntity->GetType() == ELightType::Spot)
		{
			float Inner = LightEntity->GetInnerConeAngle();
			float Outer = LightEntity->GetOuterConeAngle();
			if (ImGui::DragFloat("Inner Cone", &Inner, 1.0f, 0.0f, 90.0f))
			{
				LightEntity->SetInnerConeAngle(Inner);
			}
			if (ImGui::DragFloat("Outer Cone", &Outer, 1.0f, 0.0f, 90.0f))
			{
				LightEntity->SetOuterConeAngle(Outer);
			}
		}

		bool bEnabled = LightEntity->IsEnabled();
		if (ImGui::Checkbox("Enabled", &bEnabled))
		{
			LightEntity->SetEnabled(bEnabled);
		}

		bool bAnimate = LightEntity->IsAnimated();
		if (ImGui::Checkbox("Animate", &bAnimate))
		{
			LightEntity->SetAnimated(bAnimate);
		}
	}
	else if (auto* Placeable = dynamic_cast<MPlaceableEntity*>(Selected->Entity))
	{
		DrawTransform(Placeable->GetTransform());
	}

	ImGui::End();

	DrawGizmos();
}

void MEditorUI::EndFrame()
{
	if (!bInitialized)
	{
		return;
	}

	ImGui::Render();
}

void MEditorUI::DrawGizmos()
{
	if (!bHasViewContext || !GBackend)
	{
		return;
	}

	const MObjectNode* Selected = CachedObjectGraph ? CachedObjectGraph->GetNode(SelectedNode) : nullptr;
	if (!Selected || !Selected->Entity)
	{
		return;
	}

	uint2 ViewRect = CachedViewContext.ViewRect;
	if (GBackend && GBackend->GetBackendName() == TEXT("D3D12"))
	{
		const auto* Viewport = RRenderBackendD3D12::Get().GetViewport();
		if (Viewport)
		{
			ViewRect.x = static_cast<uint32>(Viewport->Width);
			ViewRect.y = static_cast<uint32>(Viewport->Height);
		}
	}
	if (ViewRect.x == 0 || ViewRect.y == 0)
	{
		return;
	}

	const float AspectRatio = static_cast<float>(ViewRect.x) / static_cast<float>(ViewRect.y);
	const SIMDMath::Matrix4x4 WorldToView = SIMDMath::Matrix4x4::Inverse(CachedViewContext.LocalToWorld);
	const SIMDMath::Matrix4x4 Proj = SIMDMath::Matrix4x4::PerspectiveFovLH(
		CachedViewContext.Fov * SIMDMath::DegToRad, AspectRatio, 50000.0f, CachedViewContext.MinZ);
	const SIMDMath::Matrix4x4 WorldToClip = WorldToView * Proj;

	float3 BoundsCenter = float3(0, 0, 0);
	float AxisLength = 100.0f;
	const float MaxAxisPixels = static_cast<float>(ViewRect.x) * 0.1f;

	{
		auto Meshes = GBackend->GetRenderMesh();
		bool bHasBounds = false;
		float3 Min{};
		float3 Max{};
		for (auto* Mesh : Meshes)
		{
			if (!Mesh || !Mesh->bHasBounds)
			{
				continue;
			}

			const float3 MeshMin = Mesh->BoundsCenter - Mesh->BoundsExtent * 0.5f;
			const float3 MeshMax = Mesh->BoundsCenter + Mesh->BoundsExtent * 0.5f;
			if (!bHasBounds)
			{
				Min = MeshMin;
				Max = MeshMax;
				bHasBounds = true;
			}
			else
			{
				Min.x = (std::min)(Min.x, MeshMin.x);
				Min.y = (std::min)(Min.y, MeshMin.y);
				Min.z = (std::min)(Min.z, MeshMin.z);
				Max.x = (std::max)(Max.x, MeshMax.x);
				Max.y = (std::max)(Max.y, MeshMax.y);
				Max.z = (std::max)(Max.z, MeshMax.z);
			}
		}

		if (bHasBounds)
		{
			BoundsCenter = (Min + Max) * 0.5f;
			const float3 Extent = Max - Min;
			AxisLength = (std::max)(Extent.x, (std::max)(Extent.y, Extent.z)) * 0.2f;
			if (AxisLength < 1.0f)
			{
				AxisLength = 1.0f;
			}
		}
	}

	float3 LocalOrigin = BoundsCenter;
	if (auto* Camera = dynamic_cast<MCamera*>(Selected->Entity))
	{
		LocalOrigin = Camera->GetTransform().Position;
	}
	else if (auto* Light = dynamic_cast<MLightEntity*>(Selected->Entity))
	{
		LocalOrigin = Light->GetTransform().Position;
	}
	else if (auto* Placeable = dynamic_cast<MPlaceableEntity*>(Selected->Entity))
	{
		LocalOrigin = Placeable->GetTransform().Position;
	}

	auto ClampAxisLengthToScreen = [&](const float3& Origin, float Length)
	{
		if (MaxAxisPixels <= 0.0f)
		{
			return Length;
		}

		ImVec2 O{};
		if (!ProjectWorldToScreen(WorldToClip, ViewRect, Origin, O))
		{
			return Length;
		}

		auto ComputeScreenLength = [&](const float3& Offset)
		{
			ImVec2 P{};
			if (!ProjectWorldToScreen(WorldToClip, ViewRect, Origin + Offset, P))
			{
				return 0.0f;
			}
			const float DX = P.x - O.x;
			const float DY = P.y - O.y;
			return std::sqrt(DX * DX + DY * DY);
		};

		float MaxScreenLen = 0.0f;
		MaxScreenLen = (std::max)(MaxScreenLen, ComputeScreenLength(float3(Length, 0, 0)));
		MaxScreenLen = (std::max)(MaxScreenLen, ComputeScreenLength(float3(0, Length, 0)));
		MaxScreenLen = (std::max)(MaxScreenLen, ComputeScreenLength(float3(0, 0, Length)));

		if (MaxScreenLen <= 0.0f || MaxScreenLen <= MaxAxisPixels)
		{
			return Length;
		}

		const float Scale = MaxAxisPixels / MaxScreenLen;
		return Length * Scale;
	};

	AxisLength = ClampAxisLengthToScreen(LocalOrigin, AxisLength);
	if (AxisLength < 1.0f)
	{
		AxisLength = 1.0f;
	}

	ImDrawList* DrawList = ImGui::GetForegroundDrawList();
	if (!DrawList)
	{
		return;
	}

	auto DrawAxisTripod = [&](const float3& Origin, float Length, float Thickness)
	{
		ImVec2 O, X, Y, Z;
		if (!ProjectWorldToScreen(WorldToClip, ViewRect, Origin, O))
		{
			return;
		}

		if (ProjectWorldToScreen(WorldToClip, ViewRect, Origin + float3(Length, 0, 0), X))
		{
			DrawList->AddLine(O, X, IM_COL32(255, 64, 64, 255), Thickness);
		}
		if (ProjectWorldToScreen(WorldToClip, ViewRect, Origin + float3(0, Length, 0), Y))
		{
			DrawList->AddLine(O, Y, IM_COL32(64, 255, 64, 255), Thickness);
		}
		if (ProjectWorldToScreen(WorldToClip, ViewRect, Origin + float3(0, 0, Length), Z))
		{
			DrawList->AddLine(O, Z, IM_COL32(64, 128, 255, 255), Thickness);
		}
	};

	DrawAxisTripod(float3(0, 0, 0), AxisLength, 2.0f);
	DrawAxisTripod(LocalOrigin, AxisLength * 0.75f, 2.0f);

	{
		const float GizmoScale = (std::min)(28.0f, MaxAxisPixels);
		const ImVec2 Center(45.0f, static_cast<float>(ViewRect.y) - 45.0f);

		auto ToScreen = [&](const float3& Dir)
		{
			float3 N = float3::Normalize(Dir);
			return ImVec2(Center.x + N.x * GizmoScale, Center.y - N.y * GizmoScale);
		};

		const float3 XAxis = SIMDMath::TransformVector(float3(1, 0, 0), WorldToView);
		const float3 YAxis = SIMDMath::TransformVector(float3(0, 1, 0), WorldToView);
		const float3 ZAxis = SIMDMath::TransformVector(float3(0, 0, 1), WorldToView);

		const ImVec2 XPos = ToScreen(XAxis);
		const ImVec2 YPos = ToScreen(YAxis);
		const ImVec2 ZPos = ToScreen(ZAxis);

		DrawList->AddLine(Center, XPos, IM_COL32(255, 64, 64, 255), 2.0f);
		DrawList->AddLine(Center, YPos, IM_COL32(64, 255, 64, 255), 2.0f);
		DrawList->AddLine(Center, ZPos, IM_COL32(64, 128, 255, 255), 2.0f);

		DrawList->AddText(ImVec2(XPos.x + 4.0f, XPos.y - 8.0f), IM_COL32(255, 128, 128, 255), "X");
		DrawList->AddText(ImVec2(YPos.x + 4.0f, YPos.y - 8.0f), IM_COL32(128, 255, 128, 255), "Y");
		DrawList->AddText(ImVec2(ZPos.x + 4.0f, ZPos.y - 8.0f), IM_COL32(128, 160, 255, 255), "Z");
	}

	{
		auto* Placeable = dynamic_cast<MPlaceableEntity*>(Selected->Entity);
		if (Placeable)
		{
			float3 Position = { Placeable->GetTransform().Position.x, Placeable->GetTransform().Position.y, Placeable->GetTransform().Position.z };
			ImVec2 ScreenPos;
			if (ProjectWorldToScreen(WorldToClip, ViewRect, Position, ScreenPos))
			{
				const ImU32 Color = IM_COL32(255, 200, 64, 255);
				DrawList->AddCircleFilled(ScreenPos, 6.0f, Color);
				DrawList->AddCircle(ScreenPos, 9.0f, Color, 12, 2.0f);

				std::string Label = "Selected";
				if (Selected->Type == EObjectNodeType::Camera)
				{
					Label = "[C]";
				}
				else if (Selected->Type == EObjectNodeType::Light)
				{
					Label = "[L]";
					if (auto* Light = dynamic_cast<MLightEntity*>(Selected->Entity))
					{
						switch (Light->GetType())
						{
						case ELightType::Directional: Label = "[LD]"; break;
						case ELightType::Point: Label = "[LP]"; break;
						case ELightType::Spot: Label = "[LS]"; break;
						}
					}
				}
				else if (Selected->Type == EObjectNodeType::Entity)
				{
					Label = "[E]";
				}

				std::string Name(Selected->Name.begin(), Selected->Name.end());
				Label += " " + Name;
				DrawList->AddText(ImVec2(ScreenPos.x + 10.0f, ScreenPos.y - 18.0f), Color, Label.c_str());
			}
		}

		if (auto* Light = dynamic_cast<MLightEntity*>(Selected->Entity))
		{
			if (Light->GetType() == ELightType::Directional)
			{
				const float3 Origin = { Light->GetTransform().Position.x, Light->GetTransform().Position.y, Light->GetTransform().Position.z };
				const float3 Dir = float3::Normalize(Light->GetWorldDirection());
				const float3 End = Origin + Dir * 150.0f;

				ImVec2 ScreenOrigin;
				ImVec2 ScreenEnd;
				if (ProjectWorldToScreen(WorldToClip, ViewRect, Origin, ScreenOrigin) &&
					ProjectWorldToScreen(WorldToClip, ViewRect, End, ScreenEnd))
				{
					const ImU32 DirColor = IM_COL32(255, 220, 128, 255);
					DrawList->AddLine(ScreenOrigin, ScreenEnd, DirColor, 2.0f);

					const ImVec2 ScreenDir = ImVec2(ScreenEnd.x - ScreenOrigin.x, ScreenEnd.y - ScreenOrigin.y);
					const float LenSq = ScreenDir.x * ScreenDir.x + ScreenDir.y * ScreenDir.y;
					if (LenSq > 1.0f)
					{
						const float InvLen = 1.0f / std::sqrt(LenSq);
						const ImVec2 N = ImVec2(ScreenDir.x * InvLen, ScreenDir.y * InvLen);
						const ImVec2 Perp = ImVec2(-N.y, N.x);

						const float HeadLen = 12.0f;
						const float HeadWidth = 6.0f;
						const ImVec2 Tip = ScreenEnd;
						const ImVec2 Base = ImVec2(ScreenEnd.x - N.x * HeadLen, ScreenEnd.y - N.y * HeadLen);
						const ImVec2 Left = ImVec2(Base.x + Perp.x * HeadWidth, Base.y + Perp.y * HeadWidth);
						const ImVec2 Right = ImVec2(Base.x - Perp.x * HeadWidth, Base.y - Perp.y * HeadWidth);

						DrawList->AddTriangleFilled(Tip, Left, Right, DirColor);
					}
				}
			}
		}
	}
}

void MEditorUI::EnqueueRender(RGraphicsCommandList& CommandList)
{
	if (!bInitialized)
	{
		return;
	}

	ID3D12DescriptorHeap* ImGuiHeap = ImGuiSrvHeap.Get();
	auto* Backend = &RRenderBackendD3D12::Get();
	auto* D3D12Cmd = CastAsD3D12<RGraphicsCommandListD3D12>(&CommandList);
	if (!D3D12Cmd)
	{
		return;
	}

	RTexture* BackBuffer = Backend->GetBackBufferResource();
	ResourceBarrier ToRenderTarget = MakeTransitionBarrier(BackBuffer, ResourceState::Present, ResourceState::RenderTarget);
	CommandList.SumbitResourceBarriers(1, &ToRenderTarget);

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = Backend->GetBackBufferRTVHandle();
	CommandList.OMSetRenderTargets(1, &RTVHandle, false, nullptr);

	ID3D12DescriptorHeap* Heaps[] = { ImGuiHeap };
	CommandList.SetDescriptorHeaps(1, Heaps);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), D3D12Cmd->GetRawCommandList());

	ResourceBarrier ToPresent = MakeTransitionBarrier(BackBuffer, ResourceState::RenderTarget, ResourceState::Present);
	CommandList.SumbitResourceBarriers(1, &ToPresent);
}

bool MEditorUI::HandleWin32Message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetCurrentContext() == nullptr)
	{
		return false;
	}

	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	return ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam) != 0;
}
