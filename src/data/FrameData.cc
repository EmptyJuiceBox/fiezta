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

	raw = gfx_map(gfx_ref_group(group));
	dassert(raw);

	setOutput(0);
}

FrameData::~FrameData() {
	gfx_unmap(gfx_ref_group(group));
	gfx_free_group(group);
}

void FrameData::setOutput(size_t i) {
	ptr = ((char*)raw) + gfx_group_get_binding_offset(group, i % count(), 0);
	offset = 0;
}

uint32_t FrameData::write(const void *data, size_t size) {
	memcpy(((char*)ptr) + offset, data, size);

	uint32_t currOffset = offset;
	offset += size;

	return currOffset;
}

GFXSetResource FrameData::getAsResource(size_t i, size_t binding, size_t index) {
	return GFXSetResource{
		.binding = binding,
		.index = index,
		.ref = gfx_ref_group_buffer(group, i % count(), 0)
	};
}

GFXSetGroup FrameData::getAsGroup(size_t i, size_t binding) {
	return GFXSetGroup{
		.binding = binding,
		.offset = i % count(),
		.numBindings = 1,
		.group = group
	};
}
