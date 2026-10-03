#ifndef ENGINE_EXIT_QUEUE_H
#define ENGINE_EXIT_QUEUE_H

#include "engine/entity_class.h"
#include "engine/allocator.h"

#define EXIT_QUEUE_CAP 512

/* An entity that has left live_entities and whose class has a release: the
 * current frame may still read it, so it is released only at the flush.
 * p8_fatal when the queue is full. */
void exit_queue_put(entity_t *e);

/* An arena nothing outside the current frame points to any more, released
 * whole at the flush. Same limit as the entities. */
void exit_queue_put_arena(const allocator_t *arena);

int exit_queue_count(void);
/* The most entries held at once since the process started; a flush does
 * not lower it. */
int exit_queue_max(void);

/* For each entry, in order: an entity goes through forget, which sets to
 * NULL whatever still points to it, then through its class's release; an
 * arena is released. The queue is empty on return. */
void exit_queue_flush(void (*forget)(const entity_t *e));

#endif
