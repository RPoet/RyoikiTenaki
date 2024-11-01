#include "RenderBackendD3D12.h"
#include "ResourceBufferD3D12.h"

void RVertexBufferD3D12::AllocateResource()
{
	auto HeapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize);

	ThrowIfFailed(Backend.GetDevice()->CreateCommittedResource(
		&HeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Resource)));


	UINT8* pVertexDataBegin;
	CD3DX12_RANGE ReadRange(0, 0);
	ThrowIfFailed(Resource->Map(0, &ReadRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, RawBuffer, VertexBufferSize);
	Resource->Unmap(0, nullptr);

	VertexBufferView.BufferLocation = Resource->GetGPUVirtualAddress();
	VertexBufferView.StrideInBytes = Stride;
	VertexBufferView.SizeInBytes = VertexBufferSize;
}

void RVertexBufferD3D12::DeallocateResource()
{

}

void RIndexBufferD3D12::AllocateResource()
{
	const uint32 IndexBufferSize = IndexBuffer.size() * sizeof(uint32);
	auto HeapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize);

	ThrowIfFailed(Backend.GetDevice()->CreateCommittedResource(
		&HeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Resource)));


	UINT8* pIndexBufer;
	CD3DX12_RANGE ReadRange(0, 0);
	ThrowIfFailed(Resource->Map(0, &ReadRange, reinterpret_cast<void**>(&pIndexBufer)));
	memcpy(pIndexBufer, IndexBuffer.data(), IndexBufferSize);
	Resource->Unmap(0, nullptr);

	IndexBufferView.BufferLocation = Resource->GetGPUVirtualAddress();
	IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	IndexBufferView.SizeInBytes = IndexBufferSize;
}

void RIndexBufferD3D12::DeallocateResource()
{

}
