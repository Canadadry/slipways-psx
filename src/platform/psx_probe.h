#ifndef PLATFORM_PSX_PROBE_H
#define PLATFORM_PSX_PROBE_H

#include <stdint.h>

/* Probe builds only (PSX_PROBE, make psx-probe): never linked into the game or the measured
   replays of make psx-short-runs. */

/* Starts the fine clock and calibrates it; the frame timer must be started. */
void psx_probe_start(void);

/* Busy-waits hblanks horizontal blanks (1..200, the fine clock's wrap limit) on the fine clock and
   counts the call; a build uses a single value: another one, or one out of range, is a p8_fatal. */
void psx_probe_delay(uint32_t hblanks);

/* Writes "psx-probe: clk8-per-64-hblanks <n>", then "psx-probe: delay <hblanks> calls <count>"
   when psx_probe_delay was called. */
void psx_probe_write(void);

#endif
