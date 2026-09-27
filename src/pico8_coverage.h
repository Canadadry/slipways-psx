#ifndef PICO8_COVERAGE_H
#define PICO8_COVERAGE_H

#ifdef PICO8_CLASS_COVERAGE
/* Keeps `name` itself, not a copy: pass a string literal. */
void pico8_class_coverage_record(const char *name);
long pico8_class_coverage_count(const char *name);
/* Returns the first counted name that is not one of the `known_count`
 * entries of `known`, "" included; NULL when every counted name is known. */
const char *pico8_class_coverage_first_unknown(const char *const *known, int known_count);
#endif

#endif
