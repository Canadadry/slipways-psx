/* The nine planet kinds the cart handles as values -- the seven of `gx`,
 * then `pm` and `ga`, reached by `kv[hp.cw]`. test_kind_fields_and_build()
 * reads them through the shared readers kind_field()/kind_build() only; its
 * frozen value was taken on the code where these were still classes and
 * must not be re-taken. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define KIND_HASH 0xBCF0E60Bu

#define KIND_COUNT 9

#define RAW_PRESENCE_HASH 0xAAA6B4EDu
#define ABSENT_COUNT 6
#define BEHAVIOR_HASH 0x0D6BD173u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const pico8_entity_kind_t *kind_of(int i) {
    switch (i) {
    case 0:
        return pico8_qh_kind();
    case 1:
        return pico8_os_kind();
    case 2:
        return pico8_nr_kind();
    case 3:
        return pico8_fr_kind();
    case 4:
        return pico8_nl_kind();
    case 5:
        return pico8_mx_kind();
    case 6:
        return pico8_hj_kind();
    case 7:
        return pico8_pm_kind();
    default:
        return pico8_ga_kind();
    }
}

static int kind_field(int i, const char *name, pico8_value_t *out) {
    return kind_of(i)->field(name, out);
}

static void *kind_build(int i) {
    return kind_of(i)->new_entity(NULL);
}

static const pico8_struct_class_t *struct_class_of(int i) {
    switch (i) {
    case 0:
        return pico8_qh_struct_class();
    case 1:
        return pico8_os_struct_class();
    case 2:
        return pico8_nr_struct_class();
    case 3:
        return pico8_fr_struct_class();
    case 4:
        return pico8_nl_struct_class();
    case 5:
        return pico8_mx_struct_class();
    case 6:
        return pico8_hj_struct_class();
    case 7:
        return pico8_pm_struct_class();
    default:
        return pico8_ga_struct_class();
    }
}

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
        return fnv(h, v.as.ref == NULL ? 0u : 1u);
    default:
        return h;
    }
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

/* `rr`/`s`/`me`/`od` of each kind as `ie:hb`/`ie:kj` read them, absent
 * included (`qh`/`os` have no `rr`), and the struct each kind builds. */
static void test_kind_fields_and_build(void) {
    static const char *const names[] = {"rr", "s", "me", "od"};
    uint32_t h = 2166136261u;
    int i;
    size_t n;

    for (i = 0; i < KIND_COUNT; i++) {
        void *made;

        for (n = 0; n < sizeof names / sizeof names[0]; n++) {
            pico8_value_t v = pico8_value_fixed(0x5A5A);
            int found = kind_field(i, names[n], &v);

            h = fnv(h, (uint32_t)found);
            if (found) {
                h = fold_value(h, v);
            } else {
                assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x5A5A);
            }
        }
        made = kind_build(i);
        assert(made != NULL && pico8_type_tag_of(made) == PICO8_TYPE_TAG_STRUCT &&
               ((const pico8_struct_header_t *)made)->cls == struct_class_of(i));
        h = fnv(h, (uint32_t)i);
    }
    check("kind", h, KIND_HASH);
}

/* The string kind `i` serves as `name`: the very pointer. */
static const char *kind_string(int i, const char *name) {
    pico8_value_t v = pico8_value_fixed(0x5A5A);

    assert(kind_field(i, name, &v) && v.kind == PICO8_VALUE_STRING);
    return v.as.string;
}

