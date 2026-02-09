#pragma once

#include "../../Module.h"
#include "../../RenderBackend/RenderBackendD3D12/RenderBackendD3D12Common.h"
#include "../../Render/View.h"

#include <windows.h>
#include <functional>

class RGraphicsCommandList;
struct MObjectGraph;

class MEditorUI : public MModuleBase
{
	MODULE_CLASS_DECORATOR(MEditorUI)

private:
	bool bInitialized = false;
	TRefCountPtr<ID3D12DescriptorHeap> ImGuiSrvHeap;
	bool bHasViewContext = false;
	RViewContext CachedViewContext{};
	MObjectGraph* CachedObjectGraph = nullptr;
	int32 SelectedNode = -1;
	std::function<void()> OnSaveEntities;

	void InitializeImGui();
	void DrawGizmos();

public:
	virtual void Init() override;
	virtual void Teardown() override;

	void SetViewContext(const RViewContext& ViewContext);
	void SetObjectGraph(MObjectGraph* Graph);
	void SetOnSaveEntities(std::function<void()> Callback);

	void BeginFrame();
	void BuildLayout();
	void EndFrame();
	void EnqueueRender(RGraphicsCommandList& CommandList);

	static bool HandleWin32Message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
