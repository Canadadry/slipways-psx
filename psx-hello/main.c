
#include <stdint.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxapi.h>

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define CURSOR_SPEED 2
#define CURSOR_SIZE  4
#define CLICK_FLASH_FRAMES 15

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

	FntLoad(960, 0);
	FntOpen(8, 16, 304, 208, 0, 512);

	SetDispMask(1);
}

static void display(RenderContext *ctx) {
	DrawSync(0);
	VSync(0);

	ctx->active ^= 1;
	PutDrawEnv(&(ctx->db[ctx->active].draw));
	PutDispEnv(&(ctx->db[ctx->active ^ 1].disp));
}

static char pad_buff[2][34];

int main(int argc, const char *argv[]) {
	RenderContext ctx;
	init_context(&ctx);

	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);

	int cursor_x = SCREEN_XRES / 2;
	int cursor_y = SCREEN_YRES / 2;
	int mouse_mode = 0;
	int click_flash = 0;
	uint16_t prev_btn = 0xffff;

	for (;;) {
		PADTYPE *pad = (PADTYPE *) pad_buff[0];
		int connected = (pad->stat == 0);
		uint16_t btn = connected ? pad->btn : 0xffff;

		uint16_t pressed = (~btn) & prev_btn;

		if (connected) {
			if (pressed & PAD_CROSS)
				mouse_mode = !mouse_mode;

			if (mouse_mode) {
				if (!(btn & PAD_UP))    cursor_y -= CURSOR_SPEED;
				if (!(btn & PAD_DOWN))  cursor_y += CURSOR_SPEED;
				if (!(btn & PAD_LEFT))  cursor_x -= CURSOR_SPEED;
				if (!(btn & PAD_RIGHT)) cursor_x += CURSOR_SPEED;

				if (cursor_x < 0) cursor_x = 0;
				if (cursor_y < 0) cursor_y = 0;
				if (cursor_x > SCREEN_XRES - 1) cursor_x = SCREEN_XRES - 1;
				if (cursor_y > SCREEN_YRES - 1) cursor_y = SCREEN_YRES - 1;

				if (pressed & PAD_CIRCLE)
					click_flash = CLICK_FLASH_FRAMES;
			}
		}

		prev_btn = btn;
		if (click_flash > 0)
			click_flash--;

		FntPrint(-1, "SLIPWAYS PSX - psx-hello\n\n");
		if (!connected) {
			FntPrint(-1, "PORT 1: NO PAD DETECTED\nCONNECT A CONTROLLER");
		} else {
			FntPrint(
				-1,
				"PORT 1: TYPE=%x\n"
				"MOUSE MODE: %s (Cross to toggle)\n"
				"CURSOR: %d,%d (D-pad to move)\n"
				"%s",
				pad->type,
				mouse_mode ? "ON" : "off",
				cursor_x, cursor_y,
				click_flash > 0 ? "CLICK! (Circle)" : ""
			);
		}
		FntFlush(-1);

		if (connected && mouse_mode) {
			TILE tile;
			setTile(&tile);
			setXY0(&tile, cursor_x - CURSOR_SIZE / 2, cursor_y - CURSOR_SIZE / 2);
			setWH(&tile, CURSOR_SIZE, CURSOR_SIZE);
			if (click_flash > 0)
				setRGB0(&tile, 255, 255, 0);
			else
				setRGB0(&tile, 255, 255, 255);

			DrawPrim(&tile);
		}

		display(&ctx);
	}

	return 0;
}
