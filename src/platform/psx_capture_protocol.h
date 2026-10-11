#ifndef PLATFORM_PSX_CAPTURE_PROTOCOL_H
#define PLATFORM_PSX_CAPTURE_PROTOCOL_H

/* The frame-locked capture handshake with tools/psx_capture_frames.lua, on three scratchpad words
 * below the pad script's two. tools/psx_capture_protocol_check.sh (make test) fails when a value
 * here and its `local` in the Lua differ. */

#define PSX_CAPTURE_SCRATCHPAD 0x1f800000u
#define PSX_CAPTURE_STATE_OFFSET 0x3f4
#define PSX_CAPTURE_FRAME_OFFSET 0x3f0
#define PSX_CAPTURE_MASK_OFFSET 0x3ec

/* State word values. */
#define PSX_CAPTURE_PROGRAM_READY 0x43505244u
#define PSX_CAPTURE_NEXT_READY 0x434e5854u
#define PSX_CAPTURE_FRAME_SHOWN 0x43534857u

/* Frame word once the script has no capture left. */
#define PSX_CAPTURE_NO_FRAME 0xffffffffu

#endif
