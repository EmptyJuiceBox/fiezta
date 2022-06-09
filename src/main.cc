extern "C" {
#include <groufix.h>
}

#include <iostream>
#include <stdlib.h>

struct Context {
	GFXTechnique *technique;
	GFXSet *set;
	GFXRenderable renderable;
};

static void render_callback(
		GFXRecorder* recorder, unsigned int /*frame*/, void *ptr) {
	Context *ctx = (Context *)ptr;
	// Record stuff.
	gfx_cmd_bind(recorder, ctx->technique, 0, 1, 0, &ctx->set, NULL);
	gfx_cmd_draw_indexed(recorder, &ctx->renderable, 0, 0, 0, 0, 1);
}

static const char* glsl_vertex =
	"#version 450\n"
	"layout(row_major, set = 0, binding = 0) uniform UBO {\n"
	"  mat4 mvp;\n"
	"};\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec3 color;\n"
	"layout(location = 2) in vec2 texCoord;\n"
	"layout(location = 0) out vec3 fragColor;\n"
	"layout(location = 1) out vec2 fragTexCoord;\n"
	"out gl_PerVertex {\n"
	"  vec4 gl_Position;\n"
	"};\n"
	"void main() {\n"
	"  gl_Position = mvp * vec4(position, 1.0);\n"
	"  fragColor = color;\n"
	"  fragTexCoord = texCoord;\n"
	"}\n";

static const char* glsl_fragment =
	"#version 450\n"
	"layout(set = 0, binding = 1) uniform sampler2D texSampler;\n"
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

	GFXVideoMode mode = {
		.width = 600,
		.height = 400,
	};
	GFXWindowFlags flags = (GFXWindowFlags)
		(GFX_WINDOW_RESIZABLE | GFX_WINDOW_DOUBLE_BUFFER);
	GFXWindow *window = gfx_create_window(
		flags, device, NULL,
		mode, "groufix");
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
		pass, 0, GFX_ACCESS_ATTACHMENT_WRITE, (GFXShaderStage)0));

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
			.buffer = {GFXReference::GFX_REF_EMPTY, NULL, 0, {0, 0}},
		}, {
			.format = GFX_FORMAT_R32G32B32_SFLOAT,
			.offset = sizeof(float) * 3,
			.stride = sizeof(float) * 8,
			.buffer = {GFXReference::GFX_REF_EMPTY, NULL, 0, {0, 0}},
		}, {
			.format = GFX_FORMAT_R32G32_SFLOAT,
			.offset = sizeof(float) * 6,
			.stride = sizeof(float) * 8,
			.buffer = {GFXReference::GFX_REF_EMPTY, NULL, 0, {0, 0}},
		},
	};

	GFXPrimitive *primitive = gfx_alloc_prim(heap,
		GFX_MEMORY_WRITE, (GFXBufferUsage)0,
		GFX_TOPO_TRIANGLE_STRIP, 4, sizeof(uint16_t), 4,
		{GFXReference::GFX_REF_EMPTY, NULL, 0, {0, 0}},
		3, attribs);
	dassert(primitive);

	GFXBufferRef vert = { \
		.type = GFXReference::GFX_REF_PRIMITIVE_VERTICES, \
		.obj = primitive, \
		.offset = 0, \
		.values = {0, 0} \
	};

	GFXBufferRef ind = { \
		.type = GFXReference::GFX_REF_PRIMITIVE_INDICES,
		.obj = primitive,
		.offset = 0,
		.values = { 0, 0 },
	};

	dassert(gfx_write(
		vertexData, vert, GFX_TRANSFER_ASYNC, 1, 1,
		(GFXRegion[]){{ .offset = 0, .size = sizeof(vertexData) }},
		(GFXRegion[]){{ .offset = 0, .size = 0 }},
		(GFXInject[]){
			(GFXInject){
				.type = GFXInject::GFX_DEP_SIGNAL,
				.dep = dep,
				.ref = {GFXReference::GFX_REF_EMPTY, NULL, 0, {0, 0}},
				.mask = GFX_ACCESS_VERTEX_READ,
				.stage = (GFXShaderStage)0,
			}
		}));


	dassert(gfx_write(indexData, ind, GFX_TRANSFER_ASYNC, 1, 1,
		(GFXRegion[]){{ .offset = 0, .size = sizeof(indexData) }},
		(GFXRegion[]){{ .offset = 0, .size = 0 }},
		(GFXInject[]){
			(GFXInject){
				.type = GFXInject::GFX_DEP_SIGNAL,
				.dep = dep,
				.ref = {GFXReference::GFX_REF_EMPTY, NULL, 0, {0, 0}},
				.mask = GFX_ACCESS_INDEX_READ,
				.stage = (GFXShaderStage)0,
			}
		}));

	GFXImage* image = gfx_alloc_image(heap,
		GFX_MEMORY_WRITE, GFX_IMAGE_2D,
		GFX_IMAGE_SAMPLED, GFX_FORMAT_R8_UNORM, 1, 1,
		4, 4, 1);
	dassert(image);

	GFXGroup* group = gfx_alloc_group(heap,
		GFX_MEMORY_WRITE,
		GFX_BUFFER_UNIFORM,
		2, (GFXBinding[]){
			{
				.type = GFX_BINDING_BUFFER,
				.count = 1,
				.elementSize = sizeof(float) * 16,
				.numElements = 1,
				.buffers = NULL
			}, {
				.type = GFX_BINDING_IMAGE,
				.count = 1,
				.images = (GFXImageRef[]){
					(GFXImageRef){
						.type = GFXReference::GFX_REF_IMAGE,
						.obj = image,
						.offset = 0,
						.values = { 0, 0 }
					},
				},
			}
		});
	dassert(group);

	GFXShader *vertex = gfx_create_shader(GFX_STAGE_VERTEX, device);
	dassert(vertex);
	GFXShader *fragment = gfx_create_shader(GFX_STAGE_FRAGMENT, device);
	dassert(vertex);

	GFXStringReader str;
	dassert(gfx_shader_compile(
		vertex, GFX_GLSL, 1,
		gfx_string_reader(&str, glsl_vertex), NULL, NULL));
	dassert(gfx_shader_compile(
		fragment, GFX_GLSL, 1,
		gfx_string_reader(&str, glsl_fragment), NULL, NULL));

	GFXShader *shaders[] = {vertex, fragment};
	ctx.technique = gfx_renderer_add_tech(renderer, 2, shaders);
	dassert(ctx.technique);
	gfx_tech_immutable(ctx.technique, 0, 1); // Warns on fail.

	ctx.set = gfx_renderer_add_set(
		renderer, ctx.technique, 0,
		0, 1, 0, 0,
		NULL,
		(GFXSetGroup[]){{
			.binding = 0,
			.offset = 0,
			.numBindings = 0,
			.group = group,
		}},
		NULL,
		NULL);
	dassert(ctx.set);

	gfx_renderable(&ctx.renderable, pass, ctx.technique, primitive);

	while (!gfx_window_should_close(window))
	{
		GFXFrame *frame = gfx_renderer_acquire(renderer);
		gfx_frame_start(frame, 1, (GFXInject[]){
			(GFXInject){
				.type = GFXInject::GFX_DEP_WAIT,
				.dep = dep,
				.ref = {GFXReference::GFX_REF_EMPTY, NULL, 0, {0, 0}},
			}});

		gfx_recorder_render(recorder, pass, render_callback, (void *)&ctx);
		gfx_frame_submit(frame);
		gfx_heap_purge(heap);
		gfx_wait_events();
	}
}
