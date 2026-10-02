#ifndef ENGINE_ENTITY_H
#define ENGINE_ENTITY_H

#include "engine/entity_class.h"

/* Shuts the module down first. */
void entity_init(void);

/* Idempotent, and safe before the first entity_init(). */
void entity_shutdown(void);

/* 1 from entity_init() to entity_shutdown(), 0 otherwise. */
int entity_ready(void);

/* A NULL `self` is a no-op. Writes the state name into the header and sets
 * the state time to 0; a NULL state name means no state, a string is
 * borrowed. */
void entity_set_state(entity_t *self, const char *me);

/* Gives `self` its class header and sets the state name and state time,
 * those of the class's archetype. The state name is borrowed. */
void entity_init_instance(entity_t *self, const entity_class_t *cls, const char *me,
                          p8_fixed_t t);

/* The base constructor: registers `self`. A derived constructor calls it
 * exactly once, after its own work, or alone when it has none, so the work
 * runs derived to base and an inherited part never runs twice. */
void entity_construct(entity_t *self);

#endif
