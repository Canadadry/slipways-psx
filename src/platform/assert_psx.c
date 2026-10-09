#include "platform/psx_exit.h"
#include "platform/psx_tty.h"

#include <stddef.h>

/* Linked with -Wl,--wrap=_assert_abort: the SDK's own, in the same archive
   member as abort, only prints and spins, so the boot would last until its
   timeout, and a plain override is a duplicate definition. */
void __wrap__assert_abort(const char *file, int line, const char *expr);

void __wrap__assert_abort(const char *file, int line, const char *expr) {
    psx_exit_set_hook(NULL);
    psx_tty_printf("assert: %s:%d %s\n", file, line, expr);
    psx_exit(PSX_EXIT_ASSERT);
}
