#pragma once

#include <memory>
#include <utility>
#include <vector>
#include "def.h"
#include "mat.h"

class GraphNode {
public:
	mat4<float> transform;

	GraphNode() {}

	GraphNode(const mat4<float> &mat) : transform(mat) {}

	GraphNode(const float *mat) : transform(mat) {}

	GraphNode *addChild(std::unique_ptr<GraphNode> node);
	GraphNode *getChild(size_t i);
	void eraseChild(size_t i);
	std::unique_ptr<GraphNode> claimChild(size_t i);

	size_t numChildren() { return children.size(); }

	// Update the entire sub-graph.
	void update(GraphNode *parent = nullptr);

	// Record the entire sub-graph.
	void record(GFXRecorder*, unsigned int frame, void *ptr);

protected:
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

	void addPrimitive(Primitive prim);
	Primitive getPrimitive(size_t i);
	void erasePrimitive(size_t i);

	size_t numPrimitives() { return primitives.size(); }

	bool setForward(size_t i, GFXPass *pass, const GFXRenderState *state);

protected:
	// args{recorder, frame-index, user-pointer}
	virtual void _record(GFXRecorder*, unsigned int, void*);

private:
	std::vector<std::pair<Primitive, Renderable>> primitives;
};
