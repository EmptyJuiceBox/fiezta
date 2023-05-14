#pragma once

#include <memory>
#include <vector>
#include "matrix.h"

class GraphNode {
public:
	matrix<float> transform;

	GraphNode() {}

	GraphNode(const matrix<float> &mat) : transform(mat) {}

	GraphNode(const float *mat) : transform(mat) {}

	GraphNode *addChild(std::unique_ptr<GraphNode> node);
	GraphNode *getChild(size_t i);
	void eraseChild(size_t i);
	std::unique_ptr<GraphNode> claimChild(size_t i);

	size_t numChildren(void) { return children.size(); }

private:
	std::vector<std::unique_ptr<GraphNode>> children;
};
