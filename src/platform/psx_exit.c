#include "platform/psx_exit.h"

#include <stddef.h>
#include <stdint.h>

#define PCSX_EXIT_REGISTER ((volatile uint16_t *)0x1f802082)

static psx_exit_hook_fn g_hook;

void psx_exit_set_hook(psx_exit_hook_fn hook) {
    g_hook = hook;
}

void psx_exit(int code) {
    psx_exit_hook_fn hook = g_hook;
    g_hook = NULL;
    if (hook != NULL) hook(code);
    *PCSX_EXIT_REGISTER = (uint16_t)code;
    for (;;) {
    }
}
