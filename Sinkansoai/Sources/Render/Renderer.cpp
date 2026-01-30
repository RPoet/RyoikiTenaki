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

	GBackend->FunctionalityTestRender(true, GNumLights);
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

	GBackend->FunctionalityTestRender(false, GNumLights);
	GBackend->RenderFinish();
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
