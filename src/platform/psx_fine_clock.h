#ifndef PLATFORM_PSX_FINE_CLOCK_H
#define PLATFORM_PSX_FINE_CLOCK_H

#include <stdint.h>

/* Root counter 2, clocked by the system clock / 8 (about 269 ticks an NTSC horizontal blank),
   free-running over 16 bits: restarts it from 0. */
void psx_fine_clock_start(void);

uint16_t psx_fine_clock_read(void);

/* Returns the fine ticks counted over 64 horizontal blanks of psx_frame_timer; both clocks must be
   started. */
uint32_t psx_fine_clock_calibrate(void);

#endif
