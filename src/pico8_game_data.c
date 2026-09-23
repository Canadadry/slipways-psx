#include "pico8_game_data.h"

#include <stddef.h>

static pico8_ob_table_t *g_es = NULL;
static pico8_ob_table_t *g_ct = NULL;
static pico8_ob_table_t *g_b_ = NULL;
static pico8_ob_table_t *g_co = NULL;
static pico8_ob_table_t *g_fb = NULL;

void pico8_game_data_shutdown(void) {
    pico8_ob_table_free(g_es);
    g_es = NULL;
    pico8_ob_table_free(g_ct);
    g_ct = NULL;
    pico8_ob_table_free(g_b_);
    g_b_ = NULL;
    pico8_ob_table_free(g_co);
    g_co = NULL;
    pico8_ob_table_free(g_fb);
    g_fb = NULL;
}

void pico8_game_data_init(void) {
    pico8_game_data_shutdown();

    g_es = pico8_ob("1,0,1,2,", NULL);
    g_ct = pico8_ob("-0.333,0,0.25,0.5,", NULL);
    g_b_ = pico8_ob("0,100,200,400,", NULL);
    g_co = pico8_ob("-5,0,1,1,", NULL);
    g_fb = pico8_ob("2,2,5,5,7,7,10,10,14,14,14,19,19,19,", NULL);
}

const pico8_ob_table_t *pico8_es(void) {
    return g_es;
}

const pico8_ob_table_t *pico8_ct(void) {
    return g_ct;
}

const pico8_ob_table_t *pico8_b_(void) {
    return g_b_;
}

const pico8_ob_table_t *pico8_co(void) {
    return g_co;
}

const pico8_ob_table_t *pico8_fb(void) {
    return g_fb;
}

pico8_fixed_t pico8_bn(void) {
    return pico8_fixed_from_int(6);
}

/* TROU-1 */
pico8_fixed_t pico8_bq(void) {
    return PICO8_FIXED_LIT(0x00001902, 0.0977);
}
