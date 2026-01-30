#include "Renderer.h"
#include "../RenderBackend/RenderBackendCommon.h"
#include "../Input.h"

#include "../Entities/Components/Transform.h"


MTransform DirectionalLight{};
uint32 DebugInput = 0;
uint32 GNumLights = 1;
bool GUseDeferredShading = 0;
bool GInitPointLights = false;

RLightData LightData{};

RRenderer::RRenderer(RScene& Scene)
	: Scene(Scene)
{
	DirectionalLight.Rotation.x = 55.0f;

	if (!GInitPointLights)
	{
		GInitPointLights = true;

		for (int32 i = 0; i < 500; ++i)
		{
			LightData.PointLights[i].WorldPositionAndIntensity.x = (rand() % 2000) - 1000;
			LightData.PointLights[i].WorldPositionAndIntensity.y = (rand() % 2000) - 1000;
			LightData.PointLights[i].WorldPositionAndIntensity.z = (rand() % 2000) - 1000;

			LightData.PointLights[i].WorldPositionAndIntensity.w = (rand() % 200) + 200;
		}

		for (int32 i = 0; i < 500; ++i)
		{
			LightData.PointLights[i].Color.x = (rand() % 256) / 255.0f;
			LightData.PointLights[i].Color.y = (rand() % 256) / 255.0f;
			LightData.PointLights[i].Color.z = (rand() % 256) / 255.0f;
		}
	}
}


void RRenderer::ResolveViewMatrices()
{
	// First View	
	ViewMatrices[0].ViewToWorldMatrix = ViewContexts[0].LocalToWorld;
	ViewMatrices[0].WorldToViewMatrix = DirectX::XMMatrixInverse(nullptr, ViewContexts[0].LocalToWorld);

	float AspectRatio = (float)ViewContexts[0].ViewRect.x / (float)ViewContexts[0].ViewRect.y;

	ViewMatrices[0].ProjMatrix = DirectX::XMMatrixPerspectiveFovLH(ViewContexts[0].Fov * 3.141592 / 180.0f, AspectRatio, 50000.0f, ViewContexts[0].MinZ);
	ViewMatrices[0].InvProjMatrix = DirectX::XMMatrixInverse(nullptr, ViewMatrices[0].ProjMatrix);

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


	static float KeyTimer = 0;
	KeyTimer += 0.01f;
	if (MInput::Get().IsPressed('6') && KeyTimer > 0.1f)
	{
		KeyTimer = 0;
		GNumLights = 0;
	}

	if (MInput::Get().IsPressed('7') && KeyTimer > 0.1f)
	{
		KeyTimer = 0;
		GNumLights = 150;
	}

	if (MInput::Get().IsPressed('8') && KeyTimer > 0.1f)
	{
		KeyTimer = 0;
		GNumLights = 300;
	}

	if (MInput::Get().IsPressed('9') && KeyTimer > 0.1f)
	{
		KeyTimer = 0;
		GNumLights = 500;
	}

	ViewMatrices[0].DebugValue = DebugInput;

	//DirectionalLight.Rotation.y += Scene.GetDeltaTime() * 45;

	auto Direction = DirectionalLight.GetDirection();
	LightData.DirectionalLight.Direction = float4(Direction.x, Direction.y, Direction.z, 1);
	LightData.DirectionalLight.Diffuse = float4(1, 1, 1, 1);
	LightData.DirectionalLight.Specular = float4(1, 1, 1, 1);
	LightData.DirectionalLight.Ambient = float4(0.2, 0.2, 0.2, 1);

	LightData.NumPointLights = GNumLights;
	for (int32 i = 0; i < GNumLights; ++i)
	{
		LightData.PointLights[i].WorldPositionAndIntensity.y += Scene.GetDeltaTime() * 200;
		if (LightData.PointLights[i].WorldPositionAndIntensity.y > 1000)
		{
			LightData.PointLights[i].WorldPositionAndIntensity.y = -20;
		}
	}
}

void RRenderer::RenderDeferredShading(RGraphicsCommandList& CommandList)
{
	GBackend->RenderBegin();
	ResolveViewMatrices();

	auto GlobalDynamicBuffer0 = GBackend->GetGlobalDynamicBuffer(0); // 0 used for view
	auto GlobalDynamicBuffer1 = GBackend->GetGlobalDynamicBuffer(1); // 1 used for light data

	GlobalDynamicBuffer0->CopyData(ViewMatrices[0]);
	GlobalDynamicBuffer1->CopyData(LightData);

	RenderFrame(*GBackend, CommandList, true, GNumLights);
	GBackend->RenderFinish();
}

