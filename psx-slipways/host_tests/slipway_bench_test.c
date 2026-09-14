#include "t.h"
#include "../planetgen.h"
#include "../slipway_bench.h"

#include <string.h>

static void set_edge(neighbor_graph_t *g, int i, int j) {
    g->adjacent[i][j] = true;
    g->adjacent[j][i] = true;
}

static void build_fully_connected_graph(neighbor_graph_t *g, int count) {
    memset(g->adjacent, 0, sizeof(g->adjacent));
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            set_edge(g, i, j);
        }
    }
}

static void test_cap_is_never_exceeded(void) {
    neighbor_graph_t g;
    build_fully_connected_graph(&g, 10);

    slipway_edge_t out[5];
    int count = slipway_bench_select_edges(&g, 10, 5, out);

    T_CHECK(count == 5);
}

static void test_returned_count_is_min_of_actual_edges_and_cap(void) {
    neighbor_graph_t g;
    memset(g.adjacent, 0, sizeof(g.adjacent));

    set_edge(&g, 0, 1);
    set_edge(&g, 1, 2);
    set_edge(&g, 2, 3);

    slipway_edge_t out[50];
    int count = slipway_bench_select_edges(&g, 4, 50, out);

    T_CHECK(count == 3);

    int capped_count = slipway_bench_select_edges(&g, 4, 2, out);
    T_CHECK(capped_count == 2);
}

static void test_selection_is_deterministic(void) {
    neighbor_graph_t g;
    build_fully_connected_graph(&g, 8);

    slipway_edge_t out_a[10];
    slipway_edge_t out_b[10];

    int count_a = slipway_bench_select_edges(&g, 8, 10, out_a);
    int count_b = slipway_bench_select_edges(&g, 8, 10, out_b);

    T_CHECK(count_a == count_b);
    for (int i = 0; i < count_a && i < count_b; i++) {
        T_CHECK(out_a[i].a == out_b[i].a);
        T_CHECK(out_a[i].b == out_b[i].b);
    }
}

static void test_no_edge_written_when_graph_has_none(void) {
    neighbor_graph_t g;
    memset(g.adjacent, 0, sizeof(g.adjacent));

    slipway_edge_t out[10];
    int count = slipway_bench_select_edges(&g, 5, 10, out);

    T_CHECK(count == 0);
}

int main(void) {
    t_run("cap_is_never_exceeded", test_cap_is_never_exceeded);
    t_run("returned_count_is_min_of_actual_edges_and_cap", test_returned_count_is_min_of_actual_edges_and_cap);
    t_run("selection_is_deterministic", test_selection_is_deterministic);
    t_run("no_edge_written_when_graph_has_none", test_no_edge_written_when_graph_has_none);
    return t_report();
}
