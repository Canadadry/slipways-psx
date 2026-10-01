#include "engine/pointer_list.h"

#include <assert.h>
#include <stdio.h>

#include "heap_allocator.h"

static void test_add_grows_past_four_and_keeps_order(void) {
    int cells[5];
    int i;
    pico8_ptrs_t ptrs;
    pico8_ptrs_init(&ptrs, &zone_tas);
    assert(ptrs.count == 0 && ptrs.items == NULL);

    for (i = 0; i < 5; i++) {
        pico8_ptrs_add(&ptrs, &cells[i]);
    }

    assert(ptrs.count == 5);
    assert(ptrs.capacity >= 5);
    for (i = 0; i < 5; i++) {
        assert(ptrs.items[i] == &cells[i]);
    }

    pico8_ptrs_free(&ptrs);
    assert(ptrs.items == NULL && ptrs.count == 0 && ptrs.capacity == 0);
}

static void test_del_removes_only_the_first_identical_pointer(void) {
    int a, b, c;
    pico8_ptrs_t ptrs;
    pico8_ptrs_init(&ptrs, &zone_tas);
    pico8_ptrs_add(&ptrs, &a);
    pico8_ptrs_add(&ptrs, &b);
    pico8_ptrs_add(&ptrs, &c);
    pico8_ptrs_add(&ptrs, &b);

    assert(pico8_ptrs_del(&ptrs, &b) == 1);

    assert(ptrs.count == 3);
    assert(ptrs.items[0] == &a);
    assert(ptrs.items[1] == &c);
    assert(ptrs.items[2] == &b);

    pico8_ptrs_free(&ptrs);
}

static void test_del_absent_is_a_no_op(void) {
    int a, b, absent;
    pico8_ptrs_t ptrs;
    pico8_ptrs_init(&ptrs, &zone_tas);
    pico8_ptrs_add(&ptrs, &a);
    pico8_ptrs_add(&ptrs, &b);

    assert(pico8_ptrs_del(&ptrs, &absent) == 0);

    assert(ptrs.count == 2);
    assert(ptrs.items[0] == &a);
    assert(ptrs.items[1] == &b);

    pico8_ptrs_free(&ptrs);
}

static void test_null_is_stored_and_del_null_removes_the_first(void) {
    int a;
    pico8_ptrs_t ptrs;
    pico8_ptrs_init(&ptrs, &zone_tas);
    pico8_ptrs_add(&ptrs, &a);
    pico8_ptrs_add(&ptrs, NULL);
    pico8_ptrs_add(&ptrs, NULL);

    assert(ptrs.count == 3);
    assert(pico8_ptrs_del(&ptrs, NULL) == 1);

    assert(ptrs.count == 2);
    assert(ptrs.items[0] == &a);
    assert(ptrs.items[1] == NULL);

    pico8_ptrs_free(&ptrs);
}

static void test_all_walks_in_order(void) {
    int a, b, c;
    int *expected[3];
    void *e;
    int seen = 0;
    pico8_ptrs_t ptrs;
    pico8_ptrs_iter_t it;
    expected[0] = &a;
    expected[1] = &b;
    expected[2] = &c;
    pico8_ptrs_init(&ptrs, &zone_tas);
    pico8_ptrs_add(&ptrs, &a);
    pico8_ptrs_add(&ptrs, &b);
    pico8_ptrs_add(&ptrs, &c);

    it = pico8_ptrs_all(&ptrs);
    while (pico8_ptrs_iter_next(&it, &e)) {
        assert(e == expected[seen]);
        seen++;
    }
    assert(seen == 3);

    pico8_ptrs_free(&ptrs);
}

static void test_all_tolerates_deleting_the_current_element(void) {
    int a, b, c, d;
    void *visited[4];
    int visited_count = 0;
    void *e;
    pico8_ptrs_t ptrs;
    pico8_ptrs_iter_t it;
    pico8_ptrs_init(&ptrs, &zone_tas);
    pico8_ptrs_add(&ptrs, &a);
    pico8_ptrs_add(&ptrs, &b);
    pico8_ptrs_add(&ptrs, &c);
    pico8_ptrs_add(&ptrs, &d);

    it = pico8_ptrs_all(&ptrs);
    while (pico8_ptrs_iter_next(&it, &e)) {
        visited[visited_count++] = e;
        if (e == &b) {
            pico8_ptrs_del(&ptrs, &b);
        }
    }

    assert(visited_count == 4);
    assert(visited[0] == &a);
    assert(visited[1] == &b);
    assert(visited[2] == &c);
    assert(visited[3] == &d);
    assert(ptrs.count == 3);

    pico8_ptrs_free(&ptrs);
}

