#ifndef PICO8_LAYOUT_H
#define PICO8_LAYOUT_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* `lq` and `jb` are root classes declared with no spec string and are not
 * ke-descended, so neither ever self-registers into hv/cv/cg. */
void pico8_layout_init(void);
void pico8_layout_shutdown(void);

/* NULL before pico8_layout_init() and after pico8_layout_shutdown(). Neither
 * class is retrievable through pico8_class_lookup(): both register under the
 * shared "" name-registry slot. */
pico8_class_t *pico8_lq_class(void);
pico8_class_t *pico8_jb_class(void);

pico8_instance_t *pico8_lq_new(const pico8_ob_table_t *overrides);
pico8_instance_t *pico8_jb_new(const pico8_ob_table_t *overrides);

/* Omitting `a` is numerically the same as `a` of 0, so pico8_lq_ol_default
 * passes 0 rather than carrying a "was it provided" flag. A NULL self draws
 * nothing. */
void pico8_lq_ol(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p,
                  pico8_fixed_t a);
void pico8_lq_ol_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p);

/* A NULL self, or one whose "ls" field is absent, draws nothing -- not even
 * the frame. */
void pico8_jb_ol(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_fixed_t om, pico8_fixed_t a);

#endif
