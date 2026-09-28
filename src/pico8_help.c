#include "pico8_help.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_msgbox.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_type_tag.h"

static pico8_class_t *g_fv_class = NULL;
static pico8_ob_table_t *g_bo = NULL;

/* Returns a new, heap-backed list the caller owns and must release with
 * pico8_list_free(). */
static pico8_list_t pico8_help_positional_values(const pico8_ob_table_t *table) {
    pico8_list_t list;
    int i;

    pico8_list_init(&list);
    if (table == NULL) {
        return list;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_list_add(&list, table->entries[i].value);
        }
    }
    return list;
}

static pico8_value_t pico8_fv_eu_placeholder(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(NULL);
}

static pico8_value_function_t g_eu_dispatch = pico8_fv_eu_placeholder;

pico8_value_t pico8_eu_field_value(void) {
    return pico8_value_function(g_eu_dispatch);
}

void pico8_help_set_eu(pico8_value_function_t fn) {
    g_eu_dispatch = (fn != NULL) ? fn : pico8_fv_eu_placeholder;
}

static pico8_value_t pico8_fv_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_instance_t *)self, arg0, arg1);
}

void pico8_fv_pq(pico8_instance_t *self) {
    int i;

    if (g_bo != NULL) {
        for (i = 0; i < g_bo->count; i++) {
            if (g_bo->entries[i].key.kind == PICO8_OB_KEY_INDEX && g_bo->entries[i].value.kind == PICO8_VALUE_REF &&
                g_bo->entries[i].value.as.ref != NULL) {
                pico8_ob_table_t *d = (pico8_ob_table_t *)g_bo->entries[i].value.as.ref;
                assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_OB_TABLE && "bo's rows must be plain spec tables");
                pico8_ob_table_set_name(d, "my", pico8_eu_field_value());
            }
        }
    }

    if (self != NULL) {
        pico8_list_t items = pico8_help_positional_values(g_bo);
        pico8_pz(self, &items);
        pico8_list_free(&items);
    }
}

void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p) {
    pico8_value_t rb_val;
    const pico8_ob_table_t *rb_table = NULL;

    (void)p;

    pico8_ei(gfx, mem, self);

    if (self != NULL && pico8_instance_get_field(self, "rb", &rb_val) && rb_val.kind == PICO8_VALUE_REF &&
        rb_val.as.ref != NULL) {
        assert(pico8_type_tag_of(rb_val.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
               "fv's rb field must hold a plain spec table");
        rb_table = (const pico8_ob_table_t *)rb_val.as.ref;
    }

    if (gfx != NULL && mem != NULL) {
        pico8_list_t items = pico8_help_positional_values(rb_table);
        pico8_bl_default(gfx, mem, &items);
        pico8_list_free(&items);
    }
}

pico8_value_t pico8_fv_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

/* This module, unlike most, does not call pico8_pi_init() itself, and `fv`
 * extends `g_`, not `ke`: both preconditions are asserted, not repaired. */
void pico8_help_init(void) {
    assert(pico8_ob_lookup_constructor('v') != NULL && "pico8_help_init() runs after pico8_pi_init()");
    assert(pico8_g__class() != NULL && "pico8_help_init() runs after pico8_probe_init()");

    pico8_help_shutdown();

    g_bo = pico8_ob("\no(lq=o(49,\"forgiving   \"),px=\"forgiving\",\ntrade=7,en=-0.99),\n"
                     "o(lq=o(50,\"reasonable  \"),px=\"reasonable\",\ntrade=6,en=-0.6),\n"
                     "o(lq=o(51,\"challenging \"),px=\"challenging\",\ntrade=5,en=-0.4),\n"
                     "o(lq=o(52,\"tough       \"),px=\"tough\",\ntrade=4,en=-0.2),\n",
                     NULL);

    g_fv_class = pico8_class_declare(
        pico8_g__class(),
        "\nfa=10,ra=v(64,28),\nhi=v(34,56),\nca=11,\ngq=v(0,11),\neo=-1,\nff=1,\nfc=5,"
        "\nk_=o(0,1,1,2,4,9,10,9,4,2,1,1,1),\nqo=1.02,ro=-0.007,\nqg=0.007,q_=1.02,"
        "\npn=5,du=15,\npy=0.0067,pk=1,\nki=17,\nkq=b(-40,78,40,98),\nql=11,fg=5,"
        "\nrb=o(\no(192,0,11,16,4,fn=\"s\"),\no(172,20,107,2,2,fn=\"s\"),"
        "\no(\"need help?\",40,110,13,fn=\"qr\"),\no(\"slipways.net/help\",40,116,5,fn=\"qr\"),\n),\n");
    pico8_class_set_pq(g_fv_class, pico8_fv_pq);
    pico8_ob_table_set_name(g_fv_class->defaults, "hd", pico8_value_function(pico8_fv_hd_dispatch));
    pico8_ob_table_set_name(g_fv_class->defaults, "kj", pico8_value_function(pico8_fv_kj_dispatch));

    pico8_help_set_eu(NULL);
}

void pico8_help_shutdown(void) {
    if (g_fv_class != NULL) {
        pico8_class_free(g_fv_class);
        g_fv_class = NULL;
    }
    if (g_bo != NULL) {
        pico8_ob_table_free(g_bo);
        g_bo = NULL;
    }
    pico8_help_set_eu(NULL);
}

pico8_class_t *pico8_fv_class(void) {
    return g_fv_class;
}

pico8_instance_t *pico8_fv_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_fv_class, overrides);
}
