#pragma once

#include <memory>
#include <utility>
#include <vector>
#include "data.h"
#include "def.h"

class GraphNode {
public:
	mat4<float> transform;

	GraphNode() {}
	GraphNode(const mat4<float> &mat) : transform(mat) {}
	GraphNode(const float *mat) : transform(mat) {}
	virtual ~GraphNode() = default;

	GraphNode *addChild(std::unique_ptr<GraphNode> node);
	GraphNode *getChild(size_t i);
	void eraseChild(size_t i);
	std::unique_ptr<GraphNode> claimChild(size_t i);
	size_t numChildren() { return children.size(); }

	// Update the entire sub-graph.
	void update(GraphNode *parent = nullptr);

	// Write the entire sub-graph to GPU memory.
	void write(FrameData *out);

	// Number of writes this graph makes.
	size_t writes();

	// Assign a (dynamically buffered) set for each frame.
	void assignSet(unsigned int frame, GFXSet*);

	// Record the entire sub-graph.
	void record(GFXRecorder*, unsigned int frame, void *ptr);

protected:
	// args{frame-data-output}
	virtual void _write(FrameData*) {};

	// should return true if _write should be called.
	virtual bool _writes() { return false; }

	// args{frame-index, set}
	virtual void _assignSet(unsigned int, GFXSet*) {};

	// args{recorder, frame-index, user-pointer}
	virtual void _record(GFXRecorder*, unsigned int, void*) {};

	// Set during update().
	mat4<float> finalTransform;

private:
	std::vector<std::unique_ptr<GraphNode>> children;
};


class MeshNode : public GraphNode {
public:
	struct Primitive {
		GFXTechnique *tech;
		GFXPrimitive *prim;
	};

	struct Renderable {
		GFXRenderable forward;
	};

	MeshNode() {}
	MeshNode(const mat4<float> &mat) : GraphNode(mat) {}
	MeshNode(const float *mat) : GraphNode(mat) {}
	virtual ~MeshNode() = default;

	size_t addPrimitive(Primitive prim);
	Primitive getPrimitive(size_t i);
	void erasePrimitive(size_t i);
	size_t numPrimitives() { return primitives.size(); }
	bool setForward(size_t i, GFXPass *pass, const GFXRenderState *state);

protected:
	virtual void _write(FrameData*);
	virtual bool _writes() { return true; }
	virtual void _assignSet(unsigned int, GFXSet*);
	virtual void _record(GFXRecorder*, unsigned int, void*);

private:
	std::vector<std::pair<Primitive, Renderable>> primitives;
	GFXSet *sets[NUM_VIRTUAL_FRAMES];

	// Set during _write().
	uint32_t offset;
};
