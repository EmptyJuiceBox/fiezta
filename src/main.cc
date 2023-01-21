#include <groufix.h>

#include <iostream>
#include <stdlib.h>

template<typename T>
const T *ref(const T &val) {
	return &val;
}

struct Context {
	GFXTechnique *technique;
	GFXSet *set;
	GFXRenderable renderable;
};

static void render_callback(
		GFXRecorder* recorder, unsigned int /*frame*/, void *ptr) {
	Context *ctx = (Context *)ptr;
	// Record stuff.
	float mvp[] = {
		1.0f, 0.2f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	gfx_cmd_push(recorder, ctx->technique, 0, sizeof(mvp), mvp);
	gfx_cmd_bind(recorder, ctx->technique, 0, 1, 0, &ctx->set, NULL);
	gfx_cmd_draw_indexed(recorder, &ctx->renderable, 0, 0, 0, 0, 1);
}

static const char* glsl_vertex =
	"#version 450\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec3 color;\n"
	"layout(location = 2) in vec2 texCoord;\n"
	"layout(location = 0) out vec3 fragColor;\n"
	"layout(location = 1) out vec2 fragTexCoord;\n"
	"out gl_PerVertex {\n"
	"  vec4 gl_Position;\n"
	"};\n"
	"layout(row_major, push_constant) uniform Constants {\n"
	"  mat4 mvp;\n"
	"};\n"
	"void main() {\n"
	"  gl_Position = mvp * vec4(position, 1.0);\n"
	"  fragColor = color;\n"
	"  fragTexCoord = texCoord;\n"
	"}\n";

static const char* glsl_fragment =
	"#version 450\n"
	"layout(set = 0, binding = 0) uniform sampler2D texSampler;\n"
	"layout(location = 0) in vec3 fragColor;\n"
	"layout(location = 1) in vec2 fragTexCoord;\n"
	"layout(location = 0) out vec4 outColor;\n"
	"void main() {\n"
	"  float tex = texture(texSampler, fragTexCoord).r;\n"
	"  outColor = vec4(fragColor, 1.0) * tex;\n"
	"}\n";

#define dassert(expr) do { \
	if (!(expr)) { \
		std::cerr << "Asertion failed: " #expr << '\n'; \
		std::cerr << "  At " << __FILE__ << ":" << __LINE__ << "(" << __func__ << ")\n"; \
		abort(); \
	} \
} while (0)

