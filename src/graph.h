#pragma once

#include <memory>
#include <vector>
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

	size_t numChildren(void) { return children.size(); }

private:
	std::vector<std::unique_ptr<GraphNode>> children;
};
