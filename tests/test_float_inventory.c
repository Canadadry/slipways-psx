/* tools/float_inventory.sh, le corps de `make float-inventory`, sur des
 * sources C, une annotation et un lua.lua fabriques : il se teste par ce
 * qu'il doit empecher. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/float_inventory.sh"
#define DIR "tmp/test_float_inventory_work"
#define SRC DIR "/pico8_specimen.c"
#define OUT DIR "/out.txt"
#define ANNOT DIR "/annotation.txt"
#define LUA DIR "/lua.lua"

/* Ligne 2 seule porte un flr(. */
#define LUA_TEXT \
    "function dv(no)\n" \
    "spr(flr(p.x),0)\n" \
    "pset(cx+p.x,cy)\n"

/* Tampon statique, termine par NUL, ecrase a chaque appel. */
static const char *read_file(const char *path) {
    static char buf[1 << 16];
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Rend le code de sortie du script, -1 s'il n'a pas pu se terminer. */
static int run_inventory(const char *sources, const char *out) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " " ANNOT " " LUA " %s > %s 2>&1", sources, out);
    int status = system(cmd);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(LUA, LUA_TEXT);
    write_file(ANNOT, "");
}

static void test_constant_carries_function_rank_and_literal(void) {
    reset();
    write_file(SRC,
               "static pico8_fixed_t pico8_one(void) {\n"
               "    return pico8_fixed_from_double(0.27);\n"
               "}\n");
    assert(run_inventory(SRC, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC " | pico8_one | constante | 1 | ligne 2 | - | - | 0.27\n") != NULL);
}

#define SPECIMEN \
    "static pico8_fixed_t pico8_one(void) {\n" \
    "    return pico8_fixed_from_double(0.27);\n" \
    "}\n" \
    "\n" \
    "static void pico8_two(pico8_vec_t pos,\n" \
    "                      pico8_fixed_t k) {\n" \
    "    pico8_spr((int)pico8_fixed_to_double(pos.x), (int) pico8_fixed_to_double(f(pos.y)));\n" \
    "    pico8_fixed_t a = pico8_fixed_mul(k, pico8_fixed_from_double(1.5));\n" \
    "    pico8_fixed_t b = pico8_fixed_sin(a);\n" \
    "    double d = pow(pico8_fixed_to_double(b), 0.75);\n" \
    "    pico8_fixed_t e = pico8_fixed_from_double(d);\n" \
    "    snprintf(buf, sizeof buf, \"%.4f\", d);\n" \
    "}\n"

#define SPECIMEN_ANNOTATION \
    "# commentaire\n" \
    "\n" \
    SRC " | pico8_two | pos.x | 1 | a auditer | - | non classe\n" \
    SRC " | pico8_two | f(pos.y) | 1 | a auditer | - | non classe\n"

static void test_each_token_gets_one_category_and_a_rank_in_its_function(void) {
    reset();
    write_file(SRC, SPECIMEN);
    write_file(ANNOT, SPECIMEN_ANNOTATION);
    assert(run_inventory(SRC, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC " | pico8_one | constante | 1 | ligne 2 | - | - | 0.27\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | constante | 1 | ligne 8 | - | - | 1.5\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | cast | 1 | ligne 7 | a auditer | - | pos.x\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | cast | 2 | ligne 7 | a auditer | - | f(pos.y)\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | autre | 1 | ligne 10 | - | - | double\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | reecrit | 1 | ligne 10 | - | - | pow\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | autre | 2 | ligne 10 | - | - | pico8_fixed_to_double\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | reecrit | 2 | ligne 11 | - | - | pico8_fixed_from_double(d)\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | reecrit | 3 | ligne 12 | - | - | %.4f\n") != NULL);
    assert(strstr(out, "| sin\n") == NULL);
    assert(strstr(out, "compte constante 2\n") != NULL);
    assert(strstr(out, "compte cast 2\n") != NULL);
    assert(strstr(out, "compte reecrit 3\n") != NULL);
    assert(strstr(out, "compte autre 2\n") != NULL);
    assert(strstr(out, "compte total 9\n") != NULL);
    assert(strstr(out, "compte population a auditer 2\n") != NULL);
}

/* Dans pico8_fixed.c, le from_double qui enveloppe le sin de la libc est la
 * conversion elle-meme : seul le sin est un site reecrit. */
static void test_libc_sin_inside_fixed_module_is_the_rewritten_site(void) {
    reset();
    write_file(DIR "/pico8_fixed.c",
               "pico8_fixed_t pico8_fixed_sin(pico8_fixed_t x) {\n"
               "    return pico8_fixed_from_double(-sin(radians));\n"
               "}\n");
    assert(run_inventory(DIR "/pico8_fixed.c", OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "| pico8_fixed_sin | autre | 1 | ligne 2 | - | - | pico8_fixed_from_double(-sin(radians))\n") != NULL);
    assert(strstr(out, "| pico8_fixed_sin | reecrit | 1 | ligne 2 | - | - | sin\n") != NULL);
    assert(strstr(out, "compte reecrit 1\n") != NULL);
}

