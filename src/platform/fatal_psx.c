#include "p8/fatal.h"

#include "platform/psx_exit.h"
#include "platform/psx_tty.h"

void p8_fatal(const char *message) {
    psx_tty_write("p8_fatal: ");
    psx_tty_write(message);
    psx_tty_write("\n");
    psx_exit(PSX_EXIT_FATAL);
}
