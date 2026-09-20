/* See pico8_data_init.h's own top comment for the full source/trace of the
 * "game data init" sequence ported here. */

#include "pico8_data_init.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_research.h"
#include "pico8_stash.h"

static pico8_ob_table_t *g_df = NULL;
static pico8_ob_table_t *g_hc = NULL;
static pico8_ob_table_t *g_fd = NULL;
static pico8_ob_table_t *g_dr = NULL;
static pico8_ob_table_t *g_nb = NULL;
static pico8_ob_table_t *g_ek = NULL;

/* Not a port of any named Lua function: private per-module bridge from
 * "positional entries of a pico8_ob_table_t" to a pico8_list_t (foreach()'s
 * own real Lua targets, `df`/`nb`, are purely positional at this point in
 * their own build), matching pico8_layout_positional_values'/
 * pico8_research_positional_values' own established per-module-duplicate
 * convention. Returns a new, heap-backed list the caller owns and must
 * release with pico8_list_free(). */
static pico8_list_t pico8_data_init_positional_values(const pico8_ob_table_t *table) {
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

/* `foreach(df,ho(df,ob([["px","s","r","d",]])))` (pico8/lua.lua:1389-1391)
 * / `foreach(nb,ho(nb,ob([["px","lv","nk","od","oy",]]),{"nk"}))`
 * (pico8/lua.lua:1442-1444) -- BOTH renaming passes share this exact same
 * shape (`rc` and the table being renamed ARE THE SAME TABLE, unlike
 * pico8_research_build_gb's own `gb`/`fd` split), so this one helper
 * serves both real call sites rather than duplicating the loop twice.
 * `rc`'s own positional entries are snapshotted into a pico8_list_t FIRST
 * (this module's own pico8_data_init_positional_values), immune to `rc`'s
 * own growth as pico8_ho_apply appends new NAMED entries into it via its
 * own `rc[ri.px]=ri` tail (`rc` here IS a bare pico8_ob_table_t, exactly
 * pico8_ho_apply's own built-in assumption -- unlike hp:pq's own `gb`
 * case, pico8_research.c, where `ri` is a real class instance and that
 * built-in tail can't be used directly). */
static void pico8_data_init_rename_pass(pico8_ob_table_t *rc, const pico8_ob_table_t *mr, const pico8_list_t *gp) {
    pico8_list_t items = pico8_data_init_positional_values(rc);
    pico8_list_iter_t it = pico8_list_all(&items);
    pico8_value_t entry;

    while (pico8_list_iter_next(&it, &entry)) {
        if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
            pico8_ho_apply(rc, mr, gp, (pico8_ob_table_t *)entry.as.ref);
        }
    }
    pico8_list_free(&items);
}

/* The full top-level, non-function statement sequence pico8/lua.lua runs
 * once at cart-load time (pico8/lua.lua:1377-1445), read as one continuous
 * block:
 *
 * r,iz=ob(unstash(8192)),ob(unstash(8259))df=ob([[
 * o("ll",45,0,me="ll"),
 * ... (11 records total, see below) ...
 * ]])foreach(df,ho(df,ob([[
 * "px","s","r","d",
 * ]])))hc=ob(unstash(8383))fd=ob(unstash(8494))function bj(jo)...end
 * hp=kv:jr([[...]])function hp:pq()...end ... (hp's own methods) ...
 * gb,rr={}
 * for ri in all(fd)do...end
 * dr=ob(unstash(9807))nb=ob(unstash(10457))foreach(nb,ho(nb,ob([["px","lv",
 * "nk","od","oy",]]),
 * {"nk"}
 * ))
 * ek=ob([[2,4,7,12,]])
 *
 * Per this PRD's own "Data-table initialization sequencing" Implementation
 * Decision (see this file's own top comment on pico8_data_init.h): ported
 * as the body of ONE explicit C function, `pico8_data_init()`, this
 * chunk's own "module load" step -- not folded into a future `_init()`/
 * `jl()`. `r`/`iz` themselves are Chunk 1's own job (pico8_stash.c/.h) --
 * NOT rebuilt here; `pico8_stash_init(mem)` is CALLED (idempotent, reused
 * exactly as Chunk 1 built it) as this function's own first step, matching
 * the real source's own statement order (`r,iz=...` textually precedes
 * `df=...` in the same unbroken sequence). `hp`'s own class + `jo.ex`/`bj`
 * (pico8_research.c/.h) are built via pico8_research_init()/
 * pico8_research_build_gb(), called here as two of this function's own
 * steps -- mirroring pico8_bootstrap.c's own pico8_jl(), which likewise
 * orchestrates other modules' sub-inits rather than inlining their
 * bodies. `df`'s own 11-record literal spec string and its `ho()` renaming
 * pass, `hc`/`fd`/`dr`/`nb`'s own `unstash()`+`ob()` decoding, `nb`'s own
 * `ho()` renaming pass (with its `{"nk"}` `gp` list), and `ek`'s own
 * literal are ALL ported directly in the body below, in the real source's
 * own order. Safe to call more than once (frees whatever a previous call
 * built first, via each `pico8_ob_table_free`/pico8_research_shutdown()
 * call below), matching pico8_entity_init()'s/pico8_stash_init()'s own
 * established re-init convention. */
