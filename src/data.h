#pragma once

#include "def.h"

class FrameData {
public:
	FrameData(
		GFXHeap *heap, size_t count, uint64_t size,
		GFXMemoryFlags flags, GFXBufferUsage usage);

	~FrameData();

	size_t count() { return gfx_group_get_num_bindings(group); }
	size_t size() { auto bind = gfx_group_get_binding(group, 0); return bind.elementSize; }
	GFXMemoryFlags flags() { return group->flags; }
	GFXBufferUsage usage() { return group->usage; }

private:
	GFXGroup *group;
};
