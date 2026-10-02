#ifndef P8_FATAL_H
#define P8_FATAL_H

#if defined(__GNUC__)
#define P8_NORETURN __attribute__((noreturn))
#else
#define P8_NORETURN
#endif

/* Provided by the platform, not the core: never returns. On PC
 * (platform/fatal_pc.c), writes the message to stderr and exits with code 3. */
void p8_fatal(const char *message) P8_NORETURN;

#endif
