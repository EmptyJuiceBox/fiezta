#include <math.h>
#include "data.h"
#include "def.h"
#include "graph.h"

struct Input {
	bool left;
	bool right;
	bool forward;
	bool back;
	bool up;
	bool down;

	vec2<double> mouse[2];
};

void key_press(GFXWindow *window, GFXKey key, int, GFXModifier mod) {
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
		break;
	default:
		break;
	}

	Input *inp = (Input*)window->ptr;
	switch (key) {
	case GFX_KEY_A:
	case GFX_KEY_LEFT:
		inp->left = true;
		break;
	case GFX_KEY_D:
	case GFX_KEY_RIGHT:
		inp->right = true;
		break;
	case GFX_KEY_W:
	case GFX_KEY_UP:
		inp->forward = true;
		break;
	case GFX_KEY_S:
	case GFX_KEY_DOWN:
		inp->back = true;
		break;
	case GFX_KEY_SPACE:
		inp->up = true;
		break;
	case GFX_KEY_LEFT_SHIFT:
		inp->down = true;
		break;
	default:
		break;
	}
}

void key_release(GFXWindow *window, GFXKey key, int, GFXModifier) {
	switch (key) {
	case GFX_KEY_F11:
		if (gfx_window_get_monitor(window) != nullptr) {
			gfx_window_set_monitor(
				window, nullptr, {600, 400, 0});
		} else {
			GFXMonitor* monitor = gfx_get_primary_monitor();
			gfx_window_set_monitor(
				window, monitor,
				gfx_monitor_get_current_mode(monitor));
		}
		break;
	default:
		break;
	}

	Input *inp = (Input*)window->ptr;
	switch (key) {
	case GFX_KEY_A:
	case GFX_KEY_LEFT:
		inp->left = false;
		break;
	case GFX_KEY_D:
	case GFX_KEY_RIGHT:
		inp->right = false;
		break;
	case GFX_KEY_W:
	case GFX_KEY_UP:
		inp->forward = false;
		break;
	case GFX_KEY_S:
	case GFX_KEY_DOWN:
		inp->back = false;
		break;
	case GFX_KEY_SPACE:
		inp->up = false;
		break;
	case GFX_KEY_LEFT_SHIFT:
		inp->down = false;
		break;
	default:
		break;
	}
}

void mouse_move(GFXWindow *window, double x, double y) {
	Input *inp = (Input*)window->ptr;
	inp->mouse[1] = inp->mouse[0];
	inp->mouse[0] = vec2<double>(x, y);
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
		GFXTechnique *tech, GFXPass *pass, GFXSet **sets,
		GraphNode *parent, GFXGltfNode *node) {
	const auto matrix = mat4<float>(node->matrix).transpose(); // Column -> row major.
	std::unique_ptr<GraphNode> parsed = {};

	if (!node->mesh)
		parsed = std::make_unique<GraphNode>(matrix);
	else {
		auto mesh = std::make_unique<MeshNode>(matrix);

		for (size_t p = 0; p < node->mesh->numPrimitives; ++p) {
			GFXGltfPrimitive *prim = &node->mesh->primitives[p];
			size_t i = mesh->addPrimitive(MeshNode::Primitive{tech, prim->primitive});
			dassert(mesh->setForward(i, pass, nullptr));
			dassert(mesh->assignSets(i, sets));
		}

		parsed = std::move(mesh);
	}

	for (size_t n = 0; n < node->numChildren; ++n)
		load_gltf_node(
			tech, pass, sets,
			parsed.get(), node->children[n]);

	return parent->addChild(std::move(parsed));
}

std::unique_ptr<GraphNode> load_gltf(
		GFXHeap *heap, GFXDependency *dep,
		GFXTechnique *tech, GFXPass *pass, GFXSet **sets,
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
			load_gltf_node(
				tech, pass, sets,
				root.get(), result.scene->nodes[n]);
	}

	gfx_release_gltf(&result);

	return root;
}

struct Camera {
	vec3<float> pos;
	float pitch;
	float yaw;
};

struct Context {
	GFXTechnique *tech;
	GraphNode *graph;
	Camera cam;
};

