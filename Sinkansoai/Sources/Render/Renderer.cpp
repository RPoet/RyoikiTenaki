#include "Renderer.h"
#include "../RenderBackend/RenderBackendCommon.h"
#include "../Input.h"

#include "MaterialRegistry.h"


#include "../Engine/Mesh.h"

namespace
{
	constexpr uint32 kMaterialFlagHasMetallicRoughness = 1u << 0;
	constexpr uint32 kMaterialFlagHasSpecular = 1u << 1;
	constexpr uint32 kMaterialFlagHasShininess = 1u << 2;
	constexpr uint32 kMaterialFlagHasAmbient = 1u << 3;
	constexpr uint32 kMaterialFlagHasEmissive = 1u << 4;
}

uint32 DebugInput = 0;
bool GUseDeferredShading = true;

RRenderer::RRenderer(RScene& Scene)
	: Scene(Scene)
{
}

void RRenderer::EnsureSceneTexturesInitialized(RRenderBackend& Backend)
{
	if (SceneTextures.IsInitialized())
	{
		return;
	}

	Backend.InitSceneTextures(SceneTextures);
}

void RRenderer::ResolveViewMatrices()
{
	// First View	
	ViewMatrices[0].ViewToWorldMatrix = ViewContexts[0].LocalToWorld;
	// SIMDMath::Matrix4x4::Inverse
	ViewMatrices[0].WorldToViewMatrix = SIMDMath::Matrix4x4::Inverse(ViewContexts[0].LocalToWorld);

	float AspectRatio = (float)ViewContexts[0].ViewRect.x / (float)ViewContexts[0].ViewRect.y;

	// SIMDMath::Matrix4x4::PerspectiveFovLH
	ViewMatrices[0].ProjMatrix = SIMDMath::Matrix4x4::PerspectiveFovLH(ViewContexts[0].Fov * 3.141592 / 180.0f, AspectRatio, 50000.0f, ViewContexts[0].MinZ);
	ViewMatrices[0].InvProjMatrix = SIMDMath::Matrix4x4::Inverse(ViewMatrices[0].ProjMatrix);

	ViewMatrices[0].WorldToClip = ViewMatrices[0].WorldToViewMatrix * ViewMatrices[0].ProjMatrix;
	ViewMatrices[0].DeltaTime = Scene.GetDeltaTime();
	ViewMatrices[0].WorldTime = Scene.GetWorldTime();
	ViewMatrices[0].ViewRect = ViewContexts[0].ViewRect;
	ViewMatrices[0].ViewTranslation = ViewContexts[0].ViewTranslation;
	
	if (MInput::Get().IsPressed('0'))
	{
		DebugInput = 0;
	}

	if (MInput::Get().IsPressed('1'))
	{
		DebugInput = 1;
	}

	if (MInput::Get().IsPressed('2'))
	{
		DebugInput = 2;
	}

	if (MInput::Get().IsPressed('3'))
	{
		DebugInput = 3;
	}

	if (MInput::Get().IsPressed('4'))
	{
		DebugInput = 4;
	}

	if (MInput::Get().IsPressed('5'))
	{
		DebugInput = 5;
	}

	ViewMatrices[0].DebugValue = DebugInput;
}

void RRenderer::RenderDeferredShading(RGraphicsCommandList& CommandList)
{
	ResolveViewMatrices();

	auto GlobalDynamicBuffer0 = GBackend->GetGlobalDynamicBuffer(0); // 0 used for view
	auto GlobalDynamicBuffer1 = GBackend->GetGlobalDynamicBuffer(1); // 1 used for light data
	const auto& LightData = Scene.GetLightData();

	GlobalDynamicBuffer0->CopyData(ViewMatrices[0]);
	GlobalDynamicBuffer1->CopyData(LightData);

	RenderFrame(*GBackend, CommandList, true, LightData.NumPointLights);
}

void RRenderer::RenderForwardShading(RGraphicsCommandList& CommandList)
{
	ResolveViewMatrices();

	auto GlobalDynamicBuffer0 = GBackend->GetGlobalDynamicBuffer(0); // 0 used for view
	auto GlobalDynamicBuffer1 = GBackend->GetGlobalDynamicBuffer(1); // 1 used for light data
	const auto& LightData = Scene.GetLightData();

	GlobalDynamicBuffer0->CopyData(ViewMatrices[0]);
	GlobalDynamicBuffer1->CopyData(LightData);

	RenderFrame(*GBackend, CommandList, false, LightData.NumPointLights);
}

