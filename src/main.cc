#include <math.h>
#include "data.h"
#include "def.h"
#include "graph.h"
#include "mat.h"

void key_press(GFXWindow* window, GFXKey key, int, GFXModifier mod) {
	switch (key) {
	case GFX_KEY_C:
	case GFX_KEY_D:
	case GFX_KEY_Z:
		if (mod & GFX_MOD_CONTROL)
			gfx_window_set_close(window, true);
		break;
	case GFX_KEY_Q:
	case GFX_KEY_ESCAPE:
		gfx_window_set_close(window, true);
	default:
		break;
	}
}

GFXShader *load_shader(GFXShaderStage stage, const char *path) {
	GFXFile file;
	dassert(gfx_file_init(&file, path, "rb"));

	GFXFileIncluder inc;
	dassert(gfx_file_includer_init(&inc, path, "rb"));

	GFXShader *shader = gfx_create_shader(stage, nullptr);
	dassert(shader);

	dassert(gfx_shader_compile(
		shader, GFX_GLSL, 1,
		&file.reader, &inc.includer, nullptr, nullptr));

	gfx_file_includer_clear(&inc);
	gfx_file_clear(&file);

	return shader;
}

GraphNode *load_gltf_node(
		GFXTechnique *tech, GFXPass *pass,
		GraphNode *parent, GFXGltfNode *node) {
	std::unique_ptr<GraphNode> parsed = {};

	if (!node->mesh)
		parsed = std::make_unique<GraphNode>(node->matrix);
	else {
		auto mesh = std::make_unique<MeshNode>(node->matrix);

		for (size_t p = 0; p < node->mesh->numPrimitives; ++p) {
			GFXGltfPrimitive *prim = &node->mesh->primitives[p];
			mesh->addPrimitive(MeshNode::Primitive{tech, prim->primitive});
			mesh->setForward(0, pass, nullptr);
		}

		parsed = std::move(mesh);
	}

	for (size_t n = 0; n < node->numChildren; ++n)
		load_gltf_node(tech, pass, parsed.get(), node->children[n]);

	return parent->addChild(std::move(parsed));
}

std::unique_ptr<GraphNode> load_gltf(
		GFXHeap *heap, GFXDependency *dep,
		GFXTechnique *tech, GFXPass *pass,
		const char *path) {
	GFXFile file;
	dassert(gfx_file_init(&file, path, "rb"));

	GFXFileIncluder inc;
	dassert(gfx_file_includer_init(&inc, path, "rb"));

	const char *attributeOrder[] = {
		"POSITION",
		"NORMAL"
	};

	const GFXGltfOptions opts = {
		.maxAttributes = sizeof(attributeOrder)/sizeof(char*),
		.orderSize = sizeof(attributeOrder)/sizeof(char*),
		.attributeOrder = attributeOrder
	};

	GFXGltfResult result;
	dassert(gfx_load_gltf(
		heap, dep, &opts,
		GFX_IMAGE_ANY_FORMAT, GFX_IMAGE_SAMPLED,
		&file.reader, &inc.includer, &result));

	gfx_file_includer_clear(&inc);
	gfx_file_clear(&file);

	// Convert to graph.
	auto root = std::make_unique<GraphNode>();

	if (result.scene) {
		for (size_t n = 0; n < result.scene->numNodes; ++n)
			load_gltf_node(tech, pass, root.get(), result.scene->nodes[n]);
	}

	gfx_release_gltf(&result);

	return root;
}

struct Context {
	GFXTechnique *tech;
	GraphNode *graph;
};

void render(GFXRecorder *recorder, unsigned int frame, void *ptr) {
	uint32_t width, height, layers;
	gfx_recorder_get_size(recorder, &width, &height, &layers);

	float invAspect = (width != 0) ? (float)height / (float)width : 1.0f;
	mat4<float> viewProj;
	viewProj[0][0] = invAspect;
	viewProj[2][2] = -0.5f;
	viewProj[2][3] = 0.7f;

	const float pi2 = 6.28318530718f;
	static float rot = 0.0f;
	rot = (rot >= pi2 ? rot - pi2 : rot) + 0.01f;
	float hCos = cosf(rot);
	float hSin = sinf(rot);

	mat4<float> model = mat4<float>(
		-0.7f * hCos, 0.7f * hSin, 0.0f, 0.0f,
		 0.0f,        0.0f,        0.7f, 0.0f,
		 0.7f * hSin, 0.7f * hCos, 0.0f, 0.0f,
		 0.0f,        0.0f,        0.0f, 1.0f);

	viewProj *= model;

	Context *ctx = (Context*)ptr;
	gfx_cmd_push(recorder, ctx->tech, 0, sizeof(viewProj.data), viewProj.data);
	ctx->graph->record(recorder, frame, nullptr);
}

