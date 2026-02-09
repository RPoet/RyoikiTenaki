#pragma once

#include "../RenderBackend.h"
#include "../ResourceBuffer.h"

#include "RenderBackendD3D12Common.h"

class RRenderBackendD3D12;
class RVertexBufferD3D12 : public RVertexBuffer
{
protected:
	RRenderBackendD3D12& Backend;
	TRefCountPtr<ID3D12Resource> Resource;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView{};

public:
	RVertexBufferD3D12(RRenderBackendD3D12& Backend, const String& Name)
		: RVertexBuffer(Name)
		, Backend(Backend)
	{}

	virtual void AllocateResource() override;

	virtual void DeallocateResource() override;


	void* GetUnderlyingResource() override final
	{
		return Resource.Get();
	}

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const
	{
		return VertexBufferView;
	}
};


class RIndexBufferD3D12 : public RIndexBuffer
{
protected:
	RRenderBackendD3D12& Backend;
	TRefCountPtr<ID3D12Resource> Resource;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView{};

public:
	RIndexBufferD3D12(RRenderBackendD3D12& Backend, const String& Name)
		: RIndexBuffer(Name)
		, Backend(Backend)
	{}

	virtual void AllocateResource() override;

	virtual void DeallocateResource() override;

	void* GetUnderlyingResource() override final
	{
		return Resource.Get();
	}

	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const
	{
		return IndexBufferView;
	}
};
