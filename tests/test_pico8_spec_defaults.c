/* The spec defaults of the 7 modules outside the `jo` tree that still parse
 * one, read only through the shared readers spec_field()/is_ready() and a
 * fresh struct of each; the interface classes, whose defaults are generated,
 * each against their own data. test_spec_defaults() and
 * test_struct_defaults() hold values frozen on the code where these were
 * still classes and must not be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_wormhole.h"

#define READY_HASH 0x05313C64u
#define SPEC_HASH 0x067B24A6u
#define STRUCT_HASH 0xEA770012u

enum {
    M_TRADE,
    M_HY,
    M_HP,
    M_PJ,
    M_IE,
    M_G_,
    M_FV,
    MODULE_COUNT
};

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static int spec_field(int i, const char *name, pico8_value_t *out) {
    switch (i) {
    case M_TRADE:
        return pico8_trade_default(name, out);
    case M_HY:
        return pico8_hy_default(name, out);
    case M_HP:
        return pico8_hp_default(name, out);
    case M_PJ:
        return pico8_pj_default(name, out);
    case M_IE:
        return pico8_ie_default(name, out);
    case M_G_:
        return pico8_g__default(name, out);
    default:
        return pico8_fv_default(name, out);
    }
}

static int is_ready(int i) {
    switch (i) {
    case M_TRADE:
        return pico8_trade_ready();
    case M_HY:
        return pico8_hy_ready();
    case M_HP:
        return pico8_hp_ready();
    case M_PJ:
        return pico8_pj_ready();
    case M_IE:
        return pico8_ie_ready();
    case M_G_:
        return pico8_g__ready();
    default:
        return pico8_fv_ready();
    }
}

static void *build(int i) {
    switch (i) {
    case M_TRADE:
        return pico8_trade_new(NULL);
    case M_HY:
        return pico8_hy_new(NULL);
    case M_HP:
        return pico8_hp_new(NULL);
    case M_PJ:
        return pico8_pj_new(NULL);
    case M_IE:
        return pico8_ie_new(NULL);
    case M_G_:
        return pico8_g__new(NULL);
    default:
        return pico8_fv_new(NULL);
    }
}

static void shutdown_of(int i) {
    switch (i) {
    case M_TRADE:
        pico8_trade_shutdown();
        break;
    case M_HY:
        pico8_economy_shutdown();
        break;
    case M_HP:
        pico8_research_shutdown();
        break;
    case M_PJ:
        pico8_score_panel_shutdown();
        break;
    case M_IE:
        pico8_research_icon_shutdown();
        break;
    case M_G_:
        pico8_probe_shutdown();
        break;
    default:
        pico8_help_shutdown();
        break;
    }
}

/* Every key of the 7 specs, ke's two, and names absent from all of them. */
static const char *const NAMES[] = {
    "mr", "gp",  "fk", "fa", "ea", "gs", "cr",        "i_", "ou", "ed", "hd", "l",  "r",  "ra", "v",  "ma", "p",
    "pc", "nu",  "rh", "ql", "le", "hw", "kq",        "id", "eh", "pl", "me", "dw", "dq", "ik", "oi", "od", "fp",
    "kk", "jj",  "k_", "nh", "fx", "py", "ki",        "pn", "pk", "fc", "qo", "ro", "qg", "q_", "du", "hi", "ca",
    "gq", "eo",  "ff", "fg", "rb", "t",  "spawn_snd", "fz", "zz", "lq", "ev", "ol", "kj",
};

#define NAME_COUNT ((int)(sizeof NAMES / sizeof NAMES[0]))

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static uint32_t fold_string(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFFFFFFFu);
}

/* A REF folds as its tag, and its entry count when it is a table or a
 * list: never its address. */
static uint32_t fold_value(uint32_t h, pico8_value_t v) {
    h = fnv(h, (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_STRING:
        return fold_string(h, v.as.string);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0u);
        }
        h = fnv(h, (uint32_t)pico8_type_tag_of(v.as.ref) + 1u);
        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE) {
            h = fnv(h, (uint32_t)((const pico8_ob_table_t *)v.as.ref)->count);
        } else if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST) {
            h = fnv(h, (uint32_t)((const pico8_list_t *)v.as.ref)->count);
        }
        return h;
    default:
        return h;
    }
}

