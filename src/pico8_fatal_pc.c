#include "pico8_fatal.h"

#include <stdio.h>
#include <stdlib.h>

void pico8_fatal(const char *message) {
    fprintf(stderr, "pico8_fatal: %s\n", message);
    exit(3);
}
