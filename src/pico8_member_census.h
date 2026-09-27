#ifndef PICO8_MEMBER_CENSUS_H
#define PICO8_MEMBER_CENSUS_H

/* Every hook compiles to nothing, or to its wrapped expression, without
 * -DPICO8_MEMBER_CENSUS.
 *
 * SEE(obj, typedef): call once the members of `obj` are set, at the end of
 * its constructor or of its module's init for a static. `typedef` is the
 * most derived typedef of `obj`; one descriptor always goes with one
 * typedef. READ wraps the struct reader call of pico8_entity_get_field and
 * yields its result. WRITE follows the struct writer call of
 * pico8_entity_set_field. WALK takes pico8_hv().
 *
 * Observations go to static storage, never freed and never on the heap;
 * at exit they are appended to $PICO8_MEMBER_CENSUS_OUT/<pid>.txt, and
 * nothing is written when that variable is unset or the process aborts. */
#ifdef PICO8_MEMBER_CENSUS
void pico8_member_census_see(const void *obj, const char *typedef_name);
int pico8_member_census_read(const void *e, const char *name, int found, const void *out_value);
void pico8_member_census_write(const void *e, const char *name, const void *value);
void pico8_member_census_walk(const void *hv);
#define PICO8_MEMBER_CENSUS_SEE(obj, typedef_name) pico8_member_census_see((obj), #typedef_name)
#define PICO8_MEMBER_CENSUS_READ(e, name, found, out_value) pico8_member_census_read((e), (name), (found), (out_value))
#define PICO8_MEMBER_CENSUS_WRITE(e, name, value) pico8_member_census_write((e), (name), &(value))
#define PICO8_MEMBER_CENSUS_WALK(hv) pico8_member_census_walk(hv)
#else
#define PICO8_MEMBER_CENSUS_SEE(obj, typedef_name) ((void)0)
#define PICO8_MEMBER_CENSUS_READ(e, name, found, out_value) (found)
#define PICO8_MEMBER_CENSUS_WRITE(e, name, value) ((void)0)
#define PICO8_MEMBER_CENSUS_WALK(hv) ((void)0)
#endif

#endif
