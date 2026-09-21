#include "pico8_tech_gate.h"

#include <stddef.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_ob.h"
#include "pico8_probe.h"

/* Lua truthiness (nil/false falsy, everything else -- including fixed 0 --
 * truthy), extended to also treat PICO8_VALUE_REF(NULL) as falsy -- the
 * same extension pico8_probe.c's own pico8_probe_truthy already
 * established, needed here since `oj(px)`'s own `not px` can genuinely be
 * called with this port's "absent Lua value" filler. Private per-module
 * copy, matching this port's own pervasive per-module-truthy-helper
 * convention (pico8_ui.c, pico8_layout.c, pico8_bootstrap.c, ...). */
static int pico8_tech_gate_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Not a port of any named Lua function: `jp(nb,...)`'s own `nb` (real Lua
 * global built by pico8_data_init.c's own foreach(nb,ho(...)) pass) is a
 * plain ob()-parsed table whose real content is purely positional (each
 * entry one tech record) -- but pico8_jp (pico8_collect.h) takes a
 * pico8_list_t, not a pico8_ob_table_t. Same private-per-module bridge
 * already established independently by pico8_layout_positional_values
 * (pico8_layout.c) and pico8_msgbox_positional_values (pico8_msgbox.c) --
 * documented duplication, not a new pattern. Returns a new, heap-backed
 * list the caller owns and must release with pico8_list_free(). */