static pico8_fixed_t kind_fixed(int i, const char *name) {
    pico8_value_t v = pico8_value_fixed(0x5A5A);

    assert(kind_field(i, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static int kind_has(int i, const char *name) {
    pico8_value_t v = pico8_value_fixed(0x5A5A);
    int found = kind_field(i, name, &v);

    assert(found || (v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x5A5A));
    return found;
}

/* Each kind serves the generated default of the class that sets each name,
 * and pico8_type_tag_of() reads a kind UNKNOWN. */
static void test_kind_reader_serves_the_generated_defaults(void) {
    int i;

    for (i = 0; i < KIND_COUNT; i++) {
        assert(pico8_type_tag_of(kind_of(i)) == PICO8_TYPE_TAG_UNKNOWN);
    }
    assert(!kind_has(0, "rr"));
    assert(kind_fixed(0, "s") == pico8_qh_start.s);
    assert(kind_string(0, "me") == pico8_qh_start.me);
    assert(kind_fixed(0, "od") == pico8_qh_start.od);

    assert(!kind_has(1, "rr"));
    assert(kind_fixed(1, "s") == pico8_os_start.s);
    assert(kind_string(1, "me") == pico8_qh_start.me);
    assert(kind_fixed(1, "od") == pico8_os_start.od);

    assert(kind_string(2, "rr") == pico8_nr_start.rr);
    assert(kind_fixed(2, "s") == pico8_nr_start.s);
    assert(kind_string(2, "me") == pico8_qh_start.me);
    assert(kind_fixed(2, "od") == pico8_nr_start.od);

    assert(kind_string(3, "rr") == pico8_fr_start.rr);
    assert(kind_fixed(3, "s") == pico8_fr_start.s);
    assert(kind_string(3, "me") == pico8_qh_start.me);
    assert(kind_fixed(3, "od") == pico8_fr_start.od);

    assert(kind_string(4, "rr") == pico8_nl_start.rr);
    assert(kind_fixed(4, "s") == pico8_nl_start.s);
    assert(kind_string(4, "me") == pico8_qh_start.me);
    assert(kind_fixed(4, "od") == pico8_nl_start.od);

    assert(kind_string(5, "rr") == pico8_mx_start.rr);
    assert(kind_fixed(5, "s") == pico8_mx_start.s);
    assert(kind_string(5, "me") == pico8_qh_start.me);
    assert(kind_fixed(5, "od") == pico8_mx_start.od);

    assert(kind_string(6, "rr") == pico8_hj_start.rr);
    assert(kind_fixed(6, "s") == pico8_hj_start.s);
    assert(kind_string(6, "me") == pico8_qh_start.me);
    assert(kind_fixed(6, "od") == pico8_hj_start.od);

    assert(kind_string(7, "rr") == pico8_hj_start.rr);
    assert(kind_fixed(7, "s") == pico8_pm_start.s);
    assert(kind_string(7, "me") == pico8_pm_start.me);
    assert(kind_fixed(7, "od") == pico8_hj_start.od);

    assert(!kind_has(8, "rr"));
    assert(!kind_has(8, "s"));
    assert(kind_string(8, "me") == pico8_jo_start.me);
    assert(!kind_has(8, "od"));
}

/* `kv[hp.cw]` only ever sees "pm" or "ga": every research of `gb` carries
 * one of them as `cw`, or none. */
static void test_every_cw_is_pm_or_ga(void) {
    int with_cw = 0;
    int k;

    assert(pico8_gb_count() > 0);
    for (k = 1; k <= pico8_gb_count(); k++) {
        const pico8_hp_t *hp;
        const char *cw;

        hp = pico8_gb_at(k);
        cw = hp->cw;
        if (cw == NULL) {
            continue;
        }
        assert(strcmp(cw, "pm") == 0 || strcmp(cw, "ga") == 0);
        with_cw++;
    }
    printf("[cw] %d of %d researches\n", with_cw, pico8_gb_count());
    assert(with_cw > 0);
}

#define ICON_COUNT 8
#define WITNESS 0x1234

static uint32_t ie_screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        h = fnv(h, (uint32_t)pico8_peek(&mem, PICO8_ADDR_SCREEN + i));
    }
    return h;
}

/* 0 absent, 1 false, 2 true, 3 other. */
static uint32_t ie_bool_state(const void *e, const char *name) {
    pico8_value_t v;
    if (!pico8_entity_get_field(e, name, &v)) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_BOOL) {
        return v.as.boolean ? 2u : 1u;
    }
    return 3u;
}

/* Icons 0..6 hold the `gx` kinds, icon 7 holds none; `pm`/`ga` are in no
 * `ie`. The view is read by name only, through its raw reader and through
 * pico8_entity_get_field(). */
