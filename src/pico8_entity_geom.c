/* pico8/lua.lua's live-entity vector-field helpers -- gk, gd, na, ma, ij --
 * see pico8_entity_geom.h's own top comment for the shared "vector field
 * == PICO8_VALUE_REF to a heap pico8_vec_t" convention this file
 * establishes, and for the field-absence defensive-default policy (mirrors
 * pico8_entity.c's own dc()/he() precedent) every helper below follows. */

#include "pico8_entity_geom.h"

#include <stddef.h>

#include "pico8_entity.h"
#include "pico8_geom.h"

static pico8_vec_t pico8_entity_geom_zero(void) {
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* Reads a field expected to hold a vector (PICO8_VALUE_REF to a heap
 * pico8_vec_t). Absent/wrong-kind/NULL-instance all read as the zero
 * vector -- see this file's top comment for why. */
static pico8_vec_t pico8_entity_geom_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_entity_geom_zero();
}

/* Same defensive read as pico8_entity_geom_field_vec above, but for a
 * pico8_value_t already in hand (mv()/dc()'s own extra-argument values,
 * e.g. gk's `mp`/`rp`) rather than one fetched from a field by name. */
static pico8_vec_t pico8_entity_geom_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_entity_geom_zero();
}

/* Reads a field expected to hold a plain PICO8_VALUE_FIXED number.
 * Absent/wrong-kind/NULL-instance all read as 0 -- see this file's top
 * comment for why. */
static pico8_fixed_t pico8_entity_geom_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

/* function gk(e,mp,rp)
 * return e.kq((e.fa>=10 and rp or mp)-e.ra
 * )end
 * See pico8_entity_geom.h's own comment on this declaration. A missing/
 * non-REF `.kq` field returns false (pico8_value_bool(0)) rather than
 * crashing on a nil call, matching this file's own defensive-default
 * policy -- not exercised by any real call site until Chunk 4's js
 * class exists. */
pico8_value_t pico8_gk(const pico8_instance_t *e, pico8_value_t mp, pico8_value_t rp) {
    pico8_fixed_t fa = pico8_entity_geom_field_fixed(e, "fa");
    pico8_vec_t ra = pico8_entity_geom_field_vec(e, "ra");
    pico8_value_t chosen = (fa >= pico8_fixed_from_int(10)) ? rp : mp;
    pico8_vec_t point = pico8_vec_sub(pico8_entity_geom_value_vec(chosen), ra);
    pico8_value_t kq;

    if (e != NULL && pico8_instance_get_field(e, "kq", &kq) && kq.kind == PICO8_VALUE_REF && kq.as.ref != NULL) {
        return pico8_value_bool(pico8_qz_call((const pico8_qz_box_t *)kq.as.ref, point));
    }
    return pico8_value_bool(0);
}

/* function gd(e)
 * return abs(e.ra.x-mo.ra.x-64)<=76 and abs(e.ra.y-mo.ra.y-64)<=76
 * end
 * See pico8_entity_geom.h's own comment on this declaration for the
 * mo-not-yet-real forward-reference caveat. */
int pico8_gd(const pico8_instance_t *e) {
    pico8_vec_t e_ra = pico8_entity_geom_field_vec(e, "ra");
    pico8_vec_t mo_ra = pico8_entity_geom_field_vec(pico8_mo(), "ra");
    pico8_fixed_t sixty_four = pico8_fixed_from_int(64);
    pico8_fixed_t seventy_six = pico8_fixed_from_int(76);

    return pico8_fixed_abs(pico8_fixed_sub(pico8_fixed_sub(e_ra.x, mo_ra.x), sixty_four)) <= seventy_six &&
           pico8_fixed_abs(pico8_fixed_sub(pico8_fixed_sub(e_ra.y, mo_ra.y), sixty_four)) <= seventy_six;
}

/* function na(self,t)
 * self.ra.y+=sin(t*0.005)*0.06
 * end
 * See pico8_entity_geom.h's own comment on this declaration for why this
 * mutates the pointee of self.ra's own heap pico8_vec_t in place rather
 * than rebinding the field. A missing/non-REF `.ra` field is a no-op
 * (nothing to mutate), matching this file's own defensive-default policy. */
void pico8_na(pico8_instance_t *self, pico8_fixed_t t) {
    pico8_value_t ra;
    if (self != NULL && pico8_instance_get_field(self, "ra", &ra) && ra.kind == PICO8_VALUE_REF && ra.as.ref != NULL) {
        pico8_vec_t *ra_vec = (pico8_vec_t *)ra.as.ref;
        pico8_fixed_t delta = pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_mul(t, pico8_fixed_from_double(0.005))),
                                               pico8_fixed_from_double(0.06));
        ra_vec->y = pico8_fixed_add(ra_vec->y, delta);
    }
}

/* function ma(p,e)e.ma=max(0,e.ma-0.1)
 * return p+qm(e.ma,e.t*0.2)end
 * See pico8_entity_geom.h's own comment on this declaration. `e.ma=...` is
 * a plain field write -- goes straight onto e's own instance table
 * (e->fields), the same table pico8_instance_get_field would read back
 * from first on any later lookup, exactly mirroring Lua's own "field
 * writes always land on the instance, __index is read-only fallback"
 * semantics (pico8_class.h's own top comment). A NULL e is a no-op write
 * (nothing to write onto), matching this file's own defensive policy. */
pico8_vec_t pico8_ma(pico8_vec_t p, pico8_instance_t *e) {
    pico8_fixed_t ma = pico8_entity_geom_field_fixed(e, "ma");
    pico8_fixed_t decremented = pico8_fixed_sub(ma, pico8_fixed_from_double(0.1));
    pico8_fixed_t clamped = (decremented > 0) ? decremented : 0;
    pico8_fixed_t t = pico8_entity_geom_field_fixed(e, "t");

    if (e != NULL) {
        pico8_ob_table_set_name(e->fields, "ma", pico8_value_fixed(clamped));
    }
    return pico8_vec_add(p, pico8_vec_qm(clamped, pico8_fixed_mul(t, pico8_fixed_from_double(0.2))));
}

/* function ij(p,j_)local mk,nd=32767
 * for o in all(j_)do
 * local d=#(p-o.p)if p~=o.p and d<mk then
 * nd,mk=o,d
 * end
 * end
 * return mk,nd
 * end
 * See pico8_entity_geom.h's own comment on this declaration for why `.p`
 * (not `.ra`) is the field read here, and for *out_nearest's
 * pico8_value_ref(NULL)-means-nil convention. */
pico8_fixed_t pico8_ij(pico8_vec_t p, const pico8_list_t *j_, pico8_value_t *out_nearest) {
    pico8_fixed_t mk = pico8_fixed_from_int(32767);
    pico8_list_iter_t it;
    pico8_value_t e;

    *out_nearest = pico8_value_ref(NULL);
    it = pico8_list_all(j_);
    while (pico8_list_iter_next(&it, &e)) {
        pico8_instance_t *o = (pico8_instance_t *)e.as.ref;
        pico8_vec_t o_p = pico8_entity_geom_field_vec(o, "p");
        pico8_fixed_t d = pico8_vec_len_sq(pico8_vec_sub(p, o_p));
        int same_position = (p.x == o_p.x && p.y == o_p.y);

        if (!same_position && d < mk) {
            mk = d;
            *out_nearest = e;
        }
    }
    return mk;
}