void RRenderer::RenderForwardShading(RGraphicsCommandList& CommandList)
{
	GBackend->RenderBegin();
	ResolveViewMatrices();

	auto GlobalDynamicBuffer0 = GBackend->GetGlobalDynamicBuffer(0); // 0 used for view
	auto GlobalDynamicBuffer1 = GBackend->GetGlobalDynamicBuffer(1); // 1 used for light data

	GlobalDynamicBuffer0->CopyData(ViewMatrices[0]);
	GlobalDynamicBuffer1->CopyData(LightData);

	RenderFrame(*GBackend, CommandList, false, GNumLights);
	GBackend->RenderFinish();
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

	auto* Mesh = Backend.GetRenderMesh();
	if (Mesh)
	{
		CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList.SetVertexBuffer(0, Mesh->PositionVertexBuffer);
		CommandList.SetVertexBuffer(1, Mesh->UVVertexBuffer);
		CommandList.SetIndexBuffer(Mesh->IndexBuffer);

		for (int32 iSection = 0; iSection < Mesh->Sections.size(); ++iSection)
		{
			const auto& Section = Mesh->Sections[iSection];
			const auto& Color = Mesh->Materials[Section.MaterialId].Colors.size() > 0 ? Mesh->Materials[Section.MaterialId].Colors[0] : float3(1, 1, 1);
			const int32 NumIndices = Section.End - Section.Start;
			const int32 StartIndex = Section.Start;

			CommandList.SetGraphicsRoot32BitConstants(3, sizeof(Color) / 4, &Color, 0);
			CommandList.SetGraphicsRoot32BitConstant(4, Section.MaterialId, 0);
			CommandList.DrawIndexedInstanced(NumIndices, 1, StartIndex, 0, 0);
		}
	}

	CommandList.EndEvent();
}