static void test_ie_ke_view_by_name(void) {
    static const char *const names[] = {"rr", "s", "me", "od", "", "x", "et", "ke", "header", "has_s", "has_od"};
    const size_t name_count = sizeof names / sizeof names[0];
    pico8_ie_t *icons[ICON_COUNT];
    const pico8_struct_class_t *cls = pico8_ie_ke_view_struct_class();
    uint32_t raw = 2166136261u;
    uint32_t behavior = 2166136261u;
    int absent = 0;
    int i;

    for (i = 0; i < ICON_COUNT; i++) {
        if (i < 7) {
            pico8_ie_keys_t keys = {0};

            keys.ke = kind_of(i);
            icons[i] = pico8_ie_new(&keys);
        } else {
            icons[i] = pico8_ie_new(NULL);
        }
        assert(icons[i] != NULL);
    }

    for (i = 0; i < ICON_COUNT; i++) {
        pico8_ie_ke_t view = pico8_ie_ke_view(icons[i]);
        size_t k;
        for (k = 0; k < name_count; k++) {
            pico8_value_t out = pico8_value_fixed(WITNESS);
            pico8_value_t by_get = pico8_value_fixed(WITNESS);
            int got = cls->field(&view, names[k], &out);
            int got_get = pico8_entity_get_field(&view, names[k], &by_get);
            assert(got == got_get);
            if (got) {
                assert(k < 4);
                assert(!(out.kind == PICO8_VALUE_REF && out.as.ref == NULL));
                assert((k == 1 || k == 3) ? out.kind == PICO8_VALUE_FIXED
                                          : (out.kind == PICO8_VALUE_STRING && out.as.string != NULL));
                assert(by_get.kind == out.kind);
                assert(out.kind == PICO8_VALUE_FIXED ? by_get.as.fixed == out.as.fixed
                                                     : by_get.as.string == out.as.string);
            } else {
                assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == WITNESS);
                assert(by_get.kind == PICO8_VALUE_FIXED && by_get.as.fixed == WITNESS);
                if (k < 4) {
                    absent++;
                }
            }
            raw = fnv(raw, (uint32_t)i);
            raw = fold_string(raw, names[k]);
            raw = fnv(raw, (uint32_t)got);
            raw = fold_value(raw, out);
        }
    }
    printf("[ie_ke absent] %d\n", absent);
    assert(absent == ABSENT_COUNT);
    check("ie_ke raw presence", raw, RAW_PRESENCE_HASH);

    for (i = 0; i < 7; i++) {
        static const char *const borrowed[] = {"rr", "me"};
        pico8_ie_ke_t first = pico8_ie_ke_view(icons[i]);
        pico8_ie_ke_t second = pico8_ie_ke_view(icons[i]);
        int b;
        for (b = 0; b < 2; b++) {
            pico8_value_t from_kind = pico8_value_ref(NULL);
            pico8_value_t a = pico8_value_ref(NULL);
            pico8_value_t c = pico8_value_ref(NULL);
            int in_kind = kind_of(i)->field(borrowed[b], &from_kind);
            assert(pico8_entity_get_field(&first, borrowed[b], &a) == in_kind);
            assert(pico8_entity_get_field(&second, borrowed[b], &c) == in_kind);
            if (in_kind) {
                assert(from_kind.kind == PICO8_VALUE_STRING);
                assert(a.as.string == from_kind.as.string && c.as.string == from_kind.as.string);
            }
        }
    }

    {
        pico8_value_t mouse;
        assert(pico8_entity_get_field(pico8_je(), "ra", &mouse) && mouse.kind == PICO8_VALUE_REF &&
               mouse.as.ref != NULL);
        *(pico8_vec_t *)mouse.as.ref = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    }
    for (i = 0; i < ICON_COUNT; i++) {
        pico8_vec_t origin = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
        uint32_t kh;
        pico8_ie_hb(icons[i]);
        kh = ie_bool_state(icons[i], "kh");
        behavior = fnv(behavior, kh);
        if (kh == 2) {
            uint32_t plain;
            uint32_t hovered;
            pico8_camera(&gfx, 0, 0);
            pico8_cls(&mem);
            pico8_ie_kj(&gfx, &mem, icons[i], origin);
            plain = ie_screen_hash();
            pico8_entity_set_field(icons[i], "m_", pico8_value_bool(1));
            pico8_cls(&mem);
            pico8_ie_kj(&gfx, &mem, icons[i], origin);
            hovered = ie_screen_hash();
            pico8_entity_set_field(icons[i], "m_", pico8_value_bool(0));
            printf("[ie_ke kj] icon %d: 0x%08Xu / 0x%08Xu\n", i, (unsigned)plain, (unsigned)hovered);
            behavior = fnv(behavior, plain);
            behavior = fnv(behavior, hovered);
        }
    }
    check("ie_ke behavior", behavior, BEHAVIOR_HASH);
}