int main() {
	Context ctx;

	dassert(gfx_init());

	GFXDevice *device = NULL;

	GFXWindow *window = gfx_create_window(
		GFX_WINDOW_RESIZABLE | GFX_WINDOW_DOUBLE_BUFFER | GFX_WINDOW_FOCUS,
		device, NULL, {600, 400, 0}, "groufix");
	dassert(window);

	GFXHeap *heap = gfx_create_heap(device);
	dassert(heap);

	GFXDependency *dep = gfx_create_dep(device, 2);
	dassert(dep);

	GFXRenderer *renderer = gfx_create_renderer(device, 2);
	dassert(renderer);

	dassert(gfx_renderer_attach_window(renderer, 0, window));

	GFXRecorder *recorder = gfx_renderer_add_recorder(renderer);
	dassert(recorder);

	GFXPass *pass = gfx_renderer_add_pass(renderer, 0, NULL);
	dassert(pass);

	dassert(gfx_pass_consume(
		pass, 0, GFX_ACCESS_ATTACHMENT_WRITE, GFX_STAGE_ANY));
	gfx_pass_clear(
		pass, 0, GFX_IMAGE_COLOR, GFXClear{{0.0f, 0.0f, 0.0f, 0.0f}});

	uint16_t indexData[] = {
		0, 1, 3, 2
	};

	float vertexData[] = {
		-0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f
	};

	GFXAttribute attribs[] = {
		{
			.format = GFX_FORMAT_R32G32B32_SFLOAT,
			.offset = 0,
			.stride = sizeof(float) * 8,
			.buffer = GFX_REF_NULL,
		}, {
			.format = GFX_FORMAT_R32G32B32_SFLOAT,
			.offset = sizeof(float) * 3,
			.stride = sizeof(float) * 8,
			.buffer = GFX_REF_NULL,
		}, {
			.format = GFX_FORMAT_R32G32_SFLOAT,
			.offset = sizeof(float) * 6,
			.stride = sizeof(float) * 8,
			.buffer = GFX_REF_NULL,
		},
	};
	GFXPrimitive *primitive = gfx_alloc_prim(heap,
		GFX_MEMORY_WRITE, GFX_BUFFER_NONE,
		GFX_TOPO_TRIANGLE_STRIP, 4, sizeof(uint16_t), 4,
		GFX_REF_NULL, 3, attribs);
	dassert(primitive);

	GFXBufferRef vert = gfx_ref_prim_vertices(primitive, 0);
	GFXBufferRef ind = gfx_ref_prim_indices(primitive);

	dassert(gfx_write(vertexData, vert, GFX_TRANSFER_ASYNC, 1, 1,
		ref(GFXRegion{.buf = {.offset = 0, .size = sizeof(vertexData)}}),
		ref(GFXRegion{}),
		ref(GFXInject{
			gfx_dep_sig(dep, GFX_ACCESS_VERTEX_READ, GFX_STAGE_ANY),
		})));

	dassert(gfx_write(indexData, ind, GFX_TRANSFER_ASYNC, 1, 1,
		ref(GFXRegion{.buf = {.offset = 0, .size = sizeof(indexData)}}),
		ref(GFXRegion{}),
		ref(GFXInject{
			gfx_dep_sig(dep, GFX_ACCESS_INDEX_READ, GFX_STAGE_ANY)
		})));

	uint8_t imgData[] = {
		255, 0, 255, 0,
		0, 255, 0, 255,
		255, 0, 255, 0,
		0, 255, 0, 255
	};

	GFXImage* image = gfx_alloc_image(heap,
		GFX_IMAGE_2D, GFX_MEMORY_WRITE,
		GFX_IMAGE_SAMPLED, GFX_FORMAT_R8_UNORM, 1, 1,
		4, 4, 1);
	dassert(image);

	GFXImageRef img = gfx_ref_image(image);

	dassert(gfx_write(imgData, img, GFX_TRANSFER_ASYNC, 1, 1,
		ref(GFXRegion{}),
		ref(GFXRegion{.img = {
			.aspect = GFX_IMAGE_COLOR,
			.mipmap = 0, .layer = 0,  .numLayers = 1,
			.x = 0,      .y = 0,      .z = 0,
			.width = 4,  .height = 4, .depth = 1
		}}),
		ref(GFXInject{
			gfx_dep_sig(dep, GFX_ACCESS_SAMPLED_READ, GFX_STAGE_FRAGMENT)
		})));

	dassert(gfx_heap_flush(heap));

	GFXShader *vertex = gfx_create_shader(GFX_STAGE_VERTEX, device);
	dassert(vertex);
	GFXShader *fragment = gfx_create_shader(GFX_STAGE_FRAGMENT, device);
	dassert(vertex);

	GFXStringReader str;
	dassert(gfx_shader_compile(
		vertex, GFX_GLSL, 1,
		gfx_string_reader(&str, glsl_vertex), NULL, NULL, NULL));
	dassert(gfx_shader_compile(
		fragment, GFX_GLSL, 1,
		gfx_string_reader(&str, glsl_fragment), NULL, NULL, NULL));

	GFXShader *shaders[] = {vertex, fragment};
	ctx.technique = gfx_renderer_add_tech(renderer, 2, shaders);
	dassert(ctx.technique);
	gfx_tech_immutable(ctx.technique, 0, 0); // Warns on fail.

	ctx.set = gfx_renderer_add_set(
		renderer, ctx.technique, 0,
		1, 0, 0, 0,
		ref(GFXSetResource{.binding = 0, .index = 0, .ref = img}),
		NULL, NULL, NULL);
	dassert(ctx.set);

	gfx_renderable(&ctx.renderable, pass, ctx.technique, primitive, NULL);

	while (!gfx_window_should_close(window))
	{
		GFXFrame *frame = gfx_renderer_acquire(renderer);
		gfx_poll_events();
		gfx_frame_start(frame, 1, ref(GFXInject{ gfx_dep_wait(dep) }));
		gfx_recorder_render(recorder, pass, render_callback, (void *)&ctx);
		gfx_frame_submit(frame);
		gfx_heap_purge(heap);
	}

	gfx_destroy_renderer(renderer);
	gfx_destroy_shader(vertex);
	gfx_destroy_shader(fragment);
	gfx_destroy_heap(heap);
	gfx_destroy_dep(dep);
	gfx_destroy_window(window);

	gfx_terminate();
}