int main() {
	dassert(gfx_init());

	GFXWindow *window = gfx_create_window(
		GFX_WINDOW_RESIZABLE | GFX_WINDOW_DOUBLE_BUFFER | GFX_WINDOW_FOCUS,
		nullptr, nullptr, {600, 400, 0}, "fiezta");
	dassert(window);

	window->events.key.press = key_press;

	GFXHeap *heap = gfx_create_heap(nullptr);
	dassert(heap);

	GFXDependency *dep = gfx_create_dep(nullptr, NUM_VIRTUAL_FRAMES);
	dassert(dep);

	GFXRenderer *renderer = gfx_create_renderer(heap, NUM_VIRTUAL_FRAMES);
	dassert(renderer);

	dassert(gfx_renderer_attach_window(renderer, 0, window));

	dassert(gfx_renderer_attach(renderer, 1,
		GFXAttachment{
			.type = GFX_IMAGE_2D,
			.flags = GFX_MEMORY_NONE,
			.usage = GFX_IMAGE_TEST | GFX_IMAGE_TRANSIENT,

			.format = GFX_FORMAT_D16_UNORM,
			.samples = 1,
			.mipmaps = 1,
			.layers = 1,

			.size = GFX_SIZE_RELATIVE,
			.ref = 0,
			.xScale = 1.0f,
			.yScale = 1.0f,
			.zScale = 1.0f
		}));

	GFXPass *pass = gfx_renderer_add_pass(renderer, GFX_PASS_RENDER, 0, nullptr);
	dassert(pass);

	dassert(gfx_pass_consume(
		pass, 0, GFX_ACCESS_ATTACHMENT_WRITE, GFX_STAGE_ANY));
	gfx_pass_clear(
		pass, 0, GFX_IMAGE_COLOR, {{0.0f, 0.0f, 0.0f, 0.0f}});

	dassert(gfx_pass_consume(
		pass, 1, GFX_ACCESS_ATTACHMENT_TEST, GFX_STAGE_ANY));
	gfx_pass_clear(
		pass, 1, GFX_IMAGE_DEPTH, {.test={1.0f}});

	GFXRecorder *recorder = gfx_renderer_add_recorder(renderer);
	dassert(recorder);

	// Load shaders.
	GFXShader *shaders[] = {
		load_shader(GFX_STAGE_VERTEX, "assets/basic.vert"),
		load_shader(GFX_STAGE_FRAGMENT, "assets/basic.frag")
	};

	GFXTechnique *tech = gfx_renderer_add_tech(
		renderer, sizeof(shaders)/sizeof(GFXShader*), shaders);
	dassert(tech);
	dassert(gfx_tech_lock(tech));

	// Load glTF & setup data.
	std::unique_ptr<GraphNode> graph =
		load_gltf(heap, dep, tech, pass, "assets/5t6.gltf");

	dassert(gfx_heap_flush(heap));

	std::unique_ptr<FrameData> data = {};
	const size_t dataCount = graph->writes();

	if (dataCount > 0) {
		data = std::make_unique<FrameData>(
			heap, NUM_VIRTUAL_FRAMES, sizeof(float) * 16 * dataCount,
			GFX_MEMORY_NONE, GFX_BUFFER_UNIFORM);

		for (unsigned int f = 0; f < NUM_VIRTUAL_FRAMES; ++f) {
			GFXSetGroup group = data->getAsGroup(f, 0);
			GFXSet *set = gfx_renderer_add_set(
				renderer, tech, 0,
				0, 1, 0, 0,
				nullptr, &group, nullptr, nullptr);

			graph->assignSet(f, set);
		}
	}

	// Main loop.
	while (!gfx_window_should_close(window)) {
		GFXFrame *frame = gfx_renderer_acquire(renderer);
		gfx_poll_events();

		if (data) {
			graph->update();
			graph->write(data.get());
			data->next();
		}

		Context ctx = Context{ tech, graph.get() };
		gfx_frame_start(frame);
		gfx_pass_inject(pass, 1, ref(gfx_dep_wait(dep)));
		gfx_recorder_render(recorder, pass, render, &ctx);
		gfx_frame_submit(frame);
	}

	// Cleanup.
	gfx_destroy_renderer(renderer);
	data.reset();
	gfx_destroy_heap(heap);
	gfx_destroy_dep(dep);
	gfx_destroy_window(window);

	for (size_t s = 0; s < sizeof(shaders)/sizeof(GFXShader*); ++s)
		gfx_destroy_shader(shaders[s]);

	gfx_terminate();
}
