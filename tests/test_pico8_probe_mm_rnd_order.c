#define _POSIX_C_SOURCE 200809L

#include "../src/pico8_probe.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/pico8_class.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* The PRNG has no seed setter: fork() gives the child (which calls `mm`) and
 * the parent (which draws the cart's expected sequence) the same PRNG state. */

static pico8_fixed_t field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    int ok = pico8_instance_get_field(inst, name, &v);
    assert(ok && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static void run_mm_in_child(pico8_instance_t *probe, int fd) {
    pico8_value_t mm;
    pico8_value_t out;
    pico8_value_t pv;
    pico8_value_t vv;
    pico8_vec_t got[2];
    int ok = pico8_instance_get_field(probe, "mm", &mm);
    assert(ok && mm.kind == PICO8_VALUE_FUNCTION);
    out = mm.as.function(probe, 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    ok = pico8_ob_table_get_name((pico8_ob_table_t *)out.as.ref, "p", &pv);
    assert(ok);
    ok = pico8_ob_table_get_name((pico8_ob_table_t *)out.as.ref, "v", &vv);
    assert(ok);
    got[0] = *(pico8_vec_t *)pv.as.ref;
    got[1] = *(pico8_vec_t *)vv.as.ref;
    if (write(fd, got, sizeof got) != (ssize_t)sizeof got) {
        _exit(2);
    }
    _exit(0);
}

/* pico8/lua.lua:727: `local a=(flr(rnd(self.pn))+rnd(0.5))/self.pn+self.t*0.001`,
 * Lua evaluating rnd(self.pn) before rnd(0.5). */
static void test_mm_draws_rnd_pn_before_rnd_half(void) {
    pico8_ob_table_t *overrides;
    pico8_instance_t *probe;
    pico8_fixed_t pn, t, du, py;
    pico8_fixed_t first, second, a;
    pico8_vec_t expected_p, expected_v;
    pico8_vec_t got[2];
    int fds[2];
    int status;
    pid_t pid;

    pico8_probe_init();
    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "t", pico8_value_fixed(pico8_fixed_from_int(37)));
    probe = pico8_g__new(overrides);
    pico8_ob_table_free(overrides);

    pn = field_fixed(probe, "pn");
    t = field_fixed(probe, "t");
    du = field_fixed(probe, "du");
    py = field_fixed(probe, "py");

    assert(pipe(fds) == 0);
    pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        close(fds[0]);
        run_mm_in_child(probe, fds[1]);
    }
    close(fds[1]);
    assert(read(fds[0], got, sizeof got) == (ssize_t)sizeof got);
    close(fds[0]);
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);

    first = pico8_fixed_flr(pico8_fixed_rnd(pn));
    second = pico8_fixed_rnd(PICO8_FIXED_LIT(0x00008000, 0.5));
    a = pico8_fixed_add(pico8_fixed_div(pico8_fixed_add(first, second), pn),
                        pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000041, 0.001)));
    expected_p = pico8_vec_qm(pico8_fixed_add(du, pico8_fixed_rnd_default()), a);
    expected_v = pico8_vec_pa(pico8_vec_mul(expected_p, py));

    if (got[0].x != expected_p.x || got[0].y != expected_p.y) {
        fprintf(stderr, "mm p = (%08x,%08x), cart order gives (%08x,%08x)\n", (unsigned)got[0].x,
                (unsigned)got[0].y, (unsigned)expected_p.x, (unsigned)expected_p.y);
    }
    assert(got[0].x == expected_p.x && got[0].y == expected_p.y);
    assert(got[1].x == expected_v.x && got[1].y == expected_v.y);

    pico8_probe_shutdown();
}

int main(void) {
    test_mm_draws_rnd_pn_before_rnd_half();

    printf("test_pico8_probe_mm_rnd_order: all tests passed\n");
    return 0;
}
