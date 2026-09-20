#include "pico8_stash.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_str.h"

/* gt="abcdefghijklmnopqrstuvwxyz0123456789 ().,=-+_/\"'?%\n" -- see
 * pico8_stash.h's own top comment for the confirmed 51-character count.
 * Private to this module: nothing else in pico8/lua.lua reads the global
 * `gt` outside unstash() itself. */
static const char *const PICO8_GT = "abcdefghijklmnopqrstuvwxyz0123456789 ().,=-+_/\"'?%\n";

static pico8_ob_table_t *g_r = NULL;
static pico8_ob_table_t *g_iz = NULL;

/* function unstash(pd)local s=""repeat
 * local i=peek(pd)s=s..sub(gt,i,i)
 * pd+=1
 * until i==0
 * return s
 * end
 * See pico8_stash.h's own top comment for the full trace. Ported as a
 * growable buffer (doubling capacity, matching this codebase's own
 * established growable-array convention -- see pico8_ob.c/pico8_class.c/
 * pico8_entity.c's identical doubling pattern, grepped for per this
 * phase's own duplication check rather than reinvented blindly) rather
 * than two passes (measure then build), mirroring the original's own
 * single incremental pass (`s=s..sub(...)`). */
char *pico8_unstash(const pico8_mem_t *mem, int pd) {
    size_t cap = 16;
    size_t len = 0;
    char *s = malloc(cap);
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

/* See pico8_stash.h's own comment on this declaration. */
void pico8_stash_shutdown(void) {
    pico8_ob_table_free(g_r);
    g_r = NULL;
    pico8_ob_table_free(g_iz);
    g_iz = NULL;
}

/* r,iz=ob(unstash(8192)),ob(unstash(8259)) -- see pico8_stash.h's own
 * comment on this declaration. */
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