void render(GFXRecorder *recorder, unsigned int frame, void *ptr) {
	Context *ctx = (Context*)ptr;

	uint32_t width, height, layers;
	gfx_recorder_get_size(recorder, &width, &height, &layers);

	const float pi2 = 6.28318530718f;
	const float vertFov = pi2 / 4.0f;
	const float aspect = (height != 0) ? (float)width / (float)height : 1.0f;
	const float near = 0.01f;
	const float far = 100.0f;
	const float focalLen = 1.0f / tanf(vertFov / 2.0f);

	const float x = focalLen / aspect;
	const float y = -focalLen;
	const float A = near / (far - near);
	const float B = far * A;

	const auto projection = mat4<float>(
		x,    0.0f,  0.0f, 0.0f,
		0.0f, y,     0.0f, 0.0f,
		0.0f, 0.0f,  A,    B,
		0.0f, 0.0f, -1.0f, 0.0f);

	const float cosPitch = cosf(-ctx->cam.pitch);
	const float sinPitch = sinf(-ctx->cam.pitch);

	const auto camPitch = mat4<float>(
		1.0f, 0.0f,      0.0f,     0.0f,
		0.0f, cosPitch, -sinPitch, 0.0f,
		0.0f, sinPitch,  cosPitch, 0.0f,
		0.0f, 0.0f,      0.0f,     1.0f);

	const float cosYaw = cosf(-ctx->cam.yaw);
	const float sinYaw = sinf(-ctx->cam.yaw);

	const auto camYaw = mat4<float>(
		 cosYaw, 0.0f, sinYaw, 0.0f,
		 0.0f,   1.0f, 0.0f,   0.0f,
		-sinYaw, 0.0f, cosYaw, 0.0f,
		 0.0f,   0.0f, 0.0f,   1.0f);

	const auto camPos = mat4<float>(
		1.0f, 0.0f, 0.0f, -ctx->cam.pos[0],
		0.0f, 1.0f, 0.0f, -ctx->cam.pos[1],
		0.0f, 0.0f, 1.0f, -ctx->cam.pos[2],
		0.0f, 0.0f, 0.0f,  1.0f);

	const mat4<float> viewProj = projection * camPitch * camYaw * camPos;

	gfx_cmd_push(recorder, ctx->tech, 0, sizeof(viewProj.data), viewProj.data);
	ctx->graph->record(recorder, frame, nullptr);
}