void RRenderer::Prepass(RRenderBackend& Backend, RGraphicsCommandList& CommandList)
{
	CommandList.BeginEvent(0xFF, TEXT("Prepass"));

	const auto DSVHandle = Backend.GetSceneDepthHandle();
	CommandList.ClearDepthStencilView(DSVHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0, 0, 0, nullptr);
	CommandList.OMSetRenderTargets(0, nullptr, false, &DSVHandle);

	auto* Pipeline = Backend.GetGraphicsPipeline(EGraphicsPipeline::Prepass);
	if (Pipeline)
	{
		CommandList.SetGraphicsPipeline(*Pipeline);
	}

	ID3D12DescriptorHeap* Heaps[] = { Backend.GetCBVSRVHeap() };
	CommandList.SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandList.SetGraphicsRootDescriptorTable(0, Backend.GetDescriptorHandle(EDescriptorHeapAddressSpace::ConstantBufferView));
	CommandList.SetGraphicsRootDescriptorTable(1, Backend.GetDescriptorHandle(EDescriptorHeapAddressSpace::ShaderResourceView));

	auto& Registry = MaterialRegistry::Get();
	auto Meshes = Backend.GetRenderMesh();
	for (int i = 0; i < Meshes.size(); ++i)
	{
		auto& Mesh = Meshes[i];
		if (!Mesh)
		{
			continue;
		}

		CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList.SetVertexBuffer(0, Mesh->PositionVertexBuffer);
		CommandList.SetVertexBuffer(1, Mesh->UVVertexBuffers[0]);
		if (Mesh->GetNumUVChannels() > 1 && Mesh->UVVertexBuffers.size() > 1)
		{
			CommandList.SetVertexBuffer(2, Mesh->UVVertexBuffers[1]);
		}
		CommandList.SetIndexBuffer(Mesh->IndexBuffer);
		
		// TO DO : Mesh must have its name in here so makes below like BeginEvent(0xFF, Mesh->GetName());
		CommandList.BeginEvent(0xFF, TEXT("MeshRender Sections"));
		for (int32 iSection = 0; iSection < Mesh->Sections.size(); ++iSection)
		{
			const auto& Section = Mesh->Sections[iSection];
			const Material* MaterialData = Registry.GetMaterial(Section.MaterialId);
			const auto& Color = (MaterialData && MaterialData->Colors.size() > 0) ? MaterialData->Colors[0] : float3(1, 1, 1);
			const int32 NumIndices = Section.End - Section.Start;
			const int32 StartIndex = Section.Start;
			const uint32 MaxUVIndex = (Mesh->GetNumUVChannels() > 1) ? 1u : 0u;
			const uint32 BaseUVIndex = (MaterialData && MaterialData->BaseColorUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->BaseColorUVIndex : 0u);
			const uint32 NormalUVIndex = (MaterialData && MaterialData->NormalUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->NormalUVIndex : 0u);
			const uint32 MetallicRoughnessUVIndex = (MaterialData && MaterialData->MetallicRoughnessUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->MetallicRoughnessUVIndex : 0u);
			const uint32 AmbientUVIndex = (MaterialData && MaterialData->AmbientUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->AmbientUVIndex : 0u);
			const uint32 EmissiveUVIndex = (MaterialData && MaterialData->EmissiveUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->EmissiveUVIndex : 0u);
			const uint32 ShininessUVIndex = (MaterialData && MaterialData->ShininessUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->ShininessUVIndex : 0u);
			uint32 MaterialFlags = 0;
			if (MaterialData)
			{
				if (MaterialData->bMetallicRoughness) MaterialFlags |= kMaterialFlagHasMetallicRoughness;
				if (MaterialData->bSpecular) MaterialFlags |= kMaterialFlagHasSpecular;
				if (MaterialData->bShininess) MaterialFlags |= kMaterialFlagHasShininess;
				if (MaterialData->bAmbient) MaterialFlags |= kMaterialFlagHasAmbient;
				if (MaterialData->bEmissive) MaterialFlags |= kMaterialFlagHasEmissive;
			}
			const uint32 MaterialSlot = Registry.GetSlot(Section.MaterialId);

			const float ColorData[3] = { Color.x, Color.y, Color.z };
			CommandList.SetGraphicsRoot32BitConstants(2, 3, ColorData, 0);
			const uint32 MaterialConstants[kMaterialConstantCount] =
			{
				MaterialSlot,
				BaseUVIndex,
				NormalUVIndex,
				MetallicRoughnessUVIndex,
				AmbientUVIndex,
				EmissiveUVIndex,
				ShininessUVIndex,
				MaterialFlags
			};
			CommandList.SetGraphicsRoot32BitConstants(3, kMaterialConstantCount, MaterialConstants, 0);
			CommandList.DrawIndexedInstanced(NumIndices, 1, StartIndex, 0, 0);
		}
		CommandList.EndEvent();
	}
	
	CommandList.EndEvent();
}

