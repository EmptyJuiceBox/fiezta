#pragma once

#include "def.h"

class FrameData {
public:
	FrameData(
		GFXHeap *heap, size_t numFrames,
		uint32_t numElements, uint32_t elementSize,
		GFXMemoryFlags flags, GFXBufferUsage usage);

	~FrameData();

	size_t numFrames() { return gfx_group_get_num_bindings(group); }
	size_t numElements() { return gfx_group_get_binding(group, 0).numElements; }
	size_t elementSize() { return gfx_group_get_binding(group, 0).elementSize; }
	GFXMemoryFlags flags() { return group->flags; }
	GFXBufferUsage usage() { return group->usage; }

	void setOutput(size_t i); // Set index to start outputting to.
	void write(const void *data, uint32_t offset, size_t size);
	uint32_t next(); // Returns offset of the now-finished element.

	GFXSetResource getAsResource(size_t i, size_t binding, size_t index);
	GFXSetGroup getAsGroup(size_t i, size_t binding);

private:
	GFXGroup *group;
	void *raw;
	void *ptr;
	uint32_t offset;
};
