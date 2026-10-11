#ifndef PLATFORM_PSX_EXIT_H
#define PLATFORM_PSX_EXIT_H

#include "p8/fatal.h"

enum {
    PSX_EXIT_OK = 0,
    PSX_EXIT_FATAL = 3,
    PSX_EXIT_ASSERT = 4
};

typedef void (*psx_exit_hook_fn)(int code);

/* NULL removes the hook. psx_exit clears it before calling it, so a hook that
   ends in psx_exit itself (an assert inside it) cannot run twice. */
void psx_exit_set_hook(psx_exit_hook_fn hook);

/* Leaves PCSX-Redux with code when it runs under -testmode; elsewhere the
   emulator only pauses and the call spins forever. */
void psx_exit(int code) P8_NORETURN;

#endif