void RRenderer::Basepass(RRenderBackend& Backend, RGraphicsCommandList& CommandList)
{
	CommandList.BeginEvent(0xFFFF, TEXT("Basepass"));

	// TO DO : Move to scene texture
	const uint32 SceneRTVCount = Backend.GetSceneRTVCount();
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandles[8] = {};
	for (uint32 i = 0; i < SceneRTVCount && i < _countof(RTVHandles); ++i)
	{
		RTVHandles[i] = Backend.GetSceneRTVHandle(i);
	}

	const auto DSVHandle = Backend.GetSceneDepthHandle();

	const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (uint32 i = 0; i < SceneRTVCount && i < _countof(RTVHandles); ++i)
	{
		CommandList.ClearRenderTargetView(RTVHandles[i], ClearColor, 0, nullptr);
	}

	CommandList.OMSetRenderTargets(SceneRTVCount, RTVHandles, false, &DSVHandle);

	auto* Pipeline = Backend.GetGraphicsPipeline(EGraphicsPipeline::Basepass);
	if (Pipeline)
	{
		CommandList.SetGraphicsPipeline(*Pipeline);
	}

	ID3D12DescriptorHeap* Heaps[] = { Backend.GetCBVSRVHeap() };
	CommandList.SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandList.SetGraphicsRootDescriptorTable(0, Backend.GetDescriptorHandle(EDescriptorHeapAddressSpace::ConstantBufferView));
	CommandList.SetGraphicsRootDescriptorTable(1, Backend.GetDescriptorHandle(EDescriptorHeapAddressSpace::ShaderResourceView));

	auto& Registry = MaterialRegistry::Get();
	auto Meshes = Backend.GetRenderMesh();
	for (int i = 0; i < Meshes.size(); ++i)
	{
		auto* Mesh = Meshes[i];
		if (!Mesh)
		{
			continue;
		}
		CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList.SetVertexBuffer(0, Mesh->PositionVertexBuffer);
		CommandList.SetVertexBuffer(1, Mesh->UVVertexBuffers[0]);
		uint32 NextSlot = 2;
		if (Mesh->GetNumUVChannels() > 1 && Mesh->UVVertexBuffers.size() > 1)
		{
			CommandList.SetVertexBuffer(2, Mesh->UVVertexBuffers[1]);
			NextSlot = 3;
		}
		CommandList.SetVertexBuffer(NextSlot + 0, Mesh->NormalVertexBuffer);
		CommandList.SetVertexBuffer(NextSlot + 1, Mesh->TangentVertexBuffer);
		CommandList.SetVertexBuffer(NextSlot + 2, Mesh->BitangentVertexBuffer);
		CommandList.SetIndexBuffer(Mesh->IndexBuffer);

		for (int32 iSection = 0; iSection < Mesh->Sections.size(); ++iSection)
		{
			const auto& Section = Mesh->Sections[iSection];
			const Material* MaterialData = Registry.GetMaterial(Section.MaterialId);
			const auto& Color = (MaterialData && MaterialData->Colors.size() > 0) ? MaterialData->Colors[0] : float3(1, 1, 1);
			const int32 NumIndices = Section.End - Section.Start;
			const int32 StartIndex = Section.Start;
			const uint32 MaxUVIndex = (Mesh->GetNumUVChannels() > 1) ? 1u : 0u;
			const uint32 BaseUVIndex = (MaterialData && MaterialData->BaseColorUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->BaseColorUVIndex : 0u);
			const uint32 NormalUVIndex = (MaterialData && MaterialData->NormalUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->NormalUVIndex : 0u);
			const uint32 MetallicRoughnessUVIndex = (MaterialData && MaterialData->MetallicRoughnessUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->MetallicRoughnessUVIndex : 0u);
			const uint32 AmbientUVIndex = (MaterialData && MaterialData->AmbientUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->AmbientUVIndex : 0u);
			const uint32 EmissiveUVIndex = (MaterialData && MaterialData->EmissiveUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->EmissiveUVIndex : 0u);
			const uint32 ShininessUVIndex = (MaterialData && MaterialData->ShininessUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->ShininessUVIndex : 0u);
			uint32 MaterialFlags = 0;
			if (MaterialData)
			{
				if (MaterialData->bMetallicRoughness) MaterialFlags |= kMaterialFlagHasMetallicRoughness;
				if (MaterialData->bSpecular) MaterialFlags |= kMaterialFlagHasSpecular;
				if (MaterialData->bShininess) MaterialFlags |= kMaterialFlagHasShininess;
				if (MaterialData->bAmbient) MaterialFlags |= kMaterialFlagHasAmbient;
				if (MaterialData->bEmissive) MaterialFlags |= kMaterialFlagHasEmissive;
			}
			const uint32 MaterialSlot = Registry.GetSlot(Section.MaterialId);

			const float ColorData[3] = { Color.x, Color.y, Color.z };
			CommandList.SetGraphicsRoot32BitConstants(2, 3, ColorData, 0);
			const uint32 MaterialConstants[kMaterialConstantCount] =
			{
				MaterialSlot,
				BaseUVIndex,
				NormalUVIndex,
				MetallicRoughnessUVIndex,
				AmbientUVIndex,
				EmissiveUVIndex,
				ShininessUVIndex,
				MaterialFlags
			};
			CommandList.SetGraphicsRoot32BitConstants(3, kMaterialConstantCount, MaterialConstants, 0);
			CommandList.DrawIndexedInstanced(NumIndices, 1, StartIndex, 0, 0);
		}
	}

	CommandList.EndEvent();
}

