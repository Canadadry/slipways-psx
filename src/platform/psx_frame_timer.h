#ifndef PLATFORM_PSX_FRAME_TIMER_H
#define PLATFORM_PSX_FRAME_TIMER_H

#include <stdint.h>

/* Root counter 1, clocked by the horizontal blank, free-running over 16 bits:
   restarts it from 0. */
void psx_frame_timer_start(void);

uint16_t psx_frame_timer_read(void);

/* The vblanks counted since ResetGraph. */
uint32_t psx_frame_timer_vsyncs(void);

/* Waits for two vblanks and returns the horizontal blanks counted between
   them; the timer must be started. */
uint32_t psx_frame_timer_calibrate(void);

#endif
