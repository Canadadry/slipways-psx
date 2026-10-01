#ifndef PICO8_DATA_INIT_H
#define PICO8_DATA_INIT_H

/* The cart's one-shot top-level data build, in the source's own order. It
 * also runs pico8_research_init() before pico8_research_build_gb(), which
 * needs the hp class to exist, and forgets every `e_` of nb. Safe to call
 * more than once -- it frees a previous build first. */
void pico8_data_init(void);

/* Tears down gb and the hp class too, since pico8_data_init() built them.
 * Safe to call more than once, or before the first pico8_data_init(). */
void pico8_data_shutdown(void);

#endif
