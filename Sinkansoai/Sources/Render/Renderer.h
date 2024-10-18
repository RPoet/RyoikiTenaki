#pragma once
#include "Scene.h"
#include "View.h"

#include "../RenderBackend/RenderBackend.h"
#include "../RenderBackend/RenderCommandList.h"


class RRenderer
{
private:
	// Manage Scene Primitives;

	RScene& Scene;

	vector< RViewContext > ViewContexts;
	vector< RViewMatrices > ViewMatrices;
	
public:
	RRenderer(RScene& Scene);
	~RRenderer() = default;

	void AddView(const RViewContext& ViewContext)
	{
		ViewContexts.emplace_back(ViewContext);
		ViewMatrices.emplace_back();
	}

	void ResolveViewMatrices();

	void RenderDeferredShading(RRenderCommandList& CommandList);
	void RenderForwardShading(RRenderCommandList& CommandList);
};


void DrawViweport_RT(RRenderCommandList& CommandList, RScene& Scene, const RViewContext& ViewContext);

