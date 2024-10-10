#include "Renderer.h"
#include "../RenderBackend/RenderBackendCommon.h"

RRenderer::RRenderer(RScene& Scene)
	: Scene(Scene)
{
}

void RRenderer::ResolveViewMatrices()
{

}

void RRenderer::RenderDeferredShading(RRenderCommandList& CommandList)
{
	ResolveViewMatrices();

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
