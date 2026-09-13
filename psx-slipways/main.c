
#include <stdint.h>
#include <psxgpu.h>

#include "game.h"
#include "input.h"

#define SCREEN_XRES 320
#define SCREEN_YRES 240

typedef struct {
	DISPENV disp;
	DRAWENV draw;
} Framebuffer;

typedef struct {
	Framebuffer db[2];
	int         active;
} RenderContext;

static void init_context(RenderContext *ctx) {
	ResetGraph(0);
	ctx->active = 0;

	SetDefDispEnv(&(ctx->db[0].disp),           0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(ctx->db[0].draw), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDispEnv(&(ctx->db[1].disp), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(ctx->db[1].draw),           0, 0, SCREEN_XRES, SCREEN_YRES);

	for (int i = 0; i < 2; i++) {
		setRGB0(&(ctx->db[i].draw), 0, 0, 32);
		ctx->db[i].draw.isbg = 1;
		ctx->db[i].draw.dtd  = 1;
	}

	PutDrawEnv(&(ctx->db[0].draw));

	SetDispMask(1);
}

static void display(RenderContext *ctx) {
	DrawSync(0);
	VSync(0);

	ctx->active ^= 1;
	PutDrawEnv(&(ctx->db[ctx->active].draw));
	PutDispEnv(&(ctx->db[ctx->active ^ 1].disp));
}

int main(int argc, const char *argv[]) {
	RenderContext ctx;
	init_context(&ctx);

	input_init();

	game_init();

	static volatile int test_harness_ready = 0;
	test_harness_ready = 0x1eadbeef;

	for (;;) {
		game_update();
		game_draw();

		display(&ctx);
	}

	return 0;
}
