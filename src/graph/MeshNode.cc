#include <string.h>
#include "graph.h"

size_t MeshNode::addPrimitive(MeshNode::Primitive prim) {
	// Insert empty renderable, i.e. set `pass` to nullptr.
	auto pair = std::make_pair(prim, Renderable{{.pass = nullptr}});
	primitives.push_back(pair);

	return primitives.size() - 1;
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

bool MeshNode::assignSets(size_t i, GFXSet **sets) {
	if (i < primitives.size()) {
		memcpy(primitives[i].second.sets, sets, sizeof(primitives[i].second.sets));
		return true;
	}

	return false;
}

void MeshNode::_write(FrameData *out) {
	out->write(finalTransform.data, 0, sizeof(finalTransform.data));
	offset = out->next();
}

void MeshNode::_record(GFXRecorder *recorder, unsigned int frame, void*) {
	GFXPass *pass = gfx_recorder_get_pass(recorder);
	if (!pass) return;

	for (auto &prim : primitives)
		if (prim.second.forward.pass == pass) {
			gfx_cmd_bind(
				recorder, prim.first.tech,
				0, 1, 1, &prim.second.sets[frame], &offset);
			gfx_cmd_draw_prim(
				recorder, &prim.second.forward, 1, 0);
		}
}
