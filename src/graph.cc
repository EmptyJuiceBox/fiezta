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

void GraphNode::record(
		GFXRecorder *recorder, GFXPass *pass,
		unsigned int frame, void *ptr) {
	_record(recorder, pass, frame, ptr);

	for (auto &child : children)
		child->record(recorder, pass, frame, ptr);
}

void MeshNode::addPrimitive(MeshNode::Primitive prim) {
	// Insert empty renderable, i.e. set `pass` to nullptr.
	auto pair = std::make_pair(prim, Renderable{{.pass = nullptr}});
	primitives.push_back(pair);
}

MeshNode::Primitive MeshNode::getPrimitive(size_t i) {
	if (i < primitives.size())
		return primitives[i].first;

	return { nullptr, nullptr };
}

void MeshNode::erasePrimitive(size_t i) {
	if (i < primitives.size())
		primitives.erase(primitives.begin() + i);
}

bool MeshNode::setForward(size_t i, GFXPass* pass, const GFXRenderState* state) {
	if (i < primitives.size()) {
		// Simply set `pass` to nullptr to disable.
		if (!pass) {
			primitives[i].second.forward.pass = nullptr;
			return true;
		}

		// Or re-initialize the renderable.
		return gfx_renderable(
			&primitives[i].second.forward,
			pass, primitives[i].first.tech, primitives[i].first.prim, state);
	}

	return false;
}
