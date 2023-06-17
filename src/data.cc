#include <memory>
#include <string.h>
#include "data.h"

FrameData::FrameData(
		GFXHeap *heap, size_t count, uint32_t size,
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

	flags |= GFX_MEMORY_HOST_VISIBLE | GFX_MEMORY_DEVICE_LOCAL;
	group = gfx_alloc_group(heap, flags, usage, count, bindings.get());
	dassert(group);

	// Map first buffer.
	raw = gfx_map(gfx_ref_group_buffer(group, current, 0));
	dassert(raw);
}

FrameData::~FrameData() {
	gfx_unmap(gfx_ref_group_buffer(group, current, 0));
	gfx_free_group(group);
}

uint32_t FrameData::write(const void *data, size_t size) {
	memcpy(((char*)raw) + offset, data, size);

	uint32_t currOffset = offset;
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
	raw = gfx_map(next);
	dassert(raw);
	gfx_unmap(prev);
}

GFXSetResource FrameData::getAsResource(size_t i, size_t binding, size_t index) {
	return GFXSetResource{
		.binding = binding,
		.index = index,
		.ref = gfx_ref_group_buffer(group, i, 0)
	};
}

GFXSetGroup FrameData::getAsGroup(size_t i, size_t binding) {
	return GFXSetGroup{
		.binding = binding,
		.offset = i,
		.numBindings = 0,
		.group = group
	};
}