static int same_value(pico8_value_t a, pico8_value_t b) {
    if (a.kind != b.kind) {
        return 0;
    }
    switch (a.kind) {
    case PICO8_VALUE_FIXED:
        return a.as.fixed == b.as.fixed;
    case PICO8_VALUE_STRING:
        return a.as.string == b.as.string;
    case PICO8_VALUE_BOOL:
        return a.as.boolean == b.as.boolean;
    case PICO8_VALUE_REF:
        return a.as.ref == b.as.ref;
    default:
        return 0;
    }
}

static int tags_equal(const pico8_ob_table_t *t, const pico8_str_seq_t *tags) {
    int j;

    if (t->count != tags->count) {
        return 0;
    }
    for (j = 0; j < tags->count; j++) {
        if (t->entries[j].key.kind != PICO8_OB_KEY_INDEX || t->entries[j].value.kind != PICO8_VALUE_STRING ||
            strcmp(t->entries[j].value.as.string, tags->items[j]) != 0) {
            return 0;
        }
    }
    return 1;
}

static int ints_equal(const pico8_ob_table_t *t, const pico8_int_seq_t *seq) {
    int j;

    if (t->count != seq->count) {
        return 0;
    }
    for (j = 0; j < seq->count; j++) {
        if (t->entries[j].key.kind != PICO8_OB_KEY_INDEX || t->entries[j].value.kind != PICO8_VALUE_FIXED ||
            t->entries[j].value.as.fixed != pico8_fixed_from_int(seq->items[j])) {
            return 0;
        }
    }
    return 1;
}

/* Entry by entry, nested tables included; strings by content. */
static int tables_same(const pico8_ob_table_t *a, const pico8_ob_table_t *b) {
    int j;

    if (a->count != b->count) {
        return 0;
    }
    for (j = 0; j < a->count; j++) {
        const pico8_ob_entry_t *x = &a->entries[j];
        const pico8_ob_entry_t *y = &b->entries[j];

        if (x->key.kind != y->key.kind || (x->key.kind == PICO8_OB_KEY_INDEX && x->key.index != y->key.index) ||
            (x->key.kind == PICO8_OB_KEY_NAME && strcmp(x->key.name, y->key.name) != 0) ||
            x->value.kind != y->value.kind) {
            return 0;
        }
        if ((x->value.kind == PICO8_VALUE_FIXED && x->value.as.fixed != y->value.as.fixed) ||
            (x->value.kind == PICO8_VALUE_BOOL && x->value.as.boolean != y->value.as.boolean) ||
            (x->value.kind == PICO8_VALUE_STRING && strcmp(x->value.as.string, y->value.as.string) != 0) ||
            (x->value.kind == PICO8_VALUE_REF && !tables_same(x->value.as.ref, y->value.as.ref))) {
            return 0;
        }
    }
    return 1;
}

static int row_equal(const pico8_ob_table_t *t, const pico8_jb_row_t *row) {
    pico8_ob_table_t *view = pico8_jb_row_view(row);
    int equal = tables_same(t, view);

    pico8_ob_table_free(view);
    return equal;
}

/* same_value(), plus the converted table members of a module whose struct
 * took them from its spec: a spec table and the struct's untagged REF are
 * the same default when that REF is the data generated from the spec that
 * answered, whose content must then agree. */
