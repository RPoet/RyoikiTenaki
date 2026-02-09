#pragma once
#include "Scene.h"
#include "View.h"
#include "SceneTextures.h"

#include "../RenderBackend/RenderBackend.h"
#include "../RenderBackend/RenderCommandList.h"


class RRenderer
{
private:
	// Manage Scene Primitives;

	RScene& Scene;

	vector<ViewContext> ViewContexts{};
	vector<ViewMatrices> ViewMatrices{};
	RSceneTextures SceneTextures{};

	void EnsureSceneTexturesInitialized(RRenderBackend& Backend);


public:
	RRenderer(RScene& Scene);
	~RRenderer() = default;

	void AddView(const ViewContext& ViewContext)
	{
		ViewContexts.emplace_back(ViewContext);
		ViewMatrices.emplace_back();
	}

	void ResetViews()
	{
		ViewContexts.clear();
		ViewMatrices.clear();
	}

	void ResolveViewMatrices();

	void RenderDeferredShading(RGraphicsCommandList& CommandList);
	void RenderForwardShading(RGraphicsCommandList& CommandList);

	void Prepass(RRenderBackend& Backend, RGraphicsCommandList& CommandList);
	void Basepass(RRenderBackend& Backend, RGraphicsCommandList& CommandList);
	void RenderForwardLights(RRenderBackend& Backend, RGraphicsCommandList& CommandList);
	void RenderLights(RRenderBackend& Backend, RGraphicsCommandList& CommandList);
	void RenderLocalLights(RRenderBackend& Backend, RGraphicsCommandList& CommandList, uint32 NumLocalLight);
	void Postprocess(RRenderBackend& Backend, RGraphicsCommandList& CommandList);
	void RenderFrame(RRenderBackend& Backend, RGraphicsCommandList& CommandList, bool bDeferred, uint32 NumLocalLights);
};


void DrawViweport_RT(RGraphicsCommandList& CommandList, RScene& Scene, const ViewContext& ViewContext);

