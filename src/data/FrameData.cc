#include <memory>
#include <string.h>
#include "data.h"

FrameData::FrameData(
		GFXHeap *heap, size_t numFrames,
		uint32_t numElements, uint32_t elementSize,
		GFXMemoryFlags flags, GFXBufferUsage usage) {
	GFXDevice* device = gfx_heap_get_device(heap);
	const uint32_t align = GFX_MAX(
		device->limits.minUniformBufferAlign,
		device->limits.minStorageBufferAlign);

	auto bindings = std::make_unique<GFXBinding[]>(numFrames);
	for (size_t b = 0; b < numFrames; ++b) {
		bindings[b] = GFXBinding{
			.type = GFX_BINDING_BUFFER,
			.count = 1,
			.numElements = numElements,
			.elementSize = GFX_ALIGN_UP(elementSize, align),
			.buffers = nullptr
		};
	}

	flags |= GFX_MEMORY_HOST_VISIBLE | GFX_MEMORY_DEVICE_LOCAL;
	group = gfx_alloc_group(heap, flags, usage, numFrames, bindings.get());
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
	ptr = ((char*)raw) + gfx_group_get_binding_offset(group, i % numFrames(), 0);
	offset = 0;
}

void FrameData::write(const void *data, uint32_t offset, size_t size) {
	memcpy(((char*)ptr) + this->offset + offset, data, size);
}

uint32_t FrameData::next() {
	uint32_t currOffset = offset;
	offset += elementSize();

	return currOffset;
}

GFXSetResource FrameData::getAsResource(size_t i, size_t binding, size_t index) {
	return GFXSetResource{
		.binding = binding,
		.index = index,
		.ref = gfx_ref_group_buffer(group, i % numFrames(), 0)
	};
}

GFXSetGroup FrameData::getAsGroup(size_t i, size_t binding) {
	return GFXSetGroup{
		.binding = binding,
		.offset = i % numFrames(),
		.numBindings = 1,
		.group = group
	};
}
