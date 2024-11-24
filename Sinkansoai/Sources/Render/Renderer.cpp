#include "Renderer.h"
#include "../RenderBackend/RenderBackendCommon.h"
#include "../Input.h"
RRenderer::RRenderer(RScene& Scene)
	: Scene(Scene)
{
}

uint32 DebugInput = 0;

void RRenderer::ResolveViewMatrices()
{
	// First View	
	ViewMatrices[0].ViewToWorldMatrix = ViewContexts[0].LocalToWorld;
	ViewMatrices[0].WorldToViewMatrix = DirectX::XMMatrixInverse(nullptr, ViewContexts[0].LocalToWorld);

	float AspectRatio = (float)ViewContexts[0].ViewRect.x / (float)ViewContexts[0].ViewRect.y;

	ViewMatrices[0].ProjMatrix = DirectX::XMMatrixPerspectiveFovLH(ViewContexts[0].Fov * 3.141592 / 180.0f, AspectRatio, 50000.0f, ViewContexts[0].MinZ);
	ViewMatrices[0].WorldToClip = ViewMatrices[0].WorldToViewMatrix * ViewMatrices[0].ProjMatrix;

	ViewMatrices[0].DeltaTime = Scene.GetDeltaTime();
	ViewMatrices[0].WorldTime = Scene.GetWorldTime();

	ViewMatrices[0].ViewRect = ViewContexts[0].ViewRect;



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

	ViewMatrices[0].DebugValue = DebugInput;
	//cout << Scene.GetWorldTime() << endl;
}

void RRenderer::RenderDeferredShading(RRenderCommandList& CommandList)
{
	ResolveViewMatrices();

	auto GlobalDynamicBuffer = GBackend->GetGlobalDynamicBuffer();
	GlobalDynamicBuffer->CopyData(ViewMatrices[0]);

	GBackend->FunctionalityTestRender();

	//	cout << "Render Deferred Shading" << endl;

}

void RRenderer::RenderForwardShading(RRenderCommandList& CommandList)
{
	cout << "Render Forward Shading" << endl;
}

void DrawViweport_RT(RRenderCommandList& CommandList, RScene& Scene, const RViewContext& ViewContext)
{
	auto Renderer = new RRenderer(Scene);
	Renderer->AddView(ViewContext);

	const bool bDeferredShading = true;

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