void RRenderer::RenderForwardLights(RRenderBackend& Backend, RGraphicsCommandList& CommandList)
{
	CommandList.BeginEvent(0xFFFF, TEXT("Render Forward Lights"));

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandles[] =
	{
		Backend.GetSceneRTVHandle(0),
	};

	const auto DSVHandle = Backend.GetSceneDepthHandle();

	const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	CommandList.ClearRenderTargetView(RTVHandles[0], ClearColor, 0, nullptr);

	CommandList.OMSetRenderTargets(_countof(RTVHandles), RTVHandles, false, &DSVHandle);

	auto* Pipeline = Backend.GetGraphicsPipeline(EGraphicsPipeline::ForwardLighting);
	if (Pipeline)
	{
		CommandList.SetGraphicsPipeline(*Pipeline);
	}

	ID3D12DescriptorHeap* Heaps[] = { Backend.GetCBVSRVHeap() };
	CommandList.SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandList.SetGraphicsRootDescriptorTable(0, Backend.GetDescriptorHandle(EDescriptorHeapAddressSpace::ConstantBufferView));
	CommandList.SetGraphicsRootDescriptorTable(1, Backend.GetDescriptorHandle(EDescriptorHeapAddressSpace::ShaderResourceView));

	auto& Registry = MaterialRegistry::Get();
	auto Meshes = Backend.GetRenderMesh();
	for (int i = 0; i < Meshes.size(); ++i)
	{
		auto* Mesh = Meshes[i];
		if (!Mesh)
		{
			continue;
		}
		CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList.SetVertexBuffer(0, Mesh->PositionVertexBuffer);
		CommandList.SetVertexBuffer(1, Mesh->UVVertexBuffers[0]);
		uint32 NextSlot = 2;
		if (Mesh->GetNumUVChannels() > 1 && Mesh->UVVertexBuffers.size() > 1)
		{
			CommandList.SetVertexBuffer(2, Mesh->UVVertexBuffers[1]);
			NextSlot = 3;
		}
		CommandList.SetVertexBuffer(NextSlot + 0, Mesh->NormalVertexBuffer);
		CommandList.SetVertexBuffer(NextSlot + 1, Mesh->TangentVertexBuffer);
		CommandList.SetVertexBuffer(NextSlot + 2, Mesh->BitangentVertexBuffer);
		CommandList.SetIndexBuffer(Mesh->IndexBuffer);

		for (int32 iSection = 0; iSection < Mesh->Sections.size(); ++iSection)
		{
			const auto& Section = Mesh->Sections[iSection];
			const Material* MaterialData = Registry.GetMaterial(Section.MaterialId);
			const auto& Color = (MaterialData && MaterialData->Colors.size() > 0) ? MaterialData->Colors[0] : float3(1, 1, 1);
			const int32 NumIndices = Section.End - Section.Start;
			const int32 StartIndex = Section.Start;
			const uint32 MaxUVIndex = (Mesh->GetNumUVChannels() > 1) ? 1u : 0u;
			const uint32 BaseUVIndex = (MaterialData && MaterialData->BaseColorUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->BaseColorUVIndex : 0u);
			const uint32 NormalUVIndex = (MaterialData && MaterialData->NormalUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->NormalUVIndex : 0u);
			const uint32 MetallicRoughnessUVIndex = (MaterialData && MaterialData->MetallicRoughnessUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->MetallicRoughnessUVIndex : 0u);
			const uint32 AmbientUVIndex = (MaterialData && MaterialData->AmbientUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->AmbientUVIndex : 0u);
			const uint32 EmissiveUVIndex = (MaterialData && MaterialData->EmissiveUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->EmissiveUVIndex : 0u);
			const uint32 ShininessUVIndex = (MaterialData && MaterialData->ShininessUVIndex > MaxUVIndex) ? 0u : (MaterialData ? MaterialData->ShininessUVIndex : 0u);
			uint32 MaterialFlags = 0;
			if (MaterialData)
			{
				if (MaterialData->bMetallicRoughness) MaterialFlags |= kMaterialFlagHasMetallicRoughness;
				if (MaterialData->bSpecular) MaterialFlags |= kMaterialFlagHasSpecular;
				if (MaterialData->bShininess) MaterialFlags |= kMaterialFlagHasShininess;
				if (MaterialData->bAmbient) MaterialFlags |= kMaterialFlagHasAmbient;
				if (MaterialData->bEmissive) MaterialFlags |= kMaterialFlagHasEmissive;
			}

			const uint32 MaterialSlot = Registry.GetSlot(Section.MaterialId);

			const float ColorData[3] = { Color.x, Color.y, Color.z };
			CommandList.SetGraphicsRoot32BitConstants(2, 3, ColorData, 0);
			const uint32 MaterialConstants[kMaterialConstantCount] =
			{
				MaterialSlot,
				BaseUVIndex,
				NormalUVIndex,
				MetallicRoughnessUVIndex,
				AmbientUVIndex,
				EmissiveUVIndex,
				ShininessUVIndex,
				MaterialFlags
			};
			CommandList.SetGraphicsRoot32BitConstants(3, kMaterialConstantCount, MaterialConstants, 0);
			CommandList.DrawIndexedInstanced(NumIndices, 1, StartIndex, 0, 0);
		}
	}

	CommandList.EndEvent();
}

