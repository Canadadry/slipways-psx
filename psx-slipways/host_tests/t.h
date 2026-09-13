#ifndef T_H
#define T_H

#include <stdio.h>

static int t_pass_count = 0;
static int t_fail_count = 0;
static int t_current_test_failed = 0;

#define T_CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("     %s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
            t_current_test_failed = 1; \
        } \
    } while (0)

typedef void (*t_test_fn)(void);

static void t_run(const char *name, t_test_fn fn) {
    t_current_test_failed = 0;
    fn();
    if (t_current_test_failed) {
        printf("FAIL %s\n", name);
        t_fail_count++;
    } else {
        printf("PASS %s\n", name);
        t_pass_count++;
    }
}

static int t_report(void) {
    printf("%d passed, %d failed\n", t_pass_count, t_fail_count);
    return t_fail_count > 0 ? 1 : 0;
}

#endif