static int same_default(int i, const char *name, pico8_value_t spec, pico8_value_t made) {
    if (spec.kind == PICO8_VALUE_REF && made.kind == PICO8_VALUE_REF && spec.as.ref != NULL &&
        pico8_type_tag_of(spec.as.ref) == PICO8_TYPE_TAG_OB_TABLE && made.as.ref != spec.as.ref) {
        if ((i == M_G_ || i == M_FV) && strcmp(name, "nu") == 0 && made.as.ref == &pico8_g__nu_tags) {
            assert(tags_equal(spec.as.ref, &pico8_g__nu_tags));
            return 1;
        }
        if (i == M_G_ && strcmp(name, "k_") == 0 && made.as.ref == &pico8_g__k__seq) {
            assert(ints_equal(spec.as.ref, &pico8_g__k__seq));
            return 1;
        }
        if (i == M_FV && strcmp(name, "k_") == 0 && made.as.ref == &pico8_fv_k__seq) {
            assert(ints_equal(spec.as.ref, &pico8_fv_k__seq));
            return 1;
        }
        if (i == M_FV && strcmp(name, "rb") == 0 && made.as.ref == &pico8_fv_rb_row) {
            assert(row_equal(spec.as.ref, &pico8_fv_rb_row));
            return 1;
        }
        if (i == M_HP && strcmp(name, "mr") == 0 && made.as.ref == &pico8_hp_mr_names) {
            assert(tags_equal(spec.as.ref, &pico8_hp_mr_names));
            return 1;
        }
        if (i == M_HP && strcmp(name, "gp") == 0 && made.as.ref == &pico8_hp_gp_names) {
            assert(tags_equal(spec.as.ref, &pico8_hp_gp_names));
            return 1;
        }
        if (i == M_HP && strcmp(name, "fk") == 0 && made.as.ref == &pico8_hp_fk_row) {
            assert(row_equal(spec.as.ref, &pico8_hp_fk_row));
            return 1;
        }
        if (i == M_PJ && strcmp(name, "gs") == 0 && made.as.ref == &pico8_pj_gs_row) {
            assert(row_equal(spec.as.ref, &pico8_pj_gs_row));
            return 1;
        }
        if (i == M_PJ && strcmp(name, "i_") == 0 && made.as.ref == &pico8_pj_i__row) {
            assert(row_equal(spec.as.ref, &pico8_pj_i__row));
            return 1;
        }
    }
    return same_value(spec, made);
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static uint32_t fold_ready(uint32_t h) {
    int i;
    for (i = 0; i < MODULE_COUNT; i++) {
        h = fnv(h, (uint32_t)is_ready(i));
    }
    return h;
}

/* Each module's defaults by name, its parent's included, absent included. */
static void test_spec_defaults(void) {
    uint32_t h = 2166136261u;
    int i;
    int n;

    for (i = 0; i < MODULE_COUNT; i++) {
        for (n = 0; n < NAME_COUNT; n++) {
            pico8_value_t v = pico8_value_fixed(0x5A5A);
            int found = spec_field(i, NAMES[n], &v);

            h = fnv(h, (uint32_t)found);
            if (found) {
                h = fold_value(h, v);
            } else {
                assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x5A5A);
            }
        }
        h = fnv(h, (uint32_t)i);
    }
    check("spec", h, SPEC_HASH);
}

/* A fresh struct of each module against its defaults: which names it
 * serves, and which of them are the very value or REF of the spec. */
static void test_struct_defaults(void) {
    uint32_t h = 2166136261u;
    int i;
    int n;

    for (i = 0; i < MODULE_COUNT; i++) {
        void *made = build(i);
        int readable = made != NULL && ((const pico8_struct_header_t *)made)->cls->field != NULL;

        h = fnv(h, (uint32_t)(made != NULL));
        h = fnv(h, (uint32_t)readable);
        for (n = 0; readable && n < NAME_COUNT; n++) {
            pico8_value_t from_spec = pico8_value_fixed(0x5A5A);
            pico8_value_t from_struct = pico8_value_fixed(0x5A5A);
            int in_spec = spec_field(i, NAMES[n], &from_spec);
            int in_struct = pico8_entity_get_field(made, NAMES[n], &from_struct);

            h = fnv(h, (uint32_t)in_spec);
            h = fnv(h, (uint32_t)in_struct);
            h = fnv(h, (uint32_t)(in_spec && in_struct && same_default(i, NAMES[n], from_spec, from_struct)));
        }
        h = fnv(h, (uint32_t)i);
    }
    check("struct", h, STRUCT_HASH);
}

