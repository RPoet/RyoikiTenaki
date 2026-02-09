#pragma once

#include "../../PlatformDefinitions.h"

#include <array>
#include <cstddef>

template<uint32 ScratchPadSize>
class MemoryScratchPad
{
private:
	using MemoryResource = pmr_memory_resource;

	// Custom the memory size to fit usage of command list.
	alignas(std::max_align_t) std::array<std::byte, ScratchPadSize> Buffer{};

	pmr_memory_resource* FallbackHeap;
	pmr_monotonic_buffer_resource BufferResource;

public:
	MemoryScratchPad()
		: FallbackHeap(pmr_new_delete_resource())
		, BufferResource(Buffer.data(), Buffer.size(), FallbackHeap)
	{
	}

	~MemoryScratchPad()
	{
		BufferResource.release();
	}

	void Release()
	{
		BufferResource.release();
	}

	pmr_monotonic_buffer_resource& operator()()
	{
		return BufferResource;
	}
};

using BackendCommandListMemoryScratchPad = MemoryScratchPad<8192>;