void RRenderer::RenderLights(RRenderBackend& Backend, RGraphicsCommandList& CommandList)
{
	CommandList.BeginEvent(0xFFFF, TEXT("RenderLights"));

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandles[] =
	{
		Backend.GetSceneRTVHandle(0),
		Backend.GetSceneRTVHandle(4),
	};

	const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (auto& RTV : RTVHandles)
	{
		CommandList.ClearRenderTargetView(RTV, ClearColor, 0, nullptr);
	}

	CommandList.OMSetRenderTargets(_countof(RTVHandles), RTVHandles, false, nullptr);

	auto* Pipeline = Backend.GetGraphicsPipeline(EGraphicsPipeline::DeferredLighting);
	if (Pipeline)
	{
		CommandList.SetGraphicsPipeline(*Pipeline);
	}

	ID3D12DescriptorHeap* Heaps[] = { Backend.GetCBVSRVHeap() };
	CommandList.SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandList.SetGraphicsRootDescriptorTable(0, Backend.GetDescriptorHandle(EDescriptorHeapAddressSpace::ConstantBufferView));
	CommandList.SetGraphicsRootDescriptorTable(1, Backend.GetSceneTextureGPUHandle());

	CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList.DrawInstanced(3, 1, 0, 0);

	CommandList.EndEvent();
}

