#include <memory>
#include "data.h"

FrameData::FrameData(
		GFXHeap *heap, size_t count, uint64_t size,
		GFXMemoryFlags flags, GFXBufferUsage usage) {
	auto bindings = std::make_unique<GFXBinding[]>(count);
	for (size_t b = 0; b < count; ++b) {
		bindings[b] = GFXBinding{
			.type = GFX_BINDING_BUFFER,
			.count = 1,
			.numElements = 1,
			.elementSize = size,
			.buffers = nullptr
		};
	}

	group = gfx_alloc_group(heap, flags, usage, count, bindings.get());
	dassert(group);
}

FrameData::~FrameData() {
	gfx_free_group(group);
}