/* Tolerance only for the element just returned: deleting another one makes
 * the walk re-read the current slot, so `b` is visited twice. */
static void test_all_revisits_when_another_element_is_deleted(void) {
    int a, b, c, d;
    void *visited[5];
    int visited_count = 0;
    void *e;
    pico8_ptrs_t ptrs;
    pico8_ptrs_iter_t it;
    pico8_ptrs_init(&ptrs, &zone_tas);
    pico8_ptrs_add(&ptrs, &a);
    pico8_ptrs_add(&ptrs, &b);
    pico8_ptrs_add(&ptrs, &c);
    pico8_ptrs_add(&ptrs, &d);

    it = pico8_ptrs_all(&ptrs);
    while (pico8_ptrs_iter_next(&it, &e)) {
        assert(visited_count < 5);
        visited[visited_count++] = e;
        if (e == &b) {
            pico8_ptrs_del(&ptrs, &c);
        }
    }

    assert(visited_count == 4);
    assert(visited[0] == &a);
    assert(visited[1] == &b);
    assert(visited[2] == &b);
    assert(visited[3] == &d);

    pico8_ptrs_free(&ptrs);
}

static void test_all_on_empty_yields_nothing(void) {
    void *e = &e;
    pico8_ptrs_t ptrs;
    pico8_ptrs_iter_t it;
    pico8_ptrs_init(&ptrs, &zone_tas);

    it = pico8_ptrs_all(&ptrs);
    assert(pico8_ptrs_iter_next(&it, &e) == 0);
    assert(e == &e);

    pico8_ptrs_free(&ptrs);
}

typedef struct {
    void *seen[4];
    int count;
} collector_t;

static void collect(void *e, void *ctx) {
    collector_t *collector = ctx;
    collector->seen[collector->count++] = e;
}

static void test_foreach_visits_each_once_in_order_with_ctx(void) {
    int a, b, c;
    collector_t collector;
    pico8_ptrs_t ptrs;
    collector.count = 0;
    pico8_ptrs_init(&ptrs, &zone_tas);
    pico8_ptrs_add(&ptrs, &a);
    pico8_ptrs_add(&ptrs, &b);
    pico8_ptrs_add(&ptrs, &c);

    pico8_ptrs_foreach(&ptrs, collect, &collector);

    assert(collector.count == 3);
    assert(collector.seen[0] == &a);
    assert(collector.seen[1] == &b);
    assert(collector.seen[2] == &c);

    pico8_ptrs_free(&ptrs);
}

static void test_reserved_list_holds_its_capacity_in_order(void) {
    int cells[6];
    int i;
    pico8_ptrs_t ptrs;
    pico8_ptrs_init_reserve(&ptrs, &zone_tas, 6);
    assert(ptrs.count == 0 && ptrs.capacity == 6);

    for (i = 0; i < 6; i++) {
        pico8_ptrs_add(&ptrs, &cells[i]);
    }

    assert(ptrs.count == 6 && ptrs.capacity == 6 && "a reserved list fills to its capacity without growing");
    for (i = 0; i < 6; i++) {
        assert(ptrs.items[i] == &cells[i]);
    }
    pico8_ptrs_free(&ptrs);
}

int main(void) {
    test_add_grows_past_four_and_keeps_order();
    test_del_removes_only_the_first_identical_pointer();
    test_del_absent_is_a_no_op();
    test_null_is_stored_and_del_null_removes_the_first();
    test_all_walks_in_order();
    test_all_tolerates_deleting_the_current_element();
    test_all_revisits_when_another_element_is_deleted();
    test_all_on_empty_yields_nothing();
    test_foreach_visits_each_once_in_order_with_ctx();
    test_reserved_list_holds_its_capacity_in_order();
    printf("test_pico8_ptrs: all tests passed\n");
    return 0;
}
