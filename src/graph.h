#pragma once

#include <memory>
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

private:
	std::vector<std::unique_ptr<GraphNode>> children;
};


class MeshNode : public GraphNode {
public:
	struct Primitive {
		GFXTechnique* tech;
		GFXPrimitive* prim;
	};

	MeshNode() {}

	MeshNode(const mat4<float> &mat) : GraphNode(mat) {}

	MeshNode(const float* mat) : GraphNode(mat) {}

	void addPrimitive(Primitive prim);
	Primitive getPrimitive(size_t i);
	void erasePrimitive(size_t i);

	size_t numPrimitives() { return primitives.size(); }

private:
	std::vector<Primitive> primitives;
};
