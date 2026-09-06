#include <tonc.h>

int main(void)
{
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;

	tte_init_chr4c_default(0, BG_CBB(0) | BG_SBB(31));
	tte_write("Hello, Slipways GBA!");

	while (1)
		VBlankIntrWait();

	return 0;
}
