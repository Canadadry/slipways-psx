#include "engine/entity_list.h"

#include <assert.h>
#include <stdio.h>

#include "zone_tas.h"

static void test_add_grows_past_four_keeps_order_and_free_empties(void) {
    pico8_struct_header_t cells[5];
    int i;
    pico8_ents_t ents;
    pico8_ents_init(&ents, &zone_tas);
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
    pico8_ents_init(&ents, &zone_tas);
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
    pico8_ents_init(&l, &zone_tas);
    pico8_ents_init(&r, &zone_tas);
    pico8_ents_add(&l, &a);
    pico8_ents_add(&l, &b);
    pico8_ents_add(&r, &c);

    t = pico8_ents_cat(&zone_tas, &l, &r);
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
    pico8_ents_init(&ents, &zone_tas);
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
    pico8_ents_init(&src, &zone_tas);
    for (i = 0; i < 5; i++) {
        pico8_ents_add(&src, &cells[i]);
    }

    kept = pico8_jp_ents(&zone_tas, &src, odd_rank, cells);
    assert(kept.count == 2 && kept.items[0] == &cells[1] && kept.items[1] == &cells[3]);
    assert(src.count == 5);
    for (i = 0; i < 5; i++) {
        assert(src.items[i] == &cells[i]);
    }
    pico8_ents_free(&kept);

    kept = pico8_jp_ents(&zone_tas, NULL, odd_rank, cells);
    assert(kept.count == 0);
    pico8_ents_free(&kept);
    pico8_ents_free(&src);
}

static void test_clear_keeps_items_and_capacity_and_takes_nothing_until_past_them(void) {
    pico8_struct_header_t cells[9];
    pico8_ents_t ents;
    const pico8_zone_choisie_t *arene = pico8_zone_arene_prendre(&zone_tas, "tu ents", 1024);
    int i;
    pico8_ents_init(&ents, arene);
    for (i = 0; i < 5; i++) {
        pico8_ents_add(&ents, &cells[i]);
    }
    pico8_struct_header_t **items = ents.items;
    const int capacity = ents.capacity;
    const size_t prise = pico8_zone_arene_prise(arene);

    pico8_ents_clear(&ents);
    assert(ents.count == 0 && ents.capacity == capacity && ents.items == items && ents.zone == arene);
    for (i = 0; i < capacity; i++) {
        pico8_ents_add(&ents, &cells[i]);
    }
    assert(pico8_zone_arene_prise(arene) == prise && "re-adding up to the kept capacity takes nothing");
    assert(ents.items == items && ents.items[0] == &cells[0] && ents.count == capacity);

    pico8_ents_add(&ents, &cells[capacity]);
    assert(ents.capacity == 2 * capacity && pico8_zone_arene_prise(arene) > prise && "past it, it grows as before");
    pico8_zone_arene_rendre(arene);
}

static void test_reserved_list_fills_its_capacity_without_taking_then_grows(void) {
    pico8_struct_header_t cells[4];
    pico8_ents_t ents;
    const pico8_zone_choisie_t *arene = pico8_zone_arene_prendre(&zone_tas, "tu ents", 1024);
    int i;
    pico8_ents_init_reserve(&ents, arene, 3);
    assert(ents.count == 0 && ents.capacity == 3 && ents.zone == arene);
    const size_t prise = pico8_zone_arene_prise(arene);

    for (i = 0; i < 3; i++) {
        pico8_ents_add(&ents, &cells[i]);
    }
    assert(pico8_zone_arene_prise(arene) == prise && "n adds into a list reserved for n take nothing");
    assert(ents.count == 3 && ents.capacity == 3);
    for (i = 0; i < 3; i++) {
        assert(ents.items[i] == &cells[i]);
    }

    pico8_ents_add(&ents, &cells[3]);
    assert(ents.capacity > 3 && pico8_zone_arene_prise(arene) > prise && "past the reserve, it grows in its zone");
    assert(ents.count == 4 && ents.items[0] == &cells[0] && ents.items[3] == &cells[3]);
    pico8_zone_arene_rendre(arene);
}

static void test_reserving_zero_takes_nothing(void) {
    pico8_ents_t ents;
    const pico8_zone_choisie_t *arene = pico8_zone_arene_prendre(&zone_tas, "tu ents", 1024);
    const size_t prise = pico8_zone_arene_prise(arene);
    pico8_ents_init_reserve(&ents, arene, 0);
    assert(ents.count == 0 && ents.capacity == 0 && ents.items == NULL && ents.zone == arene);
    assert(pico8_zone_arene_prise(arene) == prise && "a reserve of 0 takes nothing from its zone");
    pico8_zone_arene_rendre(arene);
}

int main(void) {
    test_add_grows_past_four_keeps_order_and_free_empties();
    test_del_first_identical_and_has();
    test_cat_copies_a_then_b_and_leaves_them();
    test_all_sees_each_when_the_returned_one_is_deleted();
    test_jp_keeps_what_the_predicate_keeps_in_order();
    test_clear_keeps_items_and_capacity_and_takes_nothing_until_past_them();
    test_reserved_list_fills_its_capacity_without_taking_then_grows();
    test_reserving_zero_takes_nothing();
    printf("test_pico8_ents: ok\n");
    return 0;
}