void RRenderer::RenderLocalLights(RRenderBackend& Backend, RGraphicsCommandList& CommandList, uint32 NumLocalLight)
{
	CommandList.BeginEvent(0xFFFF, TEXT("RenderLocalLights"));

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandles[] =
	{
		Backend.GetSceneRTVHandle(0),
		Backend.GetSceneRTVHandle(4),
	};

	const auto DSVHandle = Backend.GetSceneDepthHandle();

	CommandList.OMSetRenderTargets(_countof(RTVHandles), RTVHandles, false, &DSVHandle);

	auto* Pipeline = Backend.GetGraphicsPipeline(EGraphicsPipeline::DeferredLocalLighting);
	if (Pipeline)
	{
		CommandList.SetGraphicsPipeline(*Pipeline);
	}

	ID3D12DescriptorHeap* Heaps[] = { Backend.GetCBVSRVHeap() };
	CommandList.SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandList.SetGraphicsRootDescriptorTable(0, Backend.GetDescriptorHandle(EDescriptorHeapAddressSpace::ConstantBufferView));
	CommandList.SetGraphicsRootDescriptorTable(1, Backend.GetSceneTextureGPUHandle());

	auto* Mesh = Backend.GetLightVolumeMesh();
	if (Mesh)
	{
		CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList.SetVertexBuffer(0, Mesh->PositionVertexBuffer);
		CommandList.DrawInstanced(Mesh->NumVertices, NumLocalLight, 0, 0);
	}

	CommandList.EndEvent();
}

void RRenderer::Postprocess(RRenderBackend& Backend, RGraphicsCommandList& CommandList)
{
	CommandList.BeginEvent(0xFFFF, TEXT("Postprocess"));

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandles[] =
	{
		Backend.GetBackBufferRTVHandle(),
	};

	const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	CommandList.ClearRenderTargetView(RTVHandles[0], ClearColor, 0, nullptr);
	CommandList.OMSetRenderTargets(1, RTVHandles, false, nullptr);

	auto* Pipeline = Backend.GetGraphicsPipeline(EGraphicsPipeline::Postprocess);
	if (Pipeline)
	{
		CommandList.SetGraphicsPipeline(*Pipeline);
	}

	ID3D12DescriptorHeap* Heaps[] = { Backend.GetCBVSRVHeap() };
	CommandList.SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandList.SetGraphicsRootDescriptorTable(0, Backend.GetDescriptorHandle(EDescriptorHeapAddressSpace::ConstantBufferView));
	CommandList.SetGraphicsRootDescriptorTable(1, Backend.GetSceneTextureGPUHandle());

	CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList.DrawInstanced(3, 1, 0, 0);

	CommandList.EndEvent();
}