void pico8_data_init(pico8_mem_t *mem) {
    pico8_ob_table_t *df_mr;
    pico8_ob_table_t *nb_mr;
    pico8_list_t nb_gp;
    char *decoded;

    pico8_stash_init(mem);

    /* df=ob([[...]]) (pico8/lua.lua:1377-1389) -- 11 records, each a
     * positional `o(px,s,r,[me=...])` planet-type descriptor; pasted here
     * verbatim (byte for byte, including the trailing comma every real
     * top-level ob() spec needs, per pico8_ob.h's own documented rule). */
    pico8_ob_table_free(g_df);
    g_df = pico8_ob("\n"
                     "o(\"ll\",45,0,me=\"ll\"),\n"
                     "o(\"e\",160,4.5,\"earth-like\"),\n"
                     "o(\"f\",128,6,\"forgeworld\"),\n"
                     "o(\"m\",134,4,\"mineral\"),\n"
                     "o(\"o\",130,3,\"ocean\"),\n"
                     "o(\"r\",140,2.5,\"remnant\"),\n"
                     "o(\"x\",162,3,\"xeno\"),\n"
                     "o(\"j\",136,2,\"jungle\"),\n"
                     "o(\"i\",138,2,\"iceball\"),\n"
                     "o(\"s\",142,2,\"barren\"),\n"
                     "o(\"g\",96,4,\"gas giant\"),\n",
                     NULL);
    /* foreach(df,ho(df,ob([["px","s","r","d",]]))) (pico8/lua.lua:1389-
     * 1391): renames each record's own positional 1..4 to px/s/r/d and
     * registers it into `df` itself by its own new `.px` name (see
     * pico8_data_init_rename_pass's own comment). `gp` omitted (no r[]
     * translation for df's own fields). */
    df_mr = pico8_ob("\n\"px\",\"s\",\"r\",\"d\",\n", NULL);
    pico8_data_init_rename_pass(g_df, df_mr, NULL);
    pico8_ob_table_free(df_mr);

    /* hc=ob(unstash(8383)) (pico8/lua.lua:1391) -- real content decoded
     * from live cart memory, not a literal in pico8/lua.lua. See
     * pico8_data_init.h's own pico8_hc() comment for its real consumer. */
    decoded = pico8_unstash(mem, 8383);
    pico8_ob_table_free(g_hc);
    g_hc = pico8_ob(decoded, NULL);
    free(decoded);

    /* fd=ob(unstash(8494)) (pico8/lua.lua:1391) -- the still-undecoded
     * research-project data (see pico8_data_init.h's own pico8_fd()
     * comment). Consumed immediately below to build `gb`. */
    decoded = pico8_unstash(mem, 8494);
    pico8_ob_table_free(g_fd);
    g_fd = pico8_ob(decoded, NULL);
    free(decoded);

    /* hp=kv:jr([[...]]) / gb,rr={} for ri in all(fd)do...end
     * (pico8/lua.lua:1399-1441) -- see pico8_research.h's own top comment;
     * pico8_research_init() must run BEFORE pico8_research_build_gb() ever
     * instantiates an `hp`, per this chunk's own explicit instruction to
     * build `hp` before this init function's own body needs it. */
    pico8_research_init();
    pico8_research_build_gb(g_fd);

    /* dr=ob(unstash(9807)) (pico8/lua.lua:1442) -- see pico8_data_init.h's
     * own pico8_dr() comment on why it's decoded but not yet consumed by
     * any function in this chunk's own scope. */
    decoded = pico8_unstash(mem, 9807);
    pico8_ob_table_free(g_dr);
    g_dr = pico8_ob(decoded, NULL);
    free(decoded);

    /* nb=ob(unstash(10457)) (pico8/lua.lua:1442) plus its own
     * foreach(nb,ho(nb,ob([["px","lv","nk","od","oy",]]),{"nk"})) renaming
     * pass (pico8/lua.lua:1442-1444) -- UNLIKE df's own pass, `gp={"nk"}`
     * routes position 3's raw value through `r[...]` (pico8_stash.h's
     * `pico8_r()`), matching this port's own `pico8_gu_content`-based
     * `gu(gp,p)` check inside pico8_ho_apply itself (pico8_collect.c). */
    decoded = pico8_unstash(mem, 10457);
    pico8_ob_table_free(g_nb);
    g_nb = pico8_ob(decoded, NULL);
    free(decoded);

    nb_mr = pico8_ob("\"px\",\"lv\",\"nk\",\"od\",\"oy\",", NULL);
    pico8_list_init(&nb_gp);
    pico8_list_add(&nb_gp, pico8_value_string("nk"));
    pico8_data_init_rename_pass(g_nb, nb_mr, &nb_gp);
    pico8_list_free(&nb_gp);
    pico8_ob_table_free(nb_mr);

    /* ek=ob([[2,4,7,12,]]) (pico8/lua.lua:1445) -- see pico8_data_init.h's
     * own pico8_ek() comment on its real consumer, hy:kt() (Chunk 12). */
    pico8_ob_table_free(g_ek);
    g_ek = pico8_ob("2,4,7,12,", NULL);
}

void pico8_data_shutdown(void) {
    pico8_research_shutdown();
    pico8_ob_table_free(g_df);
    g_df = NULL;
    pico8_ob_table_free(g_hc);
    g_hc = NULL;
    pico8_ob_table_free(g_fd);
    g_fd = NULL;
    pico8_ob_table_free(g_dr);
    g_dr = NULL;
    pico8_ob_table_free(g_nb);
    g_nb = NULL;
    pico8_ob_table_free(g_ek);
    g_ek = NULL;
}

const pico8_ob_table_t *pico8_df(void) {
    return g_df;
}

const pico8_ob_table_t *pico8_hc(void) {
    return g_hc;
}

const pico8_ob_table_t *pico8_fd(void) {
    return g_fd;
}

const pico8_ob_table_t *pico8_dr(void) {
    return g_dr;
}

const pico8_ob_table_t *pico8_nb(void) {
    return g_nb;
}

const pico8_ob_table_t *pico8_ek(void) {
    return g_ek;
}
