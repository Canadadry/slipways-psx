/* The kernel's component links on entities whose constructor `malloc`s
 * without zeroing. The binary runs itself again with every `malloc`
 * filling its block (MALLOC_PERTURB_ on glibc, MallocScribble on macOS),
 * so a link no init set is not NULL. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_ui_button.h"

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static void rerun_with_malloc_filling(char **argv) {
    if (getenv("PICO8_MALLOC_FILLS") != NULL) {
        return;
    }
    setenv("MALLOC_PERTURB_", "90", 1);
    setenv("MallocScribble", "1", 1);
    setenv("PICO8_MALLOC_FILLS", "1", 1);
    execv(argv[0], argv);
    perror("execv");
    exit(1);
}

/* Reads a fresh block on purpose: through `volatile`, which the compiler
 * does not flag as uninitialized. */
static void assert_malloc_fills(void) {
    void *block = malloc(64);
    const volatile unsigned char *bytes = block;
    int i;

    assert(block != NULL);
    for (i = 0; i < 64; i++) {
        assert(bytes[i] != 0 && bytes[i] == bytes[0] && "malloc must fill its block, or this test proves nothing");
    }
    free(block);
}

/* Each link: the one a class carries, or NULL. */
static void assert_links(const pico8_struct_header_t *h, const pico8_fixed_t *fa, const pico8_vec_t *const *ra,
                         const pico8_vec_t *const *p) {
    assert(h->fa == fa);
    assert(h->ra == ra);
    assert(h->p == p);
}

/* Non-carriers, one per constructor that does not go through ke. */
static void test_non_carriers_have_null_links(void) {
    pico8_ie_t *ie = pico8_ie_new(NULL);
    pico8_ie_ke_t view = pico8_ie_ke_view(ie);
    pico8_nf_t *nf = pico8_nf_new();
    pico8_lq_t *lq = pico8_lq_new(NULL);

    assert_links(&nf->header, NULL, (const pico8_vec_t *const *)&nf->ra, NULL);
    assert_links(&lq->header, NULL, &lq->ra, NULL);
    assert_links(&pico8_jb_new(NULL)->header, NULL, NULL, NULL);
    assert_links(&pico8_hp_new(NULL)->header, NULL, NULL, NULL);
    assert_links(&pico8_hy_new()->header, NULL, NULL, NULL);
    assert_links(&pico8_trade_new(NULL)->header, NULL, NULL, NULL);
    assert_links(&view.header, NULL, NULL, NULL);
}

/* Carriers: the archetype's field for a class that never writes it, the
 * instance's otherwise. */
static void test_carriers_link_their_storage(void) {
    pico8_bg_t *bg = pico8_bg_new();
    pico8_js_t *js = pico8_js_new(NULL);
    pico8_ie_t *ie = pico8_ie_new(NULL);

    assert_links(&bg->header, &bg->def->fa, NULL, NULL);
    assert_links(&js->header, &js->fa, &js->ra, NULL);
    assert_links(&ie->header, &ie->def->fa, &ie->ra, NULL);
}

int main(int argc, char **argv) {
    (void)argc;
    rerun_with_malloc_filling(argv);
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);
    pico8_cart_load();
    pico8_jl();

    assert_malloc_fills();
    test_non_carriers_have_null_links();
    test_carriers_link_their_storage();

    printf("test_pico8_struct_header: all tests passed\n");
    return 0;
}
