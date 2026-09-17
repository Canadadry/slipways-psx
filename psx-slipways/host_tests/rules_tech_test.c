#include "t.h"
#include "../rules_tech.h"

#include <string.h>

static void test_space_folding_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_SPACE_FOLDING);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "space folding") == 0);
    T_CHECK(t->base_cost == 4);
}

static void test_geneseeds_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_GENESEEDS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "geneseeds") == 0);
    T_CHECK(t->base_cost == 5);
}

static void test_nanomaterials_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_NANOMATERIALS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "nanomaterials") == 0);
    T_CHECK(t->base_cost == 6);
}

static void test_drillbots_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_DRILLBOTS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "drillbots") == 0);
    T_CHECK(t->base_cost == 7);
}

int main(void) {
    t_run("space_folding_matches_traced_values", test_space_folding_matches_traced_values);
    t_run("geneseeds_matches_traced_values", test_geneseeds_matches_traced_values);
    t_run("nanomaterials_matches_traced_values", test_nanomaterials_matches_traced_values);
    t_run("drillbots_matches_traced_values", test_drillbots_matches_traced_values);
    return t_report();
}
