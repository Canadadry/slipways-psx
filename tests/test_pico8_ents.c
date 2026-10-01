#include "../src/pico8_ents.h"

#include <assert.h>
#include <stdio.h>

#include "../src/pico8_zone_transitoire.h"

static void test_add_grows_past_four_keeps_order_and_free_empties(void) {
    pico8_struct_header_t cells[5];
    int i;
    pico8_ents_t ents;
    pico8_ents_init(&ents, &pico8_zone_transitoire);
    assert(ents.count == 0 && ents.items == NULL);

    for (i = 0; i < 5; i++) {
        pico8_ents_add(&ents, &cells[i]);
    }
    assert(ents.count == 5 && ents.capacity >= 5);
    for (i = 0; i < 5; i++) {
        assert(ents.items[i] == &cells[i]);
    }

    pico8_ents_free(&ents);
    assert(ents.items == NULL && ents.count == 0 && ents.capacity == 0);
}

static void test_del_first_identical_and_has(void) {
    pico8_struct_header_t a, b, c, absent;
    pico8_ents_t ents;
    pico8_ents_init(&ents, &pico8_zone_transitoire);
    pico8_ents_add(&ents, &a);
    pico8_ents_add(&ents, &b);
    pico8_ents_add(&ents, &c);
    pico8_ents_add(&ents, &b);

    assert(pico8_ents_del(&ents, &b) == 1);
    assert(ents.count == 3);
    assert(ents.items[0] == &a && ents.items[1] == &c && ents.items[2] == &b);

    assert(pico8_ents_del(&ents, &absent) == 0);
    assert(ents.count == 3);

    assert(pico8_ents_has(&ents, &c) == 1);
    assert(pico8_ents_has(&ents, &absent) == 0);
    assert(pico8_ents_has(NULL, &a) == 0);
    assert(pico8_ents_has(&ents, NULL) == 0);
    pico8_ents_add(&ents, NULL);
    assert(pico8_ents_has(&ents, NULL) == 1);

    pico8_ents_free(&ents);
}

static void test_cat_copies_a_then_b_and_leaves_them(void) {
    pico8_struct_header_t a, b, c;
    pico8_ents_t l, r, t;
    pico8_ents_init(&l, &pico8_zone_transitoire);
    pico8_ents_init(&r, &pico8_zone_transitoire);
    pico8_ents_add(&l, &a);
    pico8_ents_add(&l, &b);
    pico8_ents_add(&r, &c);

    t = pico8_ents_cat(&pico8_zone_transitoire, &l, &r);
    assert(t.count == 3);
    assert(t.items[0] == &a && t.items[1] == &b && t.items[2] == &c);
    assert(l.count == 2 && l.items[0] == &a && l.items[1] == &b);
    assert(r.count == 1 && r.items[0] == &c);

    pico8_ents_free(&t);
    pico8_ents_free(&l);
    pico8_ents_free(&r);
}

static void test_all_sees_each_when_the_returned_one_is_deleted(void) {
    pico8_struct_header_t a, b, c;
    pico8_struct_header_t *seen[3];
    pico8_struct_header_t *e;
    int n = 0;
    pico8_ents_t ents;
    pico8_ents_iter_t it;
    pico8_ents_init(&ents, &pico8_zone_transitoire);
    pico8_ents_add(&ents, &a);
    pico8_ents_add(&ents, &b);
    pico8_ents_add(&ents, &c);

    it = pico8_ents_all(&ents);
    while (pico8_ents_iter_next(&it, &e)) {
        assert(n < 3);
        seen[n++] = e;
        pico8_ents_del(&ents, e);
    }
    assert(n == 3);
    assert(seen[0] == &a && seen[1] == &b && seen[2] == &c);
    assert(ents.count == 0);

    pico8_ents_free(&ents);
}

static int odd_rank(pico8_struct_header_t *e, void *ctx) {
    const pico8_struct_header_t *cells = ctx;
    return (e - cells) % 2 == 1;
}

static void test_jp_keeps_what_the_predicate_keeps_in_order(void) {
    pico8_struct_header_t cells[5];
    pico8_ents_t src;
    pico8_ents_t kept;
    int i;
    pico8_ents_init(&src, &pico8_zone_transitoire);
    for (i = 0; i < 5; i++) {
        pico8_ents_add(&src, &cells[i]);
    }

    kept = pico8_jp_ents(&pico8_zone_transitoire, &src, odd_rank, cells);
    assert(kept.count == 2 && kept.items[0] == &cells[1] && kept.items[1] == &cells[3]);
    assert(src.count == 5);
    for (i = 0; i < 5; i++) {
        assert(src.items[i] == &cells[i]);
    }
    pico8_ents_free(&kept);

    kept = pico8_jp_ents(&pico8_zone_transitoire, NULL, odd_rank, cells);
    assert(kept.count == 0);
    pico8_ents_free(&kept);
    pico8_ents_free(&src);
}

int main(void) {
    test_add_grows_past_four_keeps_order_and_free_empties();
    test_del_first_identical_and_has();
    test_cat_copies_a_then_b_and_leaves_them();
    test_all_sees_each_when_the_returned_one_is_deleted();
    test_jp_keeps_what_the_predicate_keeps_in_order();
    printf("test_pico8_ents: ok\n");
    return 0;
}
