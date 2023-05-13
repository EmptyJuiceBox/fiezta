#pragma once

#include <string.h>
#include <memory>
#include <vector>

class GraphNode {
public:
	GraphNode() : GraphNode(nullptr) {};

	GraphNode(const float *transform) {
		const float identity[] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		memcpy(
			this->transform,
			transform ? transform : identity,
			sizeof(this->transform));
	}

	void addChild(std::unique_ptr<GraphNode> node);
	GraphNode *getChild(size_t i);
	void eraseChild(size_t i);
	std::unique_ptr<GraphNode> claimChild(size_t i);

private:
	float transform[16];
	std::vector<std::unique_ptr<GraphNode>> children;
};