static pico8_list_t pico8_tech_gate_positional_values(const pico8_ob_table_t *table) {
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

/* function oj(px)
 * return not px or gu(fm.invented,px)
 * end
 * (pico8/lua.lua:1464-1466.) See pico8_tech_gate.h's own top comment on
 * this declaration for the full trace, including why pico8_gu_content is
 * used rather than pico8_gu. */
int pico8_oj(pico8_value_t px) {
    pico8_value_t invented;
    const pico8_list_t *invented_list = NULL;

    if (!pico8_tech_gate_truthy(px)) {
        return 1;
    }
    if (pico8_fm() != NULL && pico8_instance_get_field(pico8_fm(), "invented", &invented) &&
        invented.kind == PICO8_VALUE_REF) {
        invented_list = invented.as.ref;
    }
    return pico8_gu_content(invented_list, px);
}

/* function ov(px,lk,mc)
 * return oj(px)
 * and mc or lk
 * end
 * (pico8/lua.lua:1467-1470.) See pico8_tech_gate.h's own top comment on
 * this declaration for the full A-and-B-or-C trace. */
pico8_value_t pico8_ov(pico8_value_t px, pico8_value_t lk, pico8_value_t mc) {
    if (pico8_oj(px) && pico8_tech_gate_truthy(mc)) {
        return mc;
    }
    return lk;
}

/* function gi(t)
 * return flr(t.od*max(0.55,
 * fm.fu[t.nk]))
 * end
 * (pico8/lua.lua:1471-1474.) See pico8_tech_gate.h's own top comment on
 * this declaration for the full trace, including why `t` is a
 * pico8_ob_table_t* (a plain nb record), not an instance, and the
 * `fm.fu[t.nk]` polymorphic-key lookup. */
pico8_fixed_t pico8_gi(const pico8_ob_table_t *t) {
    pico8_value_t od_val;
    pico8_value_t nk_val;
    pico8_value_t fu_val;
    pico8_fixed_t od = pico8_fixed_from_int(0);
    pico8_fixed_t fu = pico8_fixed_from_int(0);
    pico8_fixed_t threshold = pico8_fixed_from_double(0.55);
    pico8_fixed_t factor;

    if (t != NULL && pico8_ob_table_get_name(t, "od", &od_val) && od_val.kind == PICO8_VALUE_FIXED) {
        od = od_val.as.fixed;
    }

    if (t != NULL && pico8_ob_table_get_name(t, "nk", &nk_val) && pico8_fm() != NULL &&
        pico8_instance_get_field(pico8_fm(), "fu", &fu_val) && fu_val.kind == PICO8_VALUE_REF &&
        fu_val.as.ref != NULL) {
        const pico8_ob_table_t *fu_table = fu_val.as.ref;
        pico8_value_t v;
        int found = 0;

        if (nk_val.kind == PICO8_VALUE_FIXED) {
            found = pico8_ob_table_get_index(fu_table, (int)pico8_fixed_to_double(nk_val.as.fixed), &v);
        } else if (nk_val.kind == PICO8_VALUE_STRING && nk_val.as.string != NULL) {
            found = pico8_ob_table_get_name(fu_table, nk_val.as.string, &v);
        }
        if (found && v.kind == PICO8_VALUE_FIXED) {
            fu = v.as.fixed;
        }
    }

    factor = (fu > threshold) ? fu : threshold;
    return pico8_fixed_flr(pico8_fixed_mul(od, factor));
}

/* `cb(bi)`'s own ctx for its filter predicate below -- `bi`, the real Lua
 * closure's own captured upvalue, threaded here as an explicit ctx per
 * pico8_jp_predicate_fn's own established convention. */
typedef struct {
    pico8_value_t bi;
} pico8_cb_ctx_t;

/* `function(t)t.e_=gi(t)<=fm.jk return t.lv==fm.ew and not oj(t.px) and
 * (t.e_ or bi) end` (pico8/lua.lua:1456-1462) -- `cb`'s own real Lua
 * closure, called once per `nb` entry by pico8_jp below. See
 * pico8_tech_gate.h's own top comment on pico8_cb for the full trace,
 * including the real, unconditional `t.e_=...` side effect on every entry
 * visited (ported here exactly: the write happens before the kept/
 * not-kept decision, for every `t`, matching the real closure's own
 * statement order). */
static int pico8_cb_predicate(pico8_value_t e, void *ctx_ptr) {
    pico8_cb_ctx_t *ctx = ctx_ptr;
    pico8_ob_table_t *t;
    pico8_value_t jk_val;
    pico8_value_t ew_val;
    pico8_value_t lv_val;
    pico8_value_t px_val;
    pico8_fixed_t jk = pico8_fixed_from_int(0);
    pico8_fixed_t ew = pico8_fixed_from_int(0);
    int affordable;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    t = e.as.ref;

    if (pico8_fm() != NULL && pico8_instance_get_field(pico8_fm(), "jk", &jk_val) &&
        jk_val.kind == PICO8_VALUE_FIXED) {
        jk = jk_val.as.fixed;
    }
    affordable = pico8_gi(t) <= jk;
    pico8_ob_table_set_name(t, "e_", pico8_value_bool(affordable));

    if (pico8_fm() != NULL && pico8_instance_get_field(pico8_fm(), "ew", &ew_val) &&
        ew_val.kind == PICO8_VALUE_FIXED) {
        ew = ew_val.as.fixed;
    }
    if (!(pico8_ob_table_get_name(t, "lv", &lv_val) && lv_val.kind == PICO8_VALUE_FIXED &&
          lv_val.as.fixed == ew)) {
        return 0;
    }

    if (!pico8_ob_table_get_name(t, "px", &px_val)) {
        px_val = pico8_value_ref(NULL);
    }
    if (pico8_oj(px_val)) {
        return 0;
    }

    return affordable || pico8_tech_gate_truthy(ctx->bi);
}

/* function cb(bi)
 * return jp(nb,function(t)...end)
 * end
 * (pico8/lua.lua:1455-1463.) See pico8_tech_gate.h's own top comment on
 * this declaration for the full trace. */
pico8_list_t pico8_cb(pico8_value_t bi) {
    pico8_list_t items = pico8_tech_gate_positional_values(pico8_nb());
    pico8_cb_ctx_t ctx;
    pico8_list_t result;

    ctx.bi = bi;
    result = pico8_jp(&items, pico8_cb_predicate, &ctx);
    pico8_list_free(&items);
    return result;
}
