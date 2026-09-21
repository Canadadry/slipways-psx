#ifndef PICO8_TYPE_TAG_H
#define PICO8_TYPE_TAG_H

#include <string.h>

/* A real, first-member type discriminator for the three mutually
 * confusable heap shapes this port uses to model Lua's ONE table type.
 *
 * WHY THIS EXISTS. `pico8_value_t`'s REF kind (pico8_value.h) is a bare
 * `void *`: it carries no type of its own, exactly like a Lua value whose
 * type is known only from context. In real Lua that is harmless, because
 * `pairs()`/`add()`/`#` all work on the single table type. This port
 * models that one Lua type with THREE distinct C structs:
 *
 *   pico8_list_t      (pico8_list.h)  {items;count;capacity}, 16-byte
 *                                     pico8_value_t elements
 *   pico8_ob_table_t  (pico8_ob.h)    {entries;count;capacity}, 32-byte
 *                                     pico8_ob_entry_t elements
 *   pico8_instance_t  (pico8_class.h) {fields;cls}
 *
 * All three were, before this tag, {pointer; 8 more bytes} = 16 bytes,
 * so a cast between any two of them was silently accepted by the
 * compiler AND produced a plausible-looking object -- while the element
 * strides differ by 2x. Reading a `pico8_list_t` as a `pico8_ob_table_t`
 * therefore walks its backing array at double stride past the end of the
 * allocation: a real heap-buffer-overflow, hit FOUR separate times during
 * this port (`pico8_gn`, `mz:hq`, `js:pq`'s own "jb" field, and finally
 * `js:pq`'s own "lq" field -- the one the pre-Phase-5 cast audit,
 * docs/plan.md's own "Notes de portage" journal, had marked safe).
 *
 * Every previous fix was local (convert at the one producing call site) or
 * heuristic (sniff `->cls` against a known class pointer, which only ever
 * distinguished instance-vs-table and let a list through). This header is
 * the structural answer instead: every one of the three shapes carries an
 * explicit tag as its FIRST member, so a REF can be asked what it really
 * is, with an explicit "I don't know" answer for anything untagged.
 *
 * CHOICE OF MAGIC VALUES. Not 0/1/2: this tag sits at offset 0, exactly
 * where each struct's first pointer member used to live, so a mis-cast
 * REF (or a stale/garbage word) is read as a tag, and small integers are
 * overwhelmingly the most likely bit patterns to find in real program
 * data (counts, indices, flags, enum values, the low half of a small
 * heap pointer). These values are instead full-width 32-bit constants
 * sharing a deliberately unusual 0x9E5C prefix: they are non-zero,
 * outside every plausible count/index range, not ASCII text, not a
 * plausible aligned pointer half (all three are odd or end in a nibble no
 * 4/8-byte-aligned allocation would), and differ from each other in the
 * low half so a partially-overwritten tag reads as UNKNOWN rather than as
 * a different one of the three. Hitting one by accident is a 1-in-2^32
 * coincidence, and the sole consequence would be the same blind cast the
 * code already did before this header existed.
 *
 * Deliberately header-only (`static inline`, no pico8_type_tag.c): this is
 * a 4-byte load and three comparisons, and keeping it out of every
 * Makefile target's own source list keeps the build wiring unchanged. */

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