void RRenderer::Basepass(RRenderBackend& Backend, RGraphicsCommandList& CommandList)
{
	CommandList.BeginEvent(0xFFFF, TEXT("Basepass"));

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

	auto* Mesh = Backend.GetRenderMesh();
	if (Mesh)
	{
		CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList.SetVertexBuffer(0, Mesh->PositionVertexBuffer);
		CommandList.SetVertexBuffer(1, Mesh->UVVertexBuffer);
		CommandList.SetVertexBuffer(2, Mesh->NormalVertexBuffer);
		CommandList.SetVertexBuffer(3, Mesh->TangentVertexBuffer);
		CommandList.SetVertexBuffer(4, Mesh->BitangentVertexBuffer);
		CommandList.SetIndexBuffer(Mesh->IndexBuffer);

		for (int32 iSection = 0; iSection < Mesh->Sections.size(); ++iSection)
		{
			const auto& Section = Mesh->Sections[iSection];
			const auto& Color = Mesh->Materials[Section.MaterialId].Colors.size() > 0 ? Mesh->Materials[Section.MaterialId].Colors[0] : float3(1, 1, 1);
			const int32 NumIndices = Section.End - Section.Start;
			const int32 StartIndex = Section.Start;

			CommandList.SetGraphicsRoot32BitConstants(3, sizeof(Color) / 4, &Color, 0);
			CommandList.SetGraphicsRoot32BitConstant(4, Section.MaterialId, 0);
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

	auto* Mesh = Backend.GetRenderMesh();
	if (Mesh)
	{
		CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList.SetVertexBuffer(0, Mesh->PositionVertexBuffer);
		CommandList.SetVertexBuffer(1, Mesh->UVVertexBuffer);
		CommandList.SetVertexBuffer(2, Mesh->NormalVertexBuffer);
		CommandList.SetVertexBuffer(3, Mesh->TangentVertexBuffer);
		CommandList.SetVertexBuffer(4, Mesh->BitangentVertexBuffer);
		CommandList.SetIndexBuffer(Mesh->IndexBuffer);

		for (int32 iSection = 0; iSection < Mesh->Sections.size(); ++iSection)
		{
			const auto& Section = Mesh->Sections[iSection];
			const auto& Color = Mesh->Materials[Section.MaterialId].Colors.size() > 0 ? Mesh->Materials[Section.MaterialId].Colors[0] : float3(1, 1, 1);
			const int32 NumIndices = Section.End - Section.Start;
			const int32 StartIndex = Section.Start;

			CommandList.SetGraphicsRoot32BitConstants(3, sizeof(Color) / 4, &Color, 0);
			CommandList.SetGraphicsRoot32BitConstant(4, Section.MaterialId, 0);
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

	{
		ID3D12Resource* SceneColor = Backend.GetSceneTextureResource(ESceneTexture::SceneColor);
		ID3D12Resource* BaseColor = Backend.GetSceneTextureResource(ESceneTexture::BaseColor);
		ID3D12Resource* WorldNormal = Backend.GetSceneTextureResource(ESceneTexture::WorldNormal);
		ID3D12Resource* Material = Backend.GetSceneTextureResource(ESceneTexture::Material);
		ID3D12Resource* SceneDepth = Backend.GetSceneTextureResource(ESceneTexture::SceneDepth);

		D3D12_RESOURCE_BARRIER Barriers[] =
		{
			MakeTransitionBarrier(SceneColor, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
			MakeTransitionBarrier(BaseColor, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
			MakeTransitionBarrier(WorldNormal, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
			MakeTransitionBarrier(Material, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
			MakeTransitionBarrier(SceneDepth, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE)
		};
		CommandList.ResourceBarrier(_countof(Barriers), Barriers);
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
		ID3D12Resource* SceneDepth = Backend.GetSceneTextureResource(ESceneTexture::SceneDepth);
		D3D12_RESOURCE_BARRIER Barriers[] =
		{
			MakeTransitionBarrier(SceneDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ),
		};
		CommandList.ResourceBarrier(_countof(Barriers), Barriers);
	}

	if (bDeferred)
	{
		Basepass(Backend, CommandList);

		{
			ID3D12Resource* BaseColor = Backend.GetSceneTextureResource(ESceneTexture::BaseColor);
			ID3D12Resource* WorldNormal = Backend.GetSceneTextureResource(ESceneTexture::WorldNormal);
			ID3D12Resource* Material = Backend.GetSceneTextureResource(ESceneTexture::Material);
			ID3D12Resource* BackBuffer = Backend.GetBackBufferResource();

			D3D12_RESOURCE_BARRIER Barriers[] =
			{
				MakeTransitionBarrier(BaseColor, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
				MakeTransitionBarrier(WorldNormal, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
				MakeTransitionBarrier(Material, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
				MakeTransitionBarrier(BackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
			};
			CommandList.ResourceBarrier(_countof(Barriers), Barriers);
		}

		RenderLights(Backend, CommandList);
		RenderLocalLights(Backend, CommandList, NumLocalLights);

		{
			ID3D12Resource* SceneColor = Backend.GetSceneTextureResource(ESceneTexture::SceneColor);
			D3D12_RESOURCE_BARRIER Barriers[] =
			{
				MakeTransitionBarrier(SceneColor, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
			};
			CommandList.ResourceBarrier(_countof(Barriers), Barriers);
		}
	}
	else
	{
		RenderForwardLights(Backend, CommandList);

		{
			ID3D12Resource* SceneColor = Backend.GetSceneTextureResource(ESceneTexture::SceneColor);
			ID3D12Resource* BaseColor = Backend.GetSceneTextureResource(ESceneTexture::BaseColor);
			ID3D12Resource* WorldNormal = Backend.GetSceneTextureResource(ESceneTexture::WorldNormal);
			ID3D12Resource* Material = Backend.GetSceneTextureResource(ESceneTexture::Material);
			ID3D12Resource* BackBuffer = Backend.GetBackBufferResource();

			D3D12_RESOURCE_BARRIER Barriers[] =
			{
				MakeTransitionBarrier(SceneColor, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
				MakeTransitionBarrier(BaseColor, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
				MakeTransitionBarrier(WorldNormal, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
				MakeTransitionBarrier(Material, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
				MakeTransitionBarrier(BackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
			};
			CommandList.ResourceBarrier(_countof(Barriers), Barriers);
		}
	}

	Postprocess(Backend, CommandList);

	{
		ID3D12Resource* BackBuffer = Backend.GetBackBufferResource();
		D3D12_RESOURCE_BARRIER Barriers[] =
		{
			MakeTransitionBarrier(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT),
		};
		CommandList.ResourceBarrier(_countof(Barriers), Barriers);
	}

	CommandList.EndEvent();
}

void DrawViweport_RT(RGraphicsCommandList& CommandList, RScene& Scene, const RViewContext& ViewContext)
{
	auto Renderer = new RRenderer(Scene);
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
		delete Renderer;
	}
}
