#pragma once

#include "../RenderBackend.h"
#include "../ResourceBuffer.h"

#include "RenderBackendD3D12Common.h"
#include "RenderCommandListD3D12.h"

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


	TRefCountPtr<ID3D12Resource>&  GetUnderlyingResource()
	{
		return Resource;
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

	TRefCountPtr<ID3D12Resource>& GetUnderlyingResource()
	{
		return Resource;
	}

	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const
	{
		return IndexBufferView;
	}
};
