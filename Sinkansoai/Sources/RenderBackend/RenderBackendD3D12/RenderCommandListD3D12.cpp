#include "RenderCommandListD3D12.h"
#include "RenderBackendD3D12.h"

void RRenderCommandListD3D12::AllocateCommandLsit(RRenderBackendD3D12& Backend)
{
	auto Device = Backend.Device;

	{
		HRESULT HR = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator));
		verify(HR);
	}

	{
		HRESULT HR = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList));
		verify(HR);
	}

	ThrowIfFailed( CommandList->Close() );
	
	cout << "Command List Allocation Success" << endl;

	bClose = false;
}

RRenderCommandListD3D12::~RRenderCommandListD3D12()
{
	if (CommandList)
	{
		CommandList->Release();
	}
	CommandList = nullptr;


	if (CommandAllocator)
	{
		CommandAllocator->Release();
	}
	CommandAllocator = nullptr;


	cout << "Command List Deleted" << endl;
}

void RRenderCommandListD3D12::Reset()
{
	verify(CommandAllocator->Reset());
	verify(CommandList->Reset(CommandAllocator, nullptr));
}

void RRenderCommandListD3D12::Close()
{
	verify(CommandList->Close());
}
