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
		if (i < children.size() - 1)
			children[i] = std::move(children.back());

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

void GraphNode::update(GraphNode *parent) {
	finalTransform =
		parent ? parent->finalTransform * transform : transform;

	for (auto &child : children)
		child->update(this);
}

void GraphNode::write(FrameData *out) {
	if (_writes())
		_write(out);

	for (auto &child : children)
		child->write(out);
}

size_t GraphNode::writes() {
	size_t childWrites = 0;

	for (auto &child : children)
		childWrites += child->writes();

	return (_writes() ? 1 : 0) + childWrites;
}

void GraphNode::record(GFXRecorder *recorder, void *ptr) {
	_record(recorder, ptr);

	for (auto &child : children)
		child->record(recorder, ptr);
}
