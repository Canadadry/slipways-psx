#ifndef TESTS_HEAP_ALLOCATOR_H
#define TESTS_HEAP_ALLOCATOR_H

#include "engine/allocator.h"

/* Zone des TU adossee a malloc, realloc et free : un bloc qu'elle alloue se
 * libere aussi par free. */
extern const allocator_t heap_allocator;

#endif
