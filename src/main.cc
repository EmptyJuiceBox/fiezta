#include "def.h"

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

int main() {
	dassert(gfx_init());

	GFXWindow *window = gfx_create_window(
		GFX_WINDOW_RESIZABLE | GFX_WINDOW_DOUBLE_BUFFER | GFX_WINDOW_FOCUS,
		nullptr, nullptr, {600, 400, 0}, "fiezta");
	dassert(window);

	window->events.key.press = key_press;

	GFXHeap* heap = gfx_create_heap(nullptr);
	dassert(heap);

	GFXRenderer* renderer = gfx_create_renderer(heap, NUM_VIRTUAL_FRAMES);
	dassert(renderer);

	dassert(gfx_renderer_attach_window(renderer, 0, window));

	GFXPass *pass = gfx_renderer_add_pass(renderer, GFX_PASS_RENDER, 0, nullptr);
	dassert(pass);

	dassert(gfx_pass_consume(
		pass, 0, GFX_ACCESS_ATTACHMENT_WRITE, GFX_STAGE_ANY));
	gfx_pass_clear(
		pass, 0, GFX_IMAGE_COLOR, {{0.0f, 0.0f, 0.0f, 0.0f}});

	// Main loop.
	while (!gfx_window_should_close(window)) {
		GFXFrame *frame = gfx_renderer_acquire(renderer);
		gfx_poll_events();
		gfx_frame_start(frame);
		gfx_frame_submit(frame);
	}

	gfx_destroy_renderer(renderer);
	gfx_destroy_heap(heap);
	gfx_destroy_window(window);
	gfx_terminate();
}
