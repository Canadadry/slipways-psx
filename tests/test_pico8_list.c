#include "../src/pico8_list.h"

#include <assert.h>
#include <stdio.h>

static void test_add_grows_list_and_appends_in_order(void) {
    pico8_list_t list;
    pico8_list_init(&list);

    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(1)));
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(2)));
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(3)));
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(4)));
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(5)));

    assert(list.count == 5);
    assert(list.capacity >= 5);
    assert(list.items[0].as.fixed == pico8_fixed_from_int(1));
    assert(list.items[1].as.fixed == pico8_fixed_from_int(2));
    assert(list.items[2].as.fixed == pico8_fixed_from_int(3));
    assert(list.items[3].as.fixed == pico8_fixed_from_int(4));
    assert(list.items[4].as.fixed == pico8_fixed_from_int(5));

    pico8_list_free(&list);
}

static void test_del_removes_first_matching_element_by_value(void) {
    int a, b, c;
    pico8_list_t list;
    pico8_list_init(&list);

    pico8_list_add(&list, pico8_value_ref(&a));
    pico8_list_add(&list, pico8_value_ref(&b));
    pico8_list_add(&list, pico8_value_ref(&c));

    int removed = pico8_list_del(&list, pico8_value_ref(&b));

    assert(removed == 1);
    assert(list.count == 2);
    assert(list.items[0].as.ref == &a);
    assert(list.items[1].as.ref == &c);

    pico8_list_free(&list);
}

static void test_del_not_found_is_a_no_op(void) {
    int a, b, not_in_list;
    pico8_list_t list;
    pico8_list_init(&list);

    pico8_list_add(&list, pico8_value_ref(&a));
    pico8_list_add(&list, pico8_value_ref(&b));

    int removed = pico8_list_del(&list, pico8_value_ref(&not_in_list));

    assert(removed == 0);
    assert(list.count == 2);
    assert(list.items[0].as.ref == &a);
    assert(list.items[1].as.ref == &b);

    pico8_list_free(&list);
}

static void test_all_iterates_in_order(void) {
    pico8_list_t list;
    pico8_list_init(&list);
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(10)));
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(20)));
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(30)));

    pico8_list_iter_t it = pico8_list_all(&list);
    pico8_value_t v;
    int seen = 0;
    int expected[3] = {10, 20, 30};

    while (pico8_list_iter_next(&it, &v)) {
        assert(v.as.fixed == pico8_fixed_from_int(expected[seen]));
        seen++;
    }
    assert(seen == 3);

    pico8_list_free(&list);
}

static void test_all_tolerates_deleting_current_iteration_element(void) {
    /* PICO-8's documented all()+del() caveat: safe to delete exactly the
     * element the iterator currently sits on -- the rest still get
     * visited, in order, exactly once. */
    int a, b, c, d;
    pico8_list_t list;
    pico8_list_init(&list);
    pico8_list_add(&list, pico8_value_ref(&a));
    pico8_list_add(&list, pico8_value_ref(&b));
    pico8_list_add(&list, pico8_value_ref(&c));
    pico8_list_add(&list, pico8_value_ref(&d));

    void *visited[4];
    int visited_count = 0;

    pico8_list_iter_t it = pico8_list_all(&list);
    pico8_value_t v;
    while (pico8_list_iter_next(&it, &v)) {
        visited[visited_count++] = v.as.ref;
        if (v.as.ref == &a) {
            pico8_list_del(&list, pico8_value_ref(&a));
        }
    }

    assert(visited_count == 4);
    assert(visited[0] == &a);
    assert(visited[1] == &b);
    assert(visited[2] == &c);
    assert(visited[3] == &d);
    assert(list.count == 3);

    pico8_list_free(&list);
}

static void test_all_on_empty_list_yields_nothing(void) {
    pico8_list_t list;
    pico8_list_init(&list);

    pico8_list_iter_t it = pico8_list_all(&list);
    pico8_value_t v;
    assert(pico8_list_iter_next(&it, &v) == 0);

    pico8_list_free(&list);
}

static void collect_into_array(pico8_value_t value, void *ctx) {
    struct {
        pico8_fixed_t *items;
        int *count;
    } *collector = ctx;
    collector->items[*collector->count] = value.as.fixed;
    (*collector->count)++;
}

static void test_foreach_visits_every_element_once_in_order(void) {
    pico8_list_t list;
    pico8_list_init(&list);
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(1)));
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(2)));
    pico8_list_add(&list, pico8_value_fixed(pico8_fixed_from_int(3)));

    pico8_fixed_t collected[3];
    int collected_count = 0;
    struct {
        pico8_fixed_t *items;
        int *count;
    } collector = {collected, &collected_count};

    pico8_list_foreach(&list, collect_into_array, &collector);

    assert(collected_count == 3);
    assert(collected[0] == pico8_fixed_from_int(1));
    assert(collected[1] == pico8_fixed_from_int(2));
    assert(collected[2] == pico8_fixed_from_int(3));

    pico8_list_free(&list);
}

int main(void) {
    test_add_grows_list_and_appends_in_order();
    test_del_removes_first_matching_element_by_value();
    test_del_not_found_is_a_no_op();
    test_all_iterates_in_order();
    test_all_tolerates_deleting_current_iteration_element();
    test_all_on_empty_list_yields_nothing();
    test_foreach_visits_every_element_once_in_order();

    printf("test_pico8_list: all tests passed\n");
    return 0;
}
