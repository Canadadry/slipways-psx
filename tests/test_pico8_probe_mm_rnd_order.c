/* `g_:mm` (lua.lua:727) draws rnd(self.pn) before rnd(0.5): Lua evaluates
 * `flr(rnd(self.pn))+rnd(0.5)` left to right. The particle a probe emits
 * through `ez` must land where that order puts it, whatever the compiler.
 *
 * The generator has no public reseed, so the same state is reached twice by
 * fork(): the child emits the particle, the parent redraws the cart's
 * sequence from the identical state with the port's own rnd. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

static pico8_instance_t *new_probe(pico8_fixed_t pn, pico8_fixed_t t, pico8_fixed_t du, pico8_fixed_t py) {
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    pico8_instance_t *probe;

    pico8_ob_table_set_name(overrides, "pn", pico8_value_fixed(pn));
    pico8_ob_table_set_name(overrides, "t", pico8_value_fixed(t));
    pico8_ob_table_set_name(overrides, "du", pico8_value_fixed(du));
    pico8_ob_table_set_name(overrides, "py", pico8_value_fixed(py));
    probe = pico8_g__new(overrides);
    pico8_ob_table_free(overrides);
    return probe;
}

static pico8_vec_t emitted_particle_p(pico8_instance_t *probe) {
    pico8_value_t qy_val;
    pico8_value_t particle;
    pico8_value_t p_val;
    pico8_list_iter_t it;

    pico8_ez(probe, pico8_fixed_from_int(1));

    assert(pico8_instance_get_field(probe, "qy", &qy_val) && qy_val.kind == PICO8_VALUE_REF && qy_val.as.ref != NULL);
    it = pico8_list_all((const pico8_list_t *)qy_val.as.ref);
    assert(pico8_list_iter_next(&it, &particle) && particle.kind == PICO8_VALUE_REF && particle.as.ref != NULL);
    assert(pico8_ob_table_get_name((const pico8_ob_table_t *)particle.as.ref, "p", &p_val) &&
           p_val.kind == PICO8_VALUE_REF && p_val.as.ref != NULL);
    return *(const pico8_vec_t *)p_val.as.ref;
}

static void test_probe_particle_draws_rnd_pn_before_rnd_half(void) {
    pico8_fixed_t pn = pico8_fixed_from_int(7);
    pico8_fixed_t t = pico8_fixed_from_int(3);
    pico8_fixed_t du = pico8_fixed_from_int(10);
    pico8_fixed_t py = pico8_fixed_from_int(1);
    pico8_instance_t *probe;
    int fds[2];
    pid_t child;
    int status;
    pico8_vec_t got;
    pico8_vec_t want;
    pico8_fixed_t first;
    pico8_fixed_t second;
    pico8_fixed_t a;

    pico8_entity_init();
    pico8_ke_init();
    pico8_probe_init();
    probe = new_probe(pn, t, du, py);
    assert(probe != NULL);

    assert(pipe(fds) == 0);
    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        pico8_vec_t p = emitted_particle_p(probe);
        close(fds[0]);
        _exit(write(fds[1], &p, sizeof p) == (ssize_t)sizeof p ? 0 : 1);
    }
    close(fds[1]);
    assert(read(fds[0], &got, sizeof got) == (ssize_t)sizeof got);
    close(fds[0]);
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);

    /* ez's loop test `rnd()<n`, then lua.lua:727-728 in Lua's order. */
    (void)pico8_fixed_rnd_default();
    first = pico8_fixed_rnd(pn);
    second = pico8_fixed_rnd(PICO8_FIXED_LIT(0x00008000, 0.5));
    a = pico8_fixed_add(pico8_fixed_div(pico8_fixed_add(pico8_fixed_flr(first), second), pn),
                        pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000041, 0.001)));
    want = pico8_vec_qm(pico8_fixed_add(du, pico8_fixed_rnd_default()), a);

    if (got.x != want.x || got.y != want.y) {
        fprintf(stderr, "g_:mm particle p = (%08x,%08x), cart order gives (%08x,%08x)\n", (unsigned)got.x,
                (unsigned)got.y, (unsigned)want.x, (unsigned)want.y);
    }
    assert(got.x == want.x && got.y == want.y);
}

int main(void) {
    test_probe_particle_draws_rnd_pn_before_rnd_half();
    printf("test_pico8_probe_mm_rnd_order: ok\n");
    return 0;
}
