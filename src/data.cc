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

	// Get alignment.
	GFXDevice *device = gfx_heap_get_device(heap);
	align = GFX_MAX(GFX_MAX(
		device->limits.minTexelBufferAlign,
		device->limits.minUniformBufferAlign),
		device->limits.minStorageBufferAlign);

	// Point to first block.
	current = raw;
	offset = 0;
}

FrameData::~FrameData() {
	gfx_unmap(gfx_ref_group(group));
	gfx_free_group(group);
}

uint32_t FrameData::write(const void *data, size_t size) {
	memcpy(((char*)current) + offset, data, size);

	uint32_t currOffset = offset;
	offset += size;

	return currOffset;
}

void FrameData::next() {
	const size_t size = this->size();
	const size_t count = this->count();

	// Get aligned size.
	const size_t alignedSize = GFX_ALIGN_UP(size, align);

	// Get current block.
	size_t currentCount = ((char*)current - (char*)raw) / alignedSize;

	// Get new block.
	currentCount = (currentCount + 1) % count;
	size_t currentOffset = currentCount * alignedSize;

	current = (void*)(((char*)raw) + currentOffset);
	offset = 0;
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
		.numBindings = 1,
		.group = group
	};
}
