#ifndef PLATFORM_PSX_PAD_PROTOCOL_H
#define PLATFORM_PSX_PAD_PROTOCOL_H

#include <stdint.h>

/* The pad script's two scratchpad words with tools/psx_pad_play.lua: the program raises the
 * handshake once its pad driver runs, the Lua writes the end word after the script's last step.
 * tools/psx_pad_protocol_check.sh (make psx-tools-test) fails when a value here and its `local`
 * in the Lua differ. */

#define PSX_PAD_SCRATCHPAD 0x1f800000u
#define PSX_PAD_HANDSHAKE_OFFSET 0x3fc
#define PSX_PAD_END_OFFSET 0x3f8
#define PSX_PAD_HANDSHAKE_MAGIC 0x50414421u
#define PSX_PAD_END_MAGIC 0x454e4421u

#define PSX_PAD_WORD(offset) ((volatile uint32_t *)(PSX_PAD_SCRATCHPAD + (offset)))

#endif