#define IE_MEMBERS_HASH 0xBFE9C2FAu

static uint32_t g_ie_members = 2166136261u;

/* Reads through the raw reader and pico8_entity_get_field(), which must
 * agree; a miss leaves the witness untouched. */
static int ie_read(const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(WITNESS);
    pico8_value_t by_get = pico8_value_fixed(WITNESS);
    int got = pico8_ie_struct_class()->field(e, name, &raw);
    int got_get = pico8_entity_get_field(e, name, &by_get);

    assert(got == got_get);
    assert(raw.kind == by_get.kind);
    if (!got) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == WITNESS);
        assert(by_get.kind == PICO8_VALUE_FIXED && by_get.as.fixed == WITNESS);
    } else {
        assert(!(raw.kind == PICO8_VALUE_REF && raw.as.ref == NULL));
        assert(raw.kind != PICO8_VALUE_REF || raw.as.ref == by_get.as.ref);
    }
    *out = raw;
    return got;
}

static const void *ie_ref(const void *e, const char *name) {
    pico8_value_t v;
    if (!ie_read(e, name, &v)) {
        return NULL;
    }
    assert(v.kind == PICO8_VALUE_REF);
    return v.as.ref;
}

static void ie_fold(uint32_t word) {
    g_ie_members = fnv(g_ie_members, word);
}

static void ie_fold_vec(const void *p) {
    const pico8_vec_t *v = p;
    ie_fold(p != NULL);
    if (v != NULL) {
        ie_fold((uint32_t)v->x);
        ie_fold((uint32_t)v->y);
    }
}

static void ie_fold_kq(const void *e) {
    static const int probe[][2] = {{0, 0},  {-1, 0}, {0, -1}, {8, 7},  {9, 8},  {10, 8}, {9, 9},
                                   {-1, -1}, {4, 4},  {8, 0},  {9, 0},  {0, 7},  {0, 8},  {0, 9}};
    const pico8_qz_box_t *kq = ie_ref(e, "kq");
    size_t i;

    assert(kq != NULL);
    for (i = 0; i < sizeof probe / sizeof probe[0]; i++) {
        ie_fold((uint32_t)pico8_qz_call(
            kq, pico8_vec_v(pico8_fixed_from_int(probe[i][0]), pico8_fixed_from_int(probe[i][1]))));
    }
}

static void ie_fold_names(const void *e) {
    static const char *const names[] = {"kq", "fa", "ra", "eh", "ke", "jb", "kh", "m_", "me", "t", "gy",
                                        "id", "",   "KQ", "x",  "has_kh", "has_m_", "ke_base", "header",
                                        "members", "m"};
    size_t k;

    for (k = 0; k < sizeof names / sizeof names[0]; k++) {
        pico8_value_t v;
        int got = ie_read(e, names[k], &v);
        g_ie_members = fold_string(g_ie_members, names[k]);
        ie_fold((uint32_t)got);
        g_ie_members = fold_value(g_ie_members, v);
        if (k >= 12) {
            assert(!got);
        }
    }
    ie_fold_kq(e);
    ie_fold_vec(ie_ref(e, "ra"));
    ie_fold_vec(ie_ref(e, "eh"));
    ie_fold_vec(ie_ref(e, "gy"));
}

static pico8_ie_t *ie_with(const pico8_entity_kind_t *ke, pico8_jb_t *jb) {
    pico8_ie_keys_t keys = {0};
    pico8_ie_t *e;

    keys.ke = ke;
    keys.jb = jb;
    e = pico8_ie_new(&keys);
    assert(e != NULL);
    return e;
}

