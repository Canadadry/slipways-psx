#ifndef PICO8_STRUCT_H
#define PICO8_STRUCT_H

#include "pico8_type_tag.h"
#include "pico8_value.h"

typedef enum {
    PICO8_METHOD_OL,
    PICO8_METHOD_COUNT
} pico8_method_t;

/* First member of every entity struct, and of the parent struct a child
 * embeds first: the tag stays at offset 0 for pico8_type_tag_of(). `tag` is
 * always PICO8_TYPE_TAG_STRUCT; `methods` points at the class's static
 * table of PICO8_METHOD_COUNT entries, indexed by pico8_method_t. */
typedef struct {
    pico8_type_tag_t tag;
    const pico8_value_function_t *methods;
} pico8_struct_header_t;

#endif