static pico8_ob_table_t *nf_l_of(void *nf) {
    pico8_value_t v;
    assert(pico8_entity_get_field(nf, "l", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

/* The second `jl()` (the one `eu` runs) finds `qb`, `nf` and `ui` ready and
 * leaves their state alone: a mark written into `nf`'s `l` and into the
 * shared `le` survives it; `qb` stays ready throughout. */
static void test_second_jl_keeps_specs(void) {
    pico8_ob_table_t *nf_l = nf_l_of(pico8_je());
    pico8_list_t *ui_le = pico8_ui_le_shared();
    pico8_value_t mark = pico8_value_fixed(0x7777);
    pico8_value_t v;
    int ui_count;

    assert(pico8_qb_ready());
    pico8_ob_table_set_name(nf_l, "mark", mark);
    pico8_list_add(ui_le, mark);
    ui_count = ui_le->count;

    pico8_jl();

    assert(pico8_qb_ready());
    assert(nf_l_of(pico8_je()) == nf_l && pico8_ob_table_get_name(nf_l_of(pico8_je()), "mark", &v));
    assert(pico8_ui_le_shared() == ui_le && pico8_ui_le_shared()->count == ui_count);
    assert(pico8_list_del(ui_le, mark));
}

/* A fresh bg starts on bg's generated defaults and ke's; ready until its
 * shutdown. */
static void test_bg_defaults_and_ready(void) {
    pico8_bg_t *bg = pico8_bg_new(NULL);

    assert(bg->fa == pico8_bg_start.fa && bg->hd == pico8_bg_start.hd);
    assert(bg->pl == pico8_bg_start.pl && bg->pl == &pico8_bg_pl_seq);
    assert(strcmp(bg->ke.me, "ph") == 0 && bg->ke.t == 0);
    assert(pico8_bg_ready());
    pico8_bg_shutdown();
    assert(!pico8_bg_ready());
}

static void test_js_defaults_and_ready(void) {
    pico8_js_t *js = pico8_js_new(NULL);

    assert(js->fa == pico8_js_start.fa);
    assert(strcmp(js->ke.me, "ph") == 0 && js->ke.t == 0);
    assert(pico8_js_ready());
    pico8_js_shutdown();
    assert(!pico8_js_ready());
}

/* `v`, `p` and `ra` are the generated vectors themselves. */
static void test_qb_defaults_and_ready(void) {
    pico8_qb_t *qb = pico8_qb_new(NULL);

    assert(qb->v == &pico8_qb_start.v && qb->p == &pico8_qb_start.p && qb->ra == &pico8_qb_start.ra);
    assert(qb->ma == pico8_qb_start.ma && qb->pc == pico8_qb_start.pc && qb->pc == &pico8_qb_pc_vecs);
    assert(strcmp(qb->ke.me, "ph") == 0 && qb->ke.t == 0);
    assert(pico8_qb_ready());
    pico8_qb_shutdown();
    assert(!pico8_qb_ready());
}

/* mz's own `me` replaces ke's; `jj` holds the generated `qs`. */
static void test_mz_defaults_and_ready(void) {
    pico8_mz_t *mz = pico8_mz_new(NULL);

    assert(mz->ke.me == pico8_mz_start.me && strcmp(mz->ke.me, "kz") == 0 && mz->ke.t == 0);
    assert(mz->nu == pico8_mz_start.nu && mz->kk == pico8_mz_start.kk);
    assert(mz->dw == pico8_mz_start.dw && mz->dq == pico8_mz_start.dq);
    assert(mz->spawn_snd == pico8_mz_start.spawn_snd && mz->ik == pico8_mz_start.ik);
    assert(mz->oi == pico8_mz_start.oi);
    assert(mz->od == pico8_mz_start.od && mz->od == pico8_fixed_from_int(3));
    assert(mz->fp == pico8_mz_start.fp && mz->fa == pico8_mz_start.fa);
    assert(mz->jj == pico8_mz_jj_wrapper() && mz->jj->qs == pico8_mz_jj_qs);
    assert(pico8_mz_ready());
    pico8_wormhole_shutdown();
    assert(!pico8_mz_ready());
}

static void test_layout_ready(void) {
    assert(pico8_lq_ready() && pico8_jb_ready());
    pico8_layout_shutdown();
    assert(!pico8_lq_ready() && !pico8_jb_ready());
}

int main(void) {
    uint32_t ready = 2166136261u;
    int i;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    ready = fold_ready(ready);
    pico8_cart_load();
    ready = fold_ready(ready);
    pico8_jl();
    ready = fold_ready(ready);

    test_spec_defaults();
    test_struct_defaults();
    test_second_jl_keeps_specs();
    test_bg_defaults_and_ready();
    test_js_defaults_and_ready();
    test_qb_defaults_and_ready();
    test_mz_defaults_and_ready();
    test_layout_ready();

    for (i = MODULE_COUNT - 1; i >= 0; i--) {
        shutdown_of(i);
        ready = fold_ready(ready);
    }
    check("ready", ready, READY_HASH);

    printf("test_pico8_spec_defaults: all tests passed\n");
    return 0;
}
