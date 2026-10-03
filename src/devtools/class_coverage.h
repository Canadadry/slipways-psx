#ifndef DEVTOOLS_CLASS_COVERAGE_H
#define DEVTOOLS_CLASS_COVERAGE_H

#ifdef CLASS_COVERAGE_BUILD
/* Keeps `name` itself, not a copy: pass a string literal. */
void class_coverage_record(const char *name);
long class_coverage_count(const char *name);
/* Returns the first counted name that is not one of the `known_count`
 * entries of `known`, "" included; NULL when every counted name is known. */
const char *class_coverage_first_unknown(const char *const *known, int known_count);
#endif

#endif
