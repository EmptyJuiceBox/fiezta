#pragma once

#include "def.h"

class FrameData {
public:
	FrameData(
		GFXHeap *heap, size_t count, uint32_t size,
		GFXMemoryFlags flags, GFXBufferUsage usage);

	~FrameData();

	size_t count() { return gfx_group_get_num_bindings(group); }
	size_t size() { return gfx_group_get_binding(group, 0).elementSize; }
	GFXMemoryFlags flags() { return group->flags; }
	GFXBufferUsage usage() { return group->usage; }

	uint32_t write(const void *data, size_t size); // Returns offset of written data.
	void next();

	GFXSetResource getAsResource(size_t i, size_t binding, size_t index);
	GFXSetGroup getAsGroup(size_t i, size_t binding);

private:
	GFXGroup *group;
	size_t current;
	uint32_t offset;
	void *raw;
};
