#include "graph.h"

GraphNode *GraphNode::addChild(std::unique_ptr<GraphNode> node) {
	children.push_back(std::move(node));
	return children.back().get();
}

GraphNode *GraphNode::getChild(size_t i) {
	return i >= children.size() ? nullptr : children[i].get();
}

void GraphNode::eraseChild(size_t i) {
	if (i < children.size()) {
		if (i < children.size() - 1) {
			children[i] = std::move(children.back());
		}

		children.pop_back();
	}
}

std::unique_ptr<GraphNode> GraphNode::claimChild(size_t i) {
	if (i < children.size()) {
		auto node = std::move(children[i]);
		eraseChild(i);
		return node;
	}

	return {};
}
