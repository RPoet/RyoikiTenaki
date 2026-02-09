#pragma once
#include "RenderResource.h"

class RVertexBuffer : public RRenderResource
{
protected:
	uint32 VertexBufferSize = 0;
	uint32 Stride = 0;
	uint8* RawBuffer = nullptr;

public:
	RVertexBuffer(const String& Name)
		: RRenderResource(Name)
	{}

	RVertexBuffer(String&& Name)
		: RRenderResource(std::move(Name))
	{}

	virtual ~RVertexBuffer()
	{
		RelaseRawBuffer();
	}

	void RelaseRawBuffer()
	{
		if (RawBuffer)
		{
			delete[] RawBuffer;
		}

		RawBuffer = nullptr;
		VertexBufferSize = 0;
	}

	uint32 GetRawVertexBufferSize() const
	{
		return VertexBufferSize;
	}

	template <class T>
	void SetRawVertexBuffer( const vector<T>& RawVertexBuffer )
	{
		RelaseRawBuffer();

		Stride = sizeof(T);

		uint32 InSize = Stride * RawVertexBuffer.size();
		RawBuffer = new uint8[InSize];

		memcpy(RawBuffer, RawVertexBuffer.data(), InSize);

		VertexBufferSize = InSize;
	}

	const uint8* GetRawVertexBuffer() const
	{
		return RawBuffer;
	}
};


class RIndexBuffer : public RRenderResource
{
protected:
	using IndexType = uint32;

	vector< IndexType > IndexBuffer;

public:

	RIndexBuffer(const String& Name)
		: RRenderResource(Name)
	{}

	RIndexBuffer(String&& Name)
		: RRenderResource(std::move(Name))
	{}

	uint32 GetIndexBufferSize() const
	{
		return sizeof(IndexType) * IndexBuffer.size();
	}

	void SetIndexBuffer(const vector< IndexType >& InIndexBuffer)
	{
		IndexBuffer = InIndexBuffer;
	}

	const vector<IndexType>& GetRawIndexBuffer()
	{
		return IndexBuffer;
	}

	uint32 GetNumIndices() const
	{
		return IndexBuffer.size();
	}

};