void RRenderer::RenderFrame(RRenderBackend& Backend, RGraphicsCommandList& CommandList, bool bDeferred, uint32 NumLocalLights)
{
	CommandList.BeginEvent(0xFFFFFFFF, TEXT("RenderFrame"));

	EnsureSceneTexturesInitialized(Backend);

	{
		// Scene Texture should be reside in Renderer.
		RTexture* SceneColor = SceneTextures.SceneColor.get();
		RTexture* BaseColor = SceneTextures.BaseColor.get();
		RTexture* WorldNormal = SceneTextures.WorldNormal.get();
		RTexture* Material = SceneTextures.Material.get();
		RTexture* SceneDepth = SceneTextures.SceneDepth.get();

		ResourceBarrier Barriers[] =
		{
			MakeTransitionBarrier(SceneColor, ResourceState::GenericRead, ResourceState::RenderTarget),
			MakeTransitionBarrier(BaseColor, ResourceState::GenericRead, ResourceState::RenderTarget),
			MakeTransitionBarrier(WorldNormal, ResourceState::GenericRead, ResourceState::RenderTarget),
			MakeTransitionBarrier(Material, ResourceState::GenericRead, ResourceState::RenderTarget),
			MakeTransitionBarrier(SceneDepth, ResourceState::DepthRead, ResourceState::DepthWrite)
		};
		CommandList.SumbitResourceBarriers(_countof(Barriers), Barriers);
	}

	const D3D12_VIEWPORT* Viewport = Backend.GetViewport();
	if (Viewport)
	{
		CommandList.SetViewports(1, Viewport);
	}
	const D3D12_RECT* Scissor = Backend.GetScissorRect();
	if (Scissor)
	{
		CommandList.SetScissorRects(1, Scissor);
	}

	Prepass(Backend, CommandList);

	{
		RTexture* SceneDepth = SceneTextures.SceneDepth.get();
		ResourceBarrier Barriers[] =
		{
			MakeTransitionBarrier(SceneDepth, ResourceState::DepthWrite, ResourceState::DepthRead),
		};
		CommandList.SumbitResourceBarriers(_countof(Barriers), Barriers);
	}

	if (bDeferred)
	{
		Basepass(Backend, CommandList);

		{
			RTexture* BaseColor = SceneTextures.BaseColor.get();
			RTexture* WorldNormal = SceneTextures.WorldNormal.get();
			RTexture* Material = SceneTextures.Material.get();

			RTexture* BackBuffer = Backend.GetBackBufferResource();

			ResourceBarrier Barriers[] =
			{
				MakeTransitionBarrier(BaseColor, ResourceState::RenderTarget, ResourceState::GenericRead),
				MakeTransitionBarrier(WorldNormal, ResourceState::RenderTarget, ResourceState::GenericRead),
				MakeTransitionBarrier(Material, ResourceState::RenderTarget, ResourceState::GenericRead),
				MakeTransitionBarrier(BackBuffer, ResourceState::Present, ResourceState::RenderTarget),
			};
			CommandList.SumbitResourceBarriers(_countof(Barriers), Barriers);
		}

		RenderLights(Backend, CommandList);
		RenderLocalLights(Backend, CommandList, NumLocalLights);

		{
			RTexture* SceneColor = SceneTextures.SceneColor.get();
			ResourceBarrier Barriers[] =
			{
				MakeTransitionBarrier(SceneColor, ResourceState::RenderTarget, ResourceState::GenericRead),
			};
			CommandList.SumbitResourceBarriers(_countof(Barriers), Barriers);
		}
	}
	else
	{
		RenderForwardLights(Backend, CommandList);

		{
			RTexture* SceneColor = SceneTextures.SceneColor.get();
			RTexture* BaseColor = SceneTextures.BaseColor.get();
			RTexture* WorldNormal = SceneTextures.WorldNormal.get();
			RTexture* Material = SceneTextures.Material.get();
			RTexture* BackBuffer = Backend.GetBackBufferResource();

			ResourceBarrier Barriers[] =
			{
				MakeTransitionBarrier(SceneColor, ResourceState::RenderTarget, ResourceState::GenericRead),
				MakeTransitionBarrier(BaseColor, ResourceState::RenderTarget, ResourceState::GenericRead),
				MakeTransitionBarrier(WorldNormal, ResourceState::RenderTarget, ResourceState::GenericRead),
				MakeTransitionBarrier(Material, ResourceState::RenderTarget, ResourceState::GenericRead),
				MakeTransitionBarrier(BackBuffer, ResourceState::Present, ResourceState::RenderTarget),
			};
			CommandList.SumbitResourceBarriers(_countof(Barriers), Barriers);
		}
	}

	Postprocess(Backend, CommandList);

	{
		RTexture* BackBuffer = Backend.GetBackBufferResource();
		ResourceBarrier Barriers[] =
		{
			MakeTransitionBarrier(BackBuffer, ResourceState::RenderTarget, ResourceState::Present),
		};
		CommandList.SumbitResourceBarriers(_countof(Barriers), Barriers);
	}

	CommandList.EndEvent();
}

void DrawViweport_RT(RGraphicsCommandList& CommandList, RScene& Scene, const ViewContext& ViewContext)
{
	static RRenderer* Renderer = nullptr;
	static RScene* CachedScene = nullptr;
	if (!Renderer || CachedScene != &Scene)
	{
		if (Renderer)
		{
			delete Renderer;
		}
		Renderer = new RRenderer(Scene);
		CachedScene = &Scene;
	}

	Renderer->ResetViews();
	Renderer->AddView(ViewContext);

	static float KeyTimer = 0;
	KeyTimer += 0.01f;
	if (MInput::Get().IsPressed('K') && KeyTimer > 0.1f)
	{
		KeyTimer = 0;
		GUseDeferredShading = !GUseDeferredShading;
	}

	const bool bDeferredShading = GUseDeferredShading;

	if (bDeferredShading)
	{
		Renderer->RenderDeferredShading(CommandList);
	}
	else
	{
		Renderer->RenderForwardShading(CommandList);
	}

	if (Renderer)
	{
		// keep renderer alive to preserve GPU resources
	}
}
