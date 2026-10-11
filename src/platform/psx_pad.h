#ifndef PLATFORM_PSX_PAD_H
#define PLATFORM_PSX_PAD_H

#include <stdint.h>

/* Starts the BIOS pad driver on both ports; once, before any read. */
void psx_pad_start(void);

/* The buttons held on port 1 in hardware bit order, 1 = pressed (the BIOS
   word is active-low); 0 when no pad answers. */
uint16_t psx_pad_read_pressed(void);

#endif