static void must_abort(void *e, const char *name) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        pico8_entity_set_field(e, name, pico8_value_bool(1));
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static uint32_t ie_kj_hash(pico8_ie_t *e) {
    pico8_camera(&gfx, 0, 0);
    pico8_cls(&mem);
    pico8_ie_kj(&gfx, &mem, e, pico8_vec_v(pico8_fixed_from_int(20), pico8_fixed_from_int(30)));
    return ie_screen_hash();
}

static pico8_jb_t *ie_jb(void) {
    return pico8_jb_new(&pico8_dr_table[2]);
}

/* `kq`, `ra`, `eh` are the module's, shared by every icon; `kh` and `m_`
 * read absent until written, `false` read present. */
static void test_ie_members_by_name(void) {
    static const char *const unwritten[] = {"kq", "fa", "ra", "eh", "ke", "jb", "kh", "gy",
                                            "id", "me", "t",  "",   "m_ ", "has_m_"};
    pico8_jb_t *J = ie_jb();
    const pico8_entity_kind_t *qh = pico8_qh_kind();
    pico8_ie_t *a = ie_with(qh, J);
    pico8_ie_t *b = ie_with(NULL, NULL);
    pico8_ie_t *n = ie_with(NULL, NULL);
    pico8_ie_t *all[3];
    pico8_value_t v;
    pico8_value_t zero_false;
    const void *spec_ra = ie_ref(a, "ra");
    const void *spec_kq = ie_ref(a, "kq");
    const void *spec_eh = ie_ref(a, "eh");
    const void *gy;
    int i;
    size_t k;

    all[0] = a;
    all[1] = b;
    all[2] = n;
    assert(spec_ra != NULL && spec_kq != NULL && spec_eh != NULL && spec_ra != spec_eh);
    assert(spec_eh == pico8_ie_eh().as.ref);
    for (i = 0; i < 3; i++) {
        assert(ie_ref(all[i], "kq") == spec_kq && ie_ref(all[i], "ra") == spec_ra && ie_ref(all[i], "eh") == spec_eh);
        assert(ie_read(all[i], "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(12));
        assert(ie_read(all[i], "me", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "ph") == 0);
        assert(ie_read(all[i], "t", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0);
        assert(ie_bool_state(all[i], "kh") == 0 && ie_bool_state(all[i], "m_") == 0);
        ie_fold_names(all[i]);
    }
    assert(ie_ref(a, "ke") == (const void *)qh && ie_ref(a, "jb") == (const void *)J);
    assert(ie_ref(b, "ke") == NULL && ie_ref(b, "jb") == NULL);
    assert(ie_ref(n, "ke") == NULL && ie_ref(n, "jb") == NULL);

    memset(&zero_false, 0, sizeof zero_false);
    zero_false.kind = PICO8_VALUE_BOOL;
    {
        struct {
            int raw;
            pico8_value_t value;
            uint32_t want;
        } steps[7];
        steps[0].raw = 0, steps[0].value = pico8_value_bool(1), steps[0].want = 2;
        steps[1].raw = 1, steps[1].value = pico8_value_bool(0), steps[1].want = 1;
        steps[2].raw = 1, steps[2].value = zero_false, steps[2].want = 1;
        steps[3].raw = 1, steps[3].value = pico8_value_ref(NULL), steps[3].want = 0;
        steps[4].raw = 0, steps[4].value = pico8_value_ref(NULL), steps[4].want = 0;
        steps[5].raw = 1, steps[5].value = pico8_value_bool(1), steps[5].want = 2;
        steps[6].raw = 0, steps[6].value = pico8_value_ref(NULL), steps[6].want = 0;
        for (i = 0; i < 7; i++) {
            if (steps[i].raw) {
                assert(pico8_ie_struct_class()->set_field(a, "m_", steps[i].value) == 1);
            } else {
                pico8_entity_set_field(a, "m_", steps[i].value);
            }
            assert(ie_bool_state(a, "m_") == steps[i].want);
            assert(ie_bool_state(a, "kh") == 0 && ie_bool_state(b, "m_") == 0);
            assert(ie_ref(a, "kq") == spec_kq && ie_ref(a, "ra") == spec_ra && ie_ref(a, "eh") == spec_eh);
            assert(ie_ref(a, "ke") == (const void *)qh && ie_ref(a, "jb") == (const void *)J);
            ie_fold_names(a);
        }
    }

    pico8_ie_set_gy(pico8_value_ref(NULL));
    pico8_ie_hb(b);
    assert(ie_bool_state(b, "kh") == 2 && ie_ref(b, "ra") == NULL);
    gy = ie_ref(b, "gy");
    assert(gy != NULL);
    ie_fold_names(b);
    pico8_ie_hb(b);
    assert(ie_bool_state(b, "kh") == 2 && ie_ref(b, "ra") == NULL && ie_ref(b, "gy") == gy);
    {
        pico8_ie_t *c = ie_with(NULL, NULL);
        pico8_ie_t *d = ie_with(pico8_nr_kind(), NULL);
        pico8_ie_t *e;
        pico8_ie_t *f = ie_with(NULL, J);
        uint32_t d_kh;

        pico8_ie_hb(c);
        assert(ie_bool_state(c, "kh") == 2 && ie_ref(c, "ra") == gy && ie_ref(c, "gy") != gy);
        ie_fold_names(c);
        gy = ie_ref(c, "gy");
        pico8_ie_hb(a);
        assert(ie_bool_state(a, "kh") == 2 && ie_ref(a, "ra") == gy && ie_ref(a, "gy") != gy);
        ie_fold_names(a);
        gy = ie_ref(a, "gy");
        pico8_ie_hb(d);
        d_kh = ie_bool_state(d, "kh");
        ie_fold(d_kh);
        if (d_kh == 0) {
            assert(ie_ref(d, "ra") == spec_ra && ie_ref(d, "gy") == gy);
        }
        ie_fold_names(d);
        pico8_ie_set_gy(pico8_ie_eh());
        e = ie_with(NULL, NULL);
        pico8_ie_hb(e);
        assert(ie_ref(e, "ra") == spec_eh);
        ie_fold_names(e);

        {
            pico8_ie_t *fresh = ie_with(NULL, NULL);
            uint32_t empty = ie_kj_hash(fresh);
            uint32_t h;
            pico8_cls(&mem);
            assert(empty == ie_screen_hash());
            ie_fold(empty);
            ie_fold(ie_kj_hash(c));
            pico8_entity_set_field(c, "m_", pico8_value_bool(1));
            ie_fold(ie_kj_hash(c));
            ie_fold(ie_kj_hash(a));
            pico8_entity_set_field(a, "m_", pico8_value_bool(1));
            h = ie_kj_hash(a);
            printf("[ie members kj] a hovered 0x%08Xu\n", (unsigned)h);
            ie_fold(h);
            pico8_entity_set_field(a, "m_", pico8_value_bool(0));
            ie_fold(ie_kj_hash(a));
            pico8_ie_hb(f);
            pico8_entity_set_field(f, "m_", pico8_value_bool(1));
            h = ie_kj_hash(f);
            printf("[ie members kj] f hovered 0x%08Xu\n", (unsigned)h);
            ie_fold(h);
        }
    }

    {
        pico8_value_t id_before;
        pico8_value_t id_after;
        pico8_value_t made;
        pico8_value_t made_id;
        assert(ie_read(a, "id", &id_before) && id_before.kind == PICO8_VALUE_FIXED);
        made = pico8_ie_ldown(a, pico8_value_ref(NULL));
        assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
        assert(pico8_entity_get_field(made.as.ref, "id", &made_id) && made_id.kind == PICO8_VALUE_STRING &&
               made_id.as.string[0] == 's');
        g_ie_members = fold_string(g_ie_members, made_id.as.string);
        assert(ie_read(b, "id", &id_after) && id_after.as.fixed == id_before.as.fixed + pico8_fixed_from_int(1));
        made = pico8_ie_ldown(b, pico8_value_ref(NULL));
        assert(made.kind == PICO8_VALUE_REF && made.as.ref == NULL);
        assert(ie_read(a, "id", &id_after) && id_after.as.fixed == id_before.as.fixed + pico8_fixed_from_int(2));
        ie_fold((uint32_t)id_after.as.fixed);
    }

    for (k = 0; k < sizeof unwritten / sizeof unwritten[0]; k++) {
        assert(pico8_ie_struct_class()->set_field(a, unwritten[k], pico8_value_bool(1)) == 0);
        ie_fold_names(a);
    }
    assert(ie_ref(a, "ke") == (const void *)qh && ie_ref(a, "jb") == (const void *)J);
    must_abort(a, "ke");
    must_abort(a, "jb");
    must_abort(a, "kq");
    must_abort(a, "fa");

    check("ie members", g_ie_members, IE_MEMBERS_HASH);
}

static void must_abort_value(void *e, void (*write)(void *, pico8_value_t), pico8_value_t v) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        write(e, v);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static void write_m_by_name(void *e, pico8_value_t v) {
    pico8_entity_set_field(e, "m_", v);
}

static void hb_from_gy(void *e, pico8_value_t v) {
    (void)e;
    pico8_ie_set_gy(v);
    pico8_ie_hb(ie_with(NULL, NULL));
}

/* `m_` takes nil or a boolean, `ra` nil or a vector off `ie.gy`; a value whose whole union is
 * zero is not nil unless it is a REF. */
static void test_ie_writer_stops_off_row(void) {
    pico8_ie_t *a = ie_with(pico8_qh_kind(), NULL);
    pico8_ie_t *other = ie_with(NULL, NULL);
    pico8_vec_t *vec = malloc(sizeof *vec);
    pico8_value_t zero_fixed;
    pico8_value_t zero_false;
    pico8_value_t zero_string;
    pico8_value_t zero_function;
    pico8_value_t hb;
    pico8_value_t list = pico8_value_ref((void *)pico8_hv());
    pico8_value_t kind = pico8_value_ref((void *)pico8_qh_kind());
    pico8_value_t vref;

    *vec = pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(1));
    vref = pico8_value_ref(vec);
    memset(&zero_fixed, 0, sizeof zero_fixed);
    zero_fixed.kind = PICO8_VALUE_FIXED;
    memset(&zero_false, 0, sizeof zero_false);
    zero_false.kind = PICO8_VALUE_BOOL;
    memset(&zero_string, 0, sizeof zero_string);
    zero_string.kind = PICO8_VALUE_STRING;
    memset(&zero_function, 0, sizeof zero_function);
    zero_function.kind = PICO8_VALUE_FUNCTION;
    assert(pico8_entity_get_field(a, "hb", &hb) && hb.kind == PICO8_VALUE_FUNCTION);

    must_abort_value(a, write_m_by_name, pico8_value_fixed(pico8_fixed_from_int(1)));
    must_abort_value(a, write_m_by_name, zero_fixed);
    must_abort_value(a, write_m_by_name, zero_string);
    must_abort_value(a, write_m_by_name, zero_function);
    must_abort_value(a, write_m_by_name, pico8_value_string("x"));
    must_abort_value(a, write_m_by_name, hb);
    must_abort_value(a, write_m_by_name, vref);
    must_abort_value(a, write_m_by_name, list);
    must_abort_value(a, write_m_by_name, pico8_value_ref(other));
    must_abort_value(a, write_m_by_name, kind);


    must_abort_value(NULL, hb_from_gy, pico8_value_fixed(pico8_fixed_from_int(1)));
    must_abort_value(NULL, hb_from_gy, zero_fixed);
    must_abort_value(NULL, hb_from_gy, zero_false);
    must_abort_value(NULL, hb_from_gy, pico8_value_ref(other));
}

int main(void) {
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    test_every_cw_is_pm_or_ga();
    pico8_jl();
    test_kind_fields_and_build();
    test_kind_reader_serves_the_generated_defaults();
    test_ie_ke_view_by_name();
    test_ie_members_by_name();
    test_ie_writer_stops_off_row();

    printf("test_pico8_entity_kind: all tests passed\n");
    return 0;
}
