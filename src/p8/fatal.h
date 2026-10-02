#ifndef P8_FATAL_H
#define P8_FATAL_H

#if defined(__GNUC__)
#define P8_NORETURN __attribute__((noreturn))
#else
#define P8_NORETURN
#endif

/* Fournie par la plateforme, pas par le coeur : ne rend jamais la main. Sur
 * PC (pico8_fatal_pc.c), ecrit message sur stderr et sort avec le code 3. */
void p8_fatal(const char *message) P8_NORETURN;

#endif