static void test_unclassified_float_token_stops_the_inventory(void) {
    reset();
    write_file(SRC,
               "static void pico8_one(void) {\n"
               "    double x = my_float_scale;\n"
               "}\n");
    assert(run_inventory(SRC, OUT) != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC ":2") != NULL);
    assert(strstr(out, "compte total") == NULL);
}

#define TWO_CASTS \
    "static void pico8_two(pico8_vec_t pos) {\n" \
    "    pico8_spr((int)pico8_fixed_to_double(pos.x), 0);\n" \
    "    pico8_spr((int)pico8_fixed_to_double(pos.y), 0);\n" \
    "}\n"

#define ANNOTATE_X(population_step_why) SRC " | pico8_two | pos.x | 1 | " population_step_why "\n"
#define ANNOTATE_Y(population_step_why) SRC " | pico8_two | pos.y | 1 | " population_step_why "\n"

static void test_orphan_annotation_stops_the_inventory(void) {
    reset();
    write_file(SRC, TWO_CASTS);
    write_file(ANNOT, ANNOTATE_X("a auditer | - | non classe")
                      ANNOTATE_Y("a auditer | - | non classe")
                      SRC " | pico8_gone | pos.z | 1 | a auditer | - | non classe\n");
    assert(run_inventory(SRC, OUT) == 4);
    const char *out = read_file(OUT);
    assert(strstr(out, "orpheline") != NULL);
    assert(strstr(out, "pico8_gone | pos.z | 1") != NULL);
    assert(strstr(out, "compte total") == NULL);
}

static void test_cast_without_annotation_stops_the_inventory(void) {
    reset();
    write_file(SRC, TWO_CASTS);
    write_file(ANNOT, ANNOTATE_X("a auditer | - | non classe"));
    assert(run_inventory(SRC, OUT) == 4);
    const char *out = read_file(OUT);
    assert(strstr(out, "cast non annote : " SRC " | pico8_two | pos.y | 1") != NULL);
    assert(strstr(out, "compte total") == NULL);
}

static void test_population_a_must_cite_a_lua_line_with_flr(void) {
    reset();
    write_file(SRC, TWO_CASTS);
    write_file(ANNOT, ANNOTATE_X("(a) | C1 | spr(flr(...)) en lua.lua:3, compteur >= 0")
                      ANNOTATE_Y("a auditer | - | non classe"));
    assert(run_inventory(SRC, OUT) == 4);
    assert(strstr(read_file(OUT), "lua.lua:3 ne contient pas flr(") != NULL);

    write_file(ANNOT, ANNOTATE_X("(a) | C1 | spr sans renvoi")
                      ANNOTATE_Y("a auditer | - | non classe"));
    assert(run_inventory(SRC, OUT) == 4);

    write_file(ANNOT, ANNOTATE_X("(a) | C1 | spr(flr(...)) en lua.lua:2, compteur >= 0")
                      ANNOTATE_Y("a auditer | - | non classe"));
    assert(run_inventory(SRC, OUT) == 0);
    assert(strstr(read_file(OUT), SRC " | pico8_two | cast | 1 | ligne 2 | (a) | C1 | pos.x\n") != NULL);
}

static void test_step_must_follow_population(void) {
    reset();
    write_file(SRC, TWO_CASTS);
    write_file(ANNOT, ANNOTATE_X("(b) | D1 | compteur de boucle")
                      ANNOTATE_Y("a auditer | - | non classe"));
    assert(run_inventory(SRC, OUT) == 4);
    write_file(ANNOT, ANNOTATE_X("(c) | C1 | coordonnee ecran")
                      ANNOTATE_Y("a auditer | - | non classe"));
    assert(run_inventory(SRC, OUT) == 4);
    write_file(ANNOT, ANNOTATE_X("a auditer | C1 | non classe")
                      ANNOTATE_Y("a auditer | - | non classe"));
    assert(run_inventory(SRC, OUT) == 4);
    write_file(ANNOT, ANNOTATE_X("(b) | C1 |  ")
                      ANNOTATE_Y("a auditer | - | non classe"));
    assert(run_inventory(SRC, OUT) == 4);
    write_file(ANNOT, ANNOTATE_X("(b) | C1 | compteur de boucle")
                      ANNOTATE_Y("(c) | D1 | coordonnee ecran"));
    assert(run_inventory(SRC, OUT) == 0);
}

/* Convertir un cast (C1) ne doit pas faire glisser l'annotation de son
 * frere sur lui, comme le ferait une cle par rang. */
static void test_removing_a_cast_keeps_its_sibling_annotation(void) {
    reset();
    write_file(SRC,
               "static void pico8_two(pico8_vec_t pos) {\n"
               "    pico8_spr(0, 0);\n"
               "    pico8_spr((int)pico8_fixed_to_double(pos.y), 0);\n"
               "}\n");
    write_file(ANNOT, ANNOTATE_X("(b) | C1 | compteur de boucle")
                      ANNOTATE_Y("(c) | D1 | coordonnee ecran"));
    assert(run_inventory(SRC, OUT) == 4);
    assert(strstr(read_file(OUT), "orpheline, aucun cast " SRC " | pico8_two | pos.x | 1") != NULL);

    write_file(ANNOT, ANNOTATE_Y("(c) | D1 | coordonnee ecran"));
    assert(run_inventory(SRC, OUT) == 0);
    assert(strstr(read_file(OUT), SRC " | pico8_two | cast | 1 | ligne 3 | (c) | D1 | pos.y\n") != NULL);
}

