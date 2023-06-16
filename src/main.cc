#include "data.h"
#include "def.h"
#include "graph.h"

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

std::unique_ptr<GraphNode> load_gltf(
		GFXHeap *heap, GFXDependency* dep, const char *path) {
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

	// TODO: Convert to graph.

	gfx_release_gltf(&result);

	return root;
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

	GFXPass *pass = gfx_renderer_add_pass(renderer, GFX_PASS_RENDER, 0, nullptr);
	dassert(pass);

	dassert(gfx_pass_consume(
		pass, 0, GFX_ACCESS_ATTACHMENT_WRITE, GFX_STAGE_ANY));
	gfx_pass_clear(
		pass, 0, GFX_IMAGE_COLOR, {{0.0f, 0.0f, 0.0f, 0.0f}});

	// Load shaders.
	GFXShader *shaders[] = {
		load_shader(GFX_STAGE_VERTEX, "assets/basic.vert"),
		load_shader(GFX_STAGE_FRAGMENT, "assets/basic.frag")
	};

	GFXTechnique *tech = gfx_renderer_add_tech(
		renderer, sizeof(shaders)/sizeof(GFXShader*), shaders);
	dassert(tech);

	// Load glTF.
	std::unique_ptr<GraphNode> graph = load_gltf(heap, dep, "assets/5t6.gltf");

	dassert(gfx_heap_flush(heap));

	// Main loop.
	while (!gfx_window_should_close(window)) {
		GFXFrame *frame = gfx_renderer_acquire(renderer);
		gfx_poll_events();
		gfx_frame_start(frame);
		gfx_pass_inject(pass, 1, ref(gfx_dep_wait(dep)));
		gfx_frame_submit(frame);
	}

	// Termination.
	for (size_t s = 0; s < sizeof(shaders)/sizeof(GFXShader*); ++s)
		gfx_destroy_shader(shaders[s]);

	gfx_destroy_renderer(renderer);
	gfx_destroy_heap(heap);
	gfx_destroy_dep(dep);
	gfx_destroy_window(window);
	gfx_terminate();
}
