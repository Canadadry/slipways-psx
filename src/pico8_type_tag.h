#ifndef PICO8_TYPE_TAG_H
#define PICO8_TYPE_TAG_H

#include <string.h>

/* A first-member type discriminator for the three heap shapes this port
 * uses to model Lua's single table type: pico8_list_t (pico8_list.h),
 * pico8_ob_table_t (pico8_ob.h) and pico8_instance_t (pico8_class.h).
 *
 * A pico8_value_t REF (pico8_value.h) is a bare `void *` and carries no
 * type of its own. The three structs are the same size and share a
 * compatible header, so a cast between any two compiles silently AND
 * yields a plausible-looking object -- while their element strides differ
 * by 2x. Reading a pico8_list_t as a pico8_ob_table_t therefore walks its
 * backing array at twice the stride, past the end of the allocation; four
 * separate heap-buffer-overflows of this port came from that confusion.
 * The tag makes "what is this REF really" a question with an answer, and
 * with an explicit "I don't know" for anything untagged.
 *
 * CHOICE OF MAGIC VALUES. Not 0/1/2: this tag sits at offset 0, exactly
 * where each struct's first pointer member used to live, so a mis-cast
 * REF (or a stale/garbage word) is read AS a tag -- and small integers
 * are overwhelmingly the most likely bit patterns to find in real program
 * data (counts, indices, flags, enum values, the low half of a small heap
 * pointer). These values are instead full-width 32-bit constants sharing
 * a deliberately unusual 0x9E5C prefix: they are non-zero, outside every
 * plausible count/index range, not ASCII text, not a plausible aligned
 * pointer half, and differ from each other in the low half so a
 * partially-overwritten tag reads as UNKNOWN rather than as a different
 * one of the three. Hitting one by accident is a 1-in-2^32 coincidence,
 * and the sole consequence would be the blind cast the code did before
 * this header existed. */

typedef unsigned int pico8_type_tag_t;

#define PICO8_TYPE_TAG_UNKNOWN 0x00000000u
#define PICO8_TYPE_TAG_LIST 0x9E5C1157u
#define PICO8_TYPE_TAG_OB_TABLE 0x9E5C0B7Au
#define PICO8_TYPE_TAG_INSTANCE 0x9E5C1451u

/* Reports which of the three tagged shapes `ref` really points at, or
 * PICO8_TYPE_TAG_UNKNOWN for NULL and for anything whose first 4 bytes are
 * not one of the three magic values (an untagged struct, a
 * pico8_vec_t/pico8_qz_box_t/... REF, a stale pointer).
 *
 * Read through memcpy rather than a struct cast so this stays well-defined
 * even when `ref` does NOT point at one of the three types -- which is
 * precisely the case the caller is trying to detect. Callers must only
 * pass a REF to an object of at least 4 bytes; every REF-carrying shape in
 * this port is far larger. */
static inline pico8_type_tag_t pico8_type_tag_of(const void *ref) {
    pico8_type_tag_t tag;

    if (ref == NULL) {
        return PICO8_TYPE_TAG_UNKNOWN;
    }
    memcpy(&tag, ref, sizeof tag);
    if (tag == PICO8_TYPE_TAG_LIST || tag == PICO8_TYPE_TAG_OB_TABLE || tag == PICO8_TYPE_TAG_INSTANCE) {
        return tag;
    }
    return PICO8_TYPE_TAG_UNKNOWN;
}

#endif /* PICO8_TYPE_TAG_H */
