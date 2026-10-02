#ifndef GAME_DATA_INIT_H
#define GAME_DATA_INIT_H

/* Builds the research projects -- research_init() first, since it forgets
 * any project already built, then research_build_projects() -- and clears
 * every technology's affordable flag. Safe to call more than once: the
 * previous projects are forgotten first. */
void data_init(void);

/* Forgets the research projects and marks the research project class down,
 * undoing data_init(). Safe to call more than once, or before the first
 * data_init(). */
void data_init_data_shutdown(void);

#endif
