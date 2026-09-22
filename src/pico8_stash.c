#include "pico8_stash.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_str.h"

static const char *const PICO8_GT = "abcdefghijklmnopqrstuvwxyz0123456789 ().,=-+_/\"'?%\n";

static pico8_ob_table_t *g_r = NULL;
static pico8_ob_table_t *g_iz = NULL;

char *pico8_unstash(const pico8_mem_t *mem, int pd) {
    size_t cap = 16;
    size_t len = 0;
    char *s = malloc(cap);

    assert(strlen(PICO8_GT) == 51 && "gt must stay 51 characters: a shorter or longer alphabet shifts every decoded byte silently");
    s[0] = '\0';

    for (;;) {
        unsigned char i = pico8_peek(mem, pd);
        char *piece = pico8_sub(PICO8_GT, i, i);
        size_t piece_len = strlen(piece);

        if (len + piece_len + 1 > cap) {
            cap = (len + piece_len + 1) * 2;
            s = realloc(s, cap);
        }
        memcpy(s + len, piece, piece_len + 1);
        len += piece_len;
        free(piece);
        pd += 1;

        if (i == 0) {
            break;
        }
    }

    return s;
}

void pico8_stash_shutdown(void) {
    pico8_ob_table_free(g_r);
    g_r = NULL;
    pico8_ob_table_free(g_iz);
    g_iz = NULL;
}

void pico8_stash_init(pico8_mem_t *mem) {
    char *r_src;
    char *iz_src;

    pico8_stash_shutdown();

    r_src = pico8_unstash(mem, 8192);
    g_r = pico8_ob(r_src, NULL);
    free(r_src);

    iz_src = pico8_unstash(mem, 8259);
    g_iz = pico8_ob(iz_src, NULL);
    free(iz_src);
}

const pico8_ob_table_t *pico8_r(void) {
    return g_r;
}

const pico8_ob_table_t *pico8_iz(void) {
    return g_iz;
}
