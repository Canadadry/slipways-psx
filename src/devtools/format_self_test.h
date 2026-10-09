#ifndef DEVTOOLS_FORMAT_SELF_TEST_H
#define DEVTOOLS_FORMAT_SELF_TEST_H

enum { FORMAT_SELF_TEST_CAP = 128 };

/* Writes one line, without '\n', exercising each printf conversion the core
   and the state text use (%d %s %ld %02ld %lu %04lu); compared across
   machines, it names a libc formatter difference directly. */
void format_self_test(char text[FORMAT_SELF_TEST_CAP]);

#endif
