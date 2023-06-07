#include <memory>
#include <string.h>
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

	flags |= GFX_MEMORY_HOST_VISIBLE;
	group = gfx_alloc_group(heap, flags, usage, count, bindings.get());
	dassert(group);

	current = 0;
	offset = 0;
	raw = nullptr;
}

FrameData::~FrameData() {
	gfx_free_group(group);
}

uint64_t FrameData::write(const void *data, size_t size) {
	memcpy(((char*)raw) + offset, data, size);

	uint64_t currOffset = offset;
	offset += size;

	return currOffset;
}

void FrameData::next() {
	GFXBufferRef prev = gfx_ref_group_buffer(group, current, 0);

	// Update indices into the group buffer.
	current = (current + 1) % count();
	offset = 0;

	GFXBufferRef next = gfx_ref_group_buffer(group, current, 0);

	// Map next first then unmap previous.
	void *newRaw = gfx_map(next);
	dassert(newRaw);

	if (raw) gfx_unmap(prev);
	raw = newRaw;
}