static void test_occurrence_separates_casts_of_the_same_extract(void) {
    reset();
    write_file(SRC,
               "static void pico8_cr(pico8_fixed_t tl) {\n"
               "    pico8_spr((int)pico8_fixed_to_double(tl), 0);\n"
               "    pico8_spr((int)pico8_fixed_to_double(tl), 1);\n"
               "}\n");
    write_file(ANNOT, SRC " | pico8_cr | tl | 2 | (c) | D1 | peut etre negatif\n"
                      SRC " | pico8_cr | tl | 1 | (b) | C1 | compteur\n");
    assert(run_inventory(SRC, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC " | pico8_cr | cast | 1 | ligne 2 | (b) | C1 | tl\n") != NULL);
    assert(strstr(out, SRC " | pico8_cr | cast | 2 | ligne 3 | (c) | D1 | tl\n") != NULL);
}

static void test_counts_by_population_and_step(void) {
    reset();
    write_file(SRC,
               "static void pico8_cr(pico8_fixed_t tl) {\n"
               "    pico8_spr((int)pico8_fixed_to_double(tl), (int)pico8_fixed_to_double(tl));\n"
               "    pico8_spr((int)pico8_fixed_to_double(tl), (int)pico8_fixed_to_double(tl));\n"
               "    pico8_spr((int)pico8_fixed_to_double(tl), (int)pico8_fixed_to_double(tl));\n"
               "}\n");
    write_file(ANNOT, SRC " | pico8_cr | tl | 1 | (a) | C1 | lua.lua:2, compteur\n"
                      SRC " | pico8_cr | tl | 2 | (a) | D1 | lua.lua:2, signe non prouve\n"
                      SRC " | pico8_cr | tl | 3 | (b) | C1 | compteur\n"
                      SRC " | pico8_cr | tl | 4 | (c) | D1 | delta\n"
                      SRC " | pico8_cr | tl | 5 | (c) | D1 | delta\n"
                      SRC " | pico8_cr | tl | 6 | a auditer | - | NON COUVERT\n");
    assert(run_inventory(SRC, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "compte population (a) 2\n") != NULL);
    assert(strstr(out, "compte population (b) 1\n") != NULL);
    assert(strstr(out, "compte population (c) 2\n") != NULL);
    assert(strstr(out, "compte population a auditer 1\n") != NULL);
    assert(strstr(out, "compte etape C1 2\n") != NULL);
    assert(strstr(out, "compte etape D1 3\n") != NULL);
    assert(strstr(out, "compte etape a auditer 1\n") != NULL);
}

static void test_lines_added_above_change_only_line_numbers(void) {
    reset();
    write_file(SRC, SPECIMEN);
    write_file(ANNOT, SPECIMEN_ANNOTATION);
    assert(run_inventory(SRC, DIR "/before.txt") == 0);
    write_file(SRC, "\n\n\n" SPECIMEN);
    assert(run_inventory(SRC, DIR "/after.txt") == 0);
    assert(system("cmp -s " DIR "/before.txt " DIR "/after.txt") != 0);
    assert(system("sed 's/| ligne [0-9]* |/|/' " DIR "/before.txt > " DIR "/before.id && "
                  "sed 's/| ligne [0-9]* |/|/' " DIR "/after.txt > " DIR "/after.id && "
                  "cmp -s " DIR "/before.id " DIR "/after.id") == 0);
}

static void test_two_runs_give_identical_reports(void) {
    reset();
    write_file(SRC, SPECIMEN);
    write_file(ANNOT, SPECIMEN_ANNOTATION);
    assert(run_inventory(SRC, DIR "/one.txt") == 0);
    assert(run_inventory(SRC, DIR "/two.txt") == 0);
    assert(system("cmp -s " DIR "/one.txt " DIR "/two.txt") == 0);
}

int main(void) {
    test_constant_carries_function_rank_and_literal();
    test_each_token_gets_one_category_and_a_rank_in_its_function();
    test_libc_sin_inside_fixed_module_is_the_rewritten_site();
    test_unclassified_float_token_stops_the_inventory();
    test_lines_added_above_change_only_line_numbers();
    test_two_runs_give_identical_reports();
    test_orphan_annotation_stops_the_inventory();
    test_cast_without_annotation_stops_the_inventory();
    test_population_a_must_cite_a_lua_line_with_flr();
    test_step_must_follow_population();
    test_removing_a_cast_keeps_its_sibling_annotation();
    test_occurrence_separates_casts_of_the_same_extract();
    test_counts_by_population_and_step();
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_inventory: all tests passed\n");
    return 0;
}
