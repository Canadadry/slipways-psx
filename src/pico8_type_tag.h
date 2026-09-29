#ifndef PICO8_TYPE_TAG_H
#define PICO8_TYPE_TAG_H

#include <string.h>

/* A first-member type discriminator for the one tagged heap shape a REF
 * can point at: the entity structs that begin with a pico8_struct_header_t
 * (pico8_struct.h).
 *
 * A REF, an opaque `void *`, carries no
 * type of its own. The tag makes "what is this REF really" a question with
 * an answer, and with an explicit "I don't know" for anything untagged.
 *
 * CHOICE OF MAGIC VALUES. Not 0/1/2: this tag sits at offset 0, exactly
 * where each struct's first pointer member used to live, so a mis-cast
 * REF (or a stale/garbage word) is read AS a tag -- and small integers
 * are overwhelmingly the most likely bit patterns to find in real program
 * data (counts, indices, flags, enum values, the low half of a small heap
 * pointer). The value is instead a full-width 32-bit constant with a
 * deliberately unusual 0x9E5C prefix: it is non-zero, outside every
 * plausible count/index range, not ASCII text and not a plausible aligned
 * pointer half. Hitting it by accident is a 1-in-2^32 coincidence,
 * and the sole consequence would be the blind cast the code did before
 * this header existed. */

typedef unsigned int pico8_type_tag_t;

#define PICO8_TYPE_TAG_UNKNOWN 0x00000000u
#define PICO8_TYPE_TAG_STRUCT 0x9E5C57C7u

/* Reports which of the tagged shapes `ref` really points at, or
 * PICO8_TYPE_TAG_UNKNOWN for NULL and for anything whose first 4 bytes are
 * not one of the magic values (an untagged struct, a
 * pico8_vec_t/pico8_qz_box_t/... REF, a stale pointer).
 *
 * Read through memcpy rather than a struct cast so this stays well-defined
 * even when `ref` does NOT point at one of the tagged types -- which is
 * precisely the case the caller is trying to detect. Callers must only
 * pass a REF to an object of at least 4 bytes; every REF-carrying shape in
 * this port is far larger. */
static inline pico8_type_tag_t pico8_type_tag_of(const void *ref) {
    pico8_type_tag_t tag;

    if (ref == NULL) {
        return PICO8_TYPE_TAG_UNKNOWN;
    }
    memcpy(&tag, ref, sizeof tag);
    if (tag == PICO8_TYPE_TAG_STRUCT) {
        return tag;
    }
    return PICO8_TYPE_TAG_UNKNOWN;
}

#endif /* PICO8_TYPE_TAG_H */
