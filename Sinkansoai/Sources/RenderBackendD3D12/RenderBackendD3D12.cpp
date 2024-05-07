#include "RenderBackendD3D12.h"

#pragma comment ( lib, "d3d12.lib")
#pragma comment ( lib, "D3DCompiler.lib")
#pragma comment ( lib, "dxgi.lib")

RRenderBackendD3D12::RRenderBackendD3D12()
{
	SetBackendName(TEXT("D3D12"));
}


RRenderBackendD3D12::~RRenderBackendD3D12()
{
}


void RRenderBackendD3D12::Init()
{
	UINT DxgiFactoryFlags = 0;

	TRefCountPtr<IDXGIFactory4> Factory;
	TRefCountPtr<ID3D12Debug> DebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
	{
		DebugController->EnableDebugLayer();
		DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
	
	ThrowIfFailed(CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(&Factory)));

	if (D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&Device) != S_OK)
	{
		ThrowIfFailed(0);
	}
	else
	{
		cout << "D3D12 Renderbackend Device Init Success" << endl;
	}

}

void RRenderBackendD3D12::Exit()
{
	cout << "D3D12 Renderbackend Exit" << endl;
}
