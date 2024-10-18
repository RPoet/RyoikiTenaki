#include "DynamicBufferD3D12.h"
#include "RenderBackendD3D12.h"

void RDynamicBufferD3D12::Allocate(const uint32 Size)
{
	const uint32 AlignedSize = (Size + 255u) & (~255u);

	auto HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(AlignedSize);

	ThrowIfFailed(Backend.GetDevice()->CreateCommittedResource(
		&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&BufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&UnderlyingResource)));


	cout << "Buffer creation sucess" << endl;
	this->Size = Size;


	// Map and initialize the constant buffer. We don't unmap this until the
	// app closes. Keeping things mapped for the lifetime of the resource is okay.
	// We do not intend to read from this resource on the CPU.
	CD3DX12_RANGE ReadRange(0, 0);

	ThrowIfFailed(UnderlyingResource->Map(0, &ReadRange, reinterpret_cast<void**>(&Data)));


	this->bInitialized = true;
}


