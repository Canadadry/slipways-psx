#ifndef PICO8_CLASS_H
#define PICO8_CLASS_H

#include "pico8_ob.h"
#include "pico8_type_tag.h"

/* `name` is this class's own owned copy of the `fz` entry of `defaults`,
 * or "" when the spec sets none. `defaults` is the whole ob()-parsed
 * spec, nothing stripped out. `parent` is NULL for a root class. `pq` is
 * installed separately, by pico8_class_set_pq(). */
typedef struct pico8_class pico8_class_t;

typedef struct pico8_instance pico8_instance_t;

/* Called with the freshly built instance during instantiation. It takes
 * the instance rather than its raw fields table because a pq body reads
 * and writes fields through the ordinary class-chain fallback. */
typedef void (*pico8_class_pq_fn)(pico8_instance_t *instance);

struct pico8_class {
    char *name;
    pico8_class_t *parent;
    pico8_ob_table_t *defaults;
    pico8_class_pq_fn pq;
};

struct pico8_instance {
    /* Always PICO8_TYPE_TAG_INSTANCE, and kept first so pico8_type_tag_of()
     * can read it off a bare REF: set by pico8_class_instantiate(), and by
     * hand by the modules that build a classless instance (`mo`, `je`,
     * `fm`, the `mz.jj`/`ie.ke` wrappers, pz()'s row wrappers). */
    pico8_type_tag_t tag;
    pico8_ob_table_t *fields;
    /* NULL for a classless instance: field lookup then sees `fields` only. */
    pico8_class_t *cls;
};

/* ob()-parses `spec` (NULL is treated as ""), links the new class to
 * `parent` (NULL declares a root class) and registers it under its own
 * `fz`, or under "" when the spec sets none -- so every class without an
 * `fz` overwrites the same "" slot, exactly as in the cart. Returns a
 * class the caller owns; release it with pico8_class_free(). */
pico8_class_t *pico8_class_declare(pico8_class_t *parent, const char *spec);

/* Frees this class's own name copy and defaults table. Does NOT touch
 * `parent`, nor any instance built from this class: an instance holds a
 * borrowed `cls` pointer and outliving its class is the caller's bug. */
void pico8_class_free(pico8_class_t *cls);

void pico8_class_set_pq(pico8_class_t *cls, pico8_class_pq_fn fn);

/* Returns the class registered under `name`, or NULL if none ever was.
 * pico8_class_registry_clear() is test-support only: it empties the
 * registry, freeing its own name copies but never the classes, which the
 * caller still owns and must free with pico8_class_free(). */
pico8_class_t *pico8_class_lookup(const char *name);
void pico8_class_registry_clear(void);

/* A second registry, separate from the `fz` one above, keyed by a lookup
 * name this port chooses: it serves the classes that set no `fz` at all
 * (they would all collide in the "" slot) and that a module must reach
 * without including their headers. Registering past the capacity, or a
 * name that does not fit an entry, aborts.
 * pico8_class_registry_named_clear() is test-support only. */
#define PICO8_CLASS_NAMED_REGISTRY_CAPACITY 8

void pico8_class_register_named(const char *name, pico8_class_t *cls);
pico8_class_t *pico8_class_lookup_named(const char *name);
void pico8_class_registry_named_clear(void);

/* Shallow-copies `overrides` (NULL means none) into a fresh instance
 * attached to `cls`, then walks the chain from `cls` up through each
 * `parent`, calling each DISTINCT effective `pq` exactly once: an
 * ancestor's `pq` that several classes of the chain inherit fires once,
 * while a subclass's own distinct `pq` fires in addition to it,
 * derived-to-base. Class defaults are never copied into the instance.
 * Returns an instance the caller owns; release it with
 * pico8_instance_free(). */
pico8_instance_t *pico8_class_instantiate(pico8_class_t *cls, const pico8_ob_table_t *overrides);

/* Frees this instance's own fields table. Does NOT touch `cls`
 * (borrowed), nor any value stored in `fields`. */
void pico8_instance_free(pico8_instance_t *inst);

/* Walks the prototype chain live at every call: the instance's own
 * fields first, then `cls`'s defaults, then each `parent`'s, up to the
 * root. Returns 1 and writes *out_value if `name` is found anywhere on
 * that chain, 0 (leaving *out_value untouched) if it is absent
 * everywhere -- pico8_ob_table_get_name's own "absent" convention. */
int pico8_instance_get_field(const pico8_instance_t *inst, const char *name, pico8_value_t *out_value);

#endif
