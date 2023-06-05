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

size_t GraphNode::update(GraphNode *parent) {
	finalTransform =
		parent ? parent->finalTransform * transform : transform;

	// If not given a parent, set order of self to 0.
	if (!parent) order = 0;

	size_t nextOrder = order + 1;
	size_t total = 1;

	for (auto &child : children) {
		// Pre-emptively set order of all children.
		child->order = nextOrder;

		size_t childTotal = child->update(this);
		nextOrder += childTotal;
		total += childTotal;
	}

	return total;
}

void GraphNode::copy(void *ptr) {
	_copy(ptr);

	for (auto &child : children)
		child->copy(ptr);
}

void GraphNode::record(GFXRecorder *recorder, unsigned int frame, void *ptr) {
	_record(recorder, frame, ptr);

	for (auto &child : children)
		child->record(recorder, frame, ptr);
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

bool MeshNode::setForward(size_t i, GFXPass *pass, const GFXRenderState *state) {
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

void MeshNode::_record(GFXRecorder *recorder, unsigned int, void*) {
	GFXPass *pass = gfx_recorder_get_pass(recorder);
	if (!pass) return;

	for (auto &prim : primitives)
		if (prim.second.forward.pass == pass)
			gfx_cmd_draw_prim(recorder, &prim.second.forward, 1, 0);
}