int main() {
	dassert(gfx_init());

	GFXWindow *window = gfx_create_window(
		GFX_WINDOW_RESIZABLE | GFX_WINDOW_CAPTURE_MOUSE |
		GFX_WINDOW_DOUBLE_BUFFER | GFX_WINDOW_FOCUS,
		nullptr, nullptr, {600, 400, 0}, "fiezta");
	dassert(window);

	Input input = {
		.left = false,
		.right = false,
		.forward = false,
		.back = false,
		.up = false,
		.down = false,
		.mouse = {vec2<double>(),vec2<double>()}
	};

	window->ptr = &input;
	window->events.key.press = key_press;
	window->events.key.release = key_release;
	window->events.mouse.move = mouse_move;

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

	GFXDepthState depth = {
		GFX_DEPTH_WRITE, GFX_CMP_GREATER}; // Reverse depth.
	GFXRasterState raster = {
		GFX_RASTER_FILL,
		GFX_FRONT_FACE_CCW, GFX_CULL_BACK,
		GFX_TOPO_TRIANGLE_LIST, 1};

	gfx_pass_set_state(pass, GFXRenderState{&raster, nullptr, &depth, nullptr});

	dassert(gfx_pass_consume(
		pass, 0, GFX_ACCESS_ATTACHMENT_WRITE, GFX_STAGE_ANY));
	gfx_pass_clear(
		pass, 0, GFX_IMAGE_COLOR, {{0.0f, 0.0f, 0.0f, 0.0f}});

	dassert(gfx_pass_consume(
		pass, 1, GFX_ACCESS_ATTACHMENT_TEST, GFX_STAGE_ANY));
	gfx_pass_clear(
		pass, 1, GFX_IMAGE_DEPTH, {.test={0.0f}});

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
	dassert(gfx_tech_dynamic(tech, 0, 0));
	dassert(gfx_tech_lock(tech));

	GFXSet *sets[NUM_VIRTUAL_FRAMES];
	for (unsigned int f = 0; f < NUM_VIRTUAL_FRAMES; ++f) {
		sets[f] = gfx_renderer_add_set(
			renderer, tech, 0,
			0, 0, 0, 0,
			nullptr, nullptr, nullptr, nullptr);
		dassert(sets[f]);
	}

	// Load glTF & setup data.
	std::unique_ptr<GraphNode> graph =
		load_gltf(heap, dep, tech, pass, sets, "assets/5t6.gltf");

	dassert(gfx_heap_flush(heap));

	std::unique_ptr<FrameData> data = {};
	const size_t dataCount = graph->writes();

	if (dataCount > 0) {
		data = std::make_unique<FrameData>(
			heap, NUM_VIRTUAL_FRAMES, sizeof(float) * 16 * dataCount,
			GFX_MEMORY_NONE, GFX_BUFFER_UNIFORM);

		for (unsigned int f = 0; f < NUM_VIRTUAL_FRAMES; ++f) {
			GFXSetGroup group = data->getAsGroup(f, 0);
			dassert(gfx_set_groups(sets[f], 1, &group));
		}
	}

	// Main loop.
	Context ctx = {
		.tech = tech,
		.graph = graph.get(),
		.cam = {vec3<float>(0.0f, 0.0f, 2.0f), 0.0f, 0.0f}
	};

	gfx_poll_events(); // Init mouse pos.

	while (!gfx_window_should_close(window)) {
		GFXFrame *frame = gfx_renderer_acquire(renderer);
		gfx_frame_start(frame);

		// Update input.
		input.mouse[1] = input.mouse[0];
		gfx_poll_events();

		// Move camera.
		const float pi2 = 6.28318530718f;
		const float pi4 = pi2 / 4.0f - 0.01f;

		const vec2<double> mouseVel = input.mouse[0] - input.mouse[1];
		ctx.cam.yaw += -(mouseVel[0] / 60);
		ctx.cam.pitch = GFX_CLAMP(ctx.cam.pitch - (mouseVel[1] / 60), -pi4, pi4);

		const float cosPitch = cosf(ctx.cam.pitch);
		const float sinPitch = sinf(ctx.cam.pitch);

		const auto camPitch = mat4<float>(
			1.0f, 0.0f,      0.0f,     0.0f,
			0.0f, cosPitch, -sinPitch, 0.0f,
			0.0f, sinPitch,  cosPitch, 0.0f,
			0.0f, 0.0f,      0.0f,     1.0f);

		const float cosYaw = cosf(ctx.cam.yaw);
		const float sinYaw = sinf(ctx.cam.yaw);

		const auto camYaw = mat4<float>(
			 cosYaw, 0.0f, sinYaw, 0.0f,
			 0.0f,   1.0f, 0.0f,   0.0f,
			-sinYaw, 0.0f, cosYaw, 0.0f,
			 0.0f,   0.0f, 0.0f,   1.0f);

		const vec3<float> forward =
			camYaw * camPitch * vec3<float>(0.0f, 0.0f, -1.0f);
		const vec3<float> right =
			forward.cross(vec3<float>(0.0f, 1.0f, 0.0f)).normalize();

		const double moveSpeed = 0.01;
		if (input.left)
			ctx.cam.pos -= right * moveSpeed;
		if (input.right)
			ctx.cam.pos += right * moveSpeed;
		if (input.forward)
			ctx.cam.pos += forward * moveSpeed;
		if (input.back)
			ctx.cam.pos -= forward * moveSpeed;
		if (input.up)
			ctx.cam.pos[1] += moveSpeed;
		if (input.down)
			ctx.cam.pos[1] -= moveSpeed;

		// Update data.
		if (data) {
			data->setOutput(gfx_frame_get_index(frame));
			graph->update();
			graph->write(data.get());
		}

		// Record frame.
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
