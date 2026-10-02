/* tools/composition_residue.sh, le corps de `make composition-residue`, sur un
 * mini-arbre fabrique.
 * Le script lit tests/ : chaque jeton de fixture est coupe en deux litteraux
 * ("get" "_pos", "n" "u"), sinon ce fichier entrerait dans le residu qu'il
 * mesure. Il ne porte non plus aucun jeton de tools/data_residue.sh, que
 * make test garde a 0 dans tests/. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_composition_residue_work"
#define SRC DIR "/src"
#define TST DIR "/tests"
#define OUT DIR "/out.txt"
#define ERR DIR "/err.txt"
#define TREE SRC " " TST
#define ATT DIR "/reachability.txt"
#define RESIDUE_2_SITES DIR "/r2.txt"
#define EMPTY_FOOTER "residue 2: 0 line(s), 0 file(s)\n"

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

/* Rend le code de sortie du script, relu depuis un fichier : le statut brut
 * de system() n'est pas decodable sans <sys/wait.h>, hors C99. La sortie
 * standard va dans OUT, l'erreur dans ERR. */
static int run_residue(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd,
             "COMPOSITION_REACHABILITY=" ATT " COMPOSITION_R2=" RESIDUE_2_SITES
             " sh tools/composition_residue.sh %s > " OUT " 2> " ERR "; echo $? > " DIR "/code",
             args);
    assert(system(cmd) == 0);
    return atoi(read_file(DIR "/code"));
}

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " SRC "/engine " SRC "/game " TST) == 0);
    write_file(SRC "/clean.c", "int main(void) { return 0; }\n");
    write_file(TST "/clean_test.c", "int helper(int x) { return x + 1; }\n");
    write_file(ATT, "");
    write_file(RESIDUE_2_SITES, "");
}

/* Deux types d'entite : ke (en-tete en premier membre, pas une parente) et
 * jo (ke en premier membre, une parente). */
#define ENTITY_PLANET "typedef struct {\n    entity" "_t header;\n} entity_" "root_t;\n" \
              "typedef struct {\n    entity_" "root_t ke;\n    int a;\n} pico8_" "jo_t;\n"

static void assert_alone_is_empty(const char *path, const char *text) {
    setup();
    write_file(path, text);
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), EMPTY_FOOTER) == 0);
}

static void test_clean_tree_prints_only_the_empty_footer(void) {
    setup();
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), EMPTY_FOOTER) == 0);
}

static void test_one_occurrence_per_category_in_table_order(void) {
    setup();
    write_file(SRC "/c1.c", "if (pico8_" "qh_of(pt) != 0) {}\n");
    write_file(SRC "/c2.c", "int r = self->get" "_pos(self);\n");
    write_file(SRC "/c3.c", "self->rr = pico8_" "nl_start.rr;\n");
    write_file(SRC "/c4.c", "return self->n" "u;\n");
    write_file(TST "/c5.c", "hp_" "pose(self, p);\n");
    write_file(TST "/c6.c", "e->pk_" "is_bool = 1;\n");
    write_file(SRC "/engine/entity_class.h", "#include \"game/" "game_data.h\"\n");
    write_file(SRC "/e.h", "typedef struct {\n    entity" "_t header;\n    pointer_" "list_t *oe;\n} pico8_"
                           "jo_t;\n");
    write_file(SRC "/p.h", "typedef struct {\n    pico8_" "jo_t jo;\n} pico8_" "qh_t;\n");
    write_file(SRC "/v.h", "particle_t *pico8_" "g__mm(void *" "self);\n");
    write_file(ATT, "q1\tx\n");
    write_file(SRC "/a.h", "#include \"b.h\"\n");
    write_file(SRC "/b.h", "#include \"a.h\"\n");
    write_file(SRC "/k.h", "int k_f(void);\n");
    write_file(SRC "/k.c", "int k = l_g();\n");
    write_file(SRC "/l.h", "int l_g(void);\n");
    write_file(SRC "/l.c", "int l = k_f();\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "parent\t1\t1\n"
                                  "parent-link\t1\t1\n"
                                  "header-accessors\t1\t1\n"
                                  "defaults\t1\t1\n"
                                  "n" "u\t1\t1\n"
                                  "name-dispatch\t1\t1\n"
                                  "one-or-false\t1\t1\n"
                                  "void-ptr\t1\t1\n"
                                  "void-self\t1\t1\n"
                                  "entity-ptrs\t1\t1\n"
                                  "core-includes\t1\t1\n"
                                  "reachability-to-prove\t1\t1\n"
                                  "mutual-inclusion\t1\t2\n"
                                  "mutual-usage\t1\t2\n"
                                  "residue 2: 15 line(s), 15 file(s)\n") == 0);
}

static void test_other_witnesses_of_each_pattern_bite(void) {
    setup();
    write_file(SRC "/w.c", "static k_t k = { .pa" "rent = x };\n"
                           "q = (*pa" "rent)(self);\n"
                           "q = self->pa" "rent;\n"
                           "q = emitter" "_of(self);\n"
                           "q = g__probe" "_of(self);\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "parent-link\t5\t1\n"
                                  "residue 2: 5 line(s), 1 file(s)\n") == 0);
    setup();
    write_file(SRC "/w.c", "int a = pico8_jo_get" "_fold_anchor(self);\n"
                           "int b = X_get" "_is_dead(self);\n"
                           "pico8_jo_set" "_is_hovered(self, v);\n"
                           "pico8_" "nl_start_t s;\n"
                           "pico8_" "jo_kw_start;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "header-accessors\t3\t1\n"
                                  "defaults\t2\t1\n"
                                  "residue 2: 5 line(s), 1 file(s)\n") == 0);
    setup();
    write_file(SRC "/w.c", "extern const str_seq_t pico8_nl_n" "u_tags;\n"
                           "int k = x_n" "u_items;\n"
                           "if (pico8_ho_" "gp_has(self->g" "p, name)) {}\n"
                           "q = pico8_hp_" "mr_names;\n"
                           "q = self.m" "r;\n"
                           "hp_" "ho(self);\n"
                           "self->probe->fp_" "is_bool = 1;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "n" "u\t2\t1\n"
                                  "name-dispatch\t4\t1\n"
                                  "one-or-false\t1\t1\n"
                                  "residue 2: 7 line(s), 1 file(s)\n") == 0);
    setup();
    write_file(SRC "/engine/entity.h", "#include \"game/" "map_tables.h\"\n");
    write_file(SRC "/engine/entity_registry.h", "#include \"game/" "data_init.h\"\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "core-includes\t2\t2\n"
                                  "residue 2: 2 line(s), 2 file(s)\n") == 0);
}

static void test_named_false_positives_do_not_bite(void) {
    assert_alone_is_empty(SRC "/f.c", "e = entity_" "state_of(me);\n");
    assert_alone_is_empty(SRC "/f.c", "/* the parent struct */\n");
    assert_alone_is_empty(SRC "/f.c", "int budget_pos;\n");
    assert_alone_is_empty(SRC "/f.c", "int get_posa;\n");
    assert_alone_is_empty(SRC "/f.c", "int get_pos_x;\n");
    assert_alone_is_empty(SRC "/f.c", "int run_start;\n");
    assert_alone_is_empty(SRC "/f.c", "int number;\n");
    assert_alone_is_empty(SRC "/f.c", "number(e, 1);\n");
    assert_alone_is_empty(TST "/f.c", "\"pico8_jo_" "default(\\\"n" "u\\\", &k);\\n\"\n");
    assert_alone_is_empty(SRC "/f.c", "int mr = 0;\n");
    assert_alone_is_empty(SRC "/f.c", "self->resource_is_bool = 0;\n");
    assert_alone_is_empty(SRC "/f.c", "self->background_is_bool = 0;\n");
    assert_alone_is_empty(SRC "/f.c", "k.side_tooltip_is_false = 1;\n");
    assert_alone_is_empty(SRC "/engine/entity_class.h", "#include \"engine/vec.h\"\n");
    assert_alone_is_empty(SRC "/pico8_planet.h", "#include \"game/" "game_data.h\"\n");
}

static void test_exclusions_edit_the_text_not_the_whole_line(void) {
    setup();
    write_file(SRC "/x.c", "if (entity_" "state_of(me) && pico8_" "qh_of(pt)) {}\n");
    write_file(TST "/x.c", "\"k(\\\"n" "u\\\")\" ; q = self->n" "u;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "parent-link\t1\t1\n"
                                  "n" "u\t1\t1\n"
                                  "residue 2: 2 line(s), 2 file(s)\n") == 0);
}

static void test_overlapping_categories_count_each_but_footer_counts_distinct_lines(void) {
    setup();
    write_file(SRC "/o.h", "const str_seq_t *pico8_jo_get" "_n" "u(const void *" "self);\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "header-accessors\t1\t1\n"
                                  "n" "u\t1\t1\n"
                                  "void-self\t1\t1\n"
                                  "residue 2: 1 line(s), 1 file(s)\n") == 0);
}

static void test_entity_reader_finds_embedded_parents_by_closure(void) {
    setup();
    write_file(SRC "/c.h", "typedef struct {\n    pico8_" "nl_t nl;\n} pico8_" "os_t;\n");
    write_file(SRC "/d.h", "typedef struct {\n    pico8_" "qh_t qh;\n} pico8_" "nl_t;\n");
    write_file(SRC "/e.h", ENTITY_PLANET "typedef struct {\n    pico8_" "jo_t jo;\n} pico8_" "qh_t;\n"
                              "struct pico8_" "mh {\n    /* c */\n\n    entity_" "root_t ke;\n};\n"
                              "typedef struct pico8_" "mh pico8_" "mh_t;\n"
                              "struct pico8_" "x {\n    pico8_" "mh_t mh;\n};\n");
    assert(run_residue("--check parent " TREE) == 1);
    assert(strcmp(read_file(OUT), "parent: " SRC "/c.h:2:    pico8_" "nl_t nl;\n"
                                  "parent: " SRC "/d.h:2:    pico8_" "qh_t qh;\n"
                                  "parent: " SRC "/e.h:5:    entity_" "root_t ke;\n"
                                  "parent: " SRC "/e.h:9:    pico8_" "jo_t jo;\n"
                                  "parent: " SRC "/e.h:14:    entity_" "root_t ke;\n"
                                  "parent: " SRC "/e.h:18:    pico8_" "mh_t mh;\n"
                                  "guarded residue (parent): 6 line(s)\n") == 0);
}

static void test_entity_reader_ignores_what_is_not_an_embedded_parent(void) {
    setup();
    write_file(SRC "/e.h", ENTITY_PLANET);
    write_file(SRC "/f.c", "typedef struct {\n    pico8_" "jo_t *jo;\n} ptr_t;\n"
                           "typedef struct {\n    int a;\n    pico8_" "jo_t jo;\n} second_t;\n"
                           "int f(void) {\n    pico8_" "jo_t jo;\n    return 0;\n}\n"
                           "typedef struct {\n    p8_fixed_t x;\n} fixed_first_t;\n"
                           "void g(void) {\nstruct local {\n    pico8_" "jo_t jo;\n} l;\n}\n"
                           "typedef struct {\n    pico8_" "jo_t jo;\n} after_t;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "parent\t2\t2\n"
                                  "residue 2: 2 line(s), 2 file(s)\n") == 0);
}

static void test_pointer_list_members_count_only_on_entity_types(void) {
    setup();
    write_file(SRC "/e.h", "typedef struct {\n    entity" "_t header;\n} entity_" "root_t;\n"
                           "typedef struct {\n    entity_" "root_t ke;\n    pointer_" "list_t *oe;\n    pointer_"
                           "list_t *needs;\n} planet_" "t;\n"
                           "typedef struct {\n    int a;\n    pointer_" "list_t *qy;\n} other_t;\n"
                           "void f(void) {\n    pointer_" "list_t list;\n}\n");
    assert(run_residue("--check entity-ptrs " TREE) == 1);
    assert(strcmp(read_file(OUT), "entity-ptrs: " SRC "/e.h:6:    pointer_" "list_t *oe;\n"
                                  "guarded residue (entity-ptrs): 1 line(s)\n") == 0);
}

static void test_registries_reads_declaration_points_outside_entity_types(void) {
    setup();
    write_file(SRC "/e.h", "typedef struct {\n    entity" "_t header;\n    pointer_"
                           "list_t *oe;\n} entity_" "root_t;\n"
                           "typedef struct {\n    pointer_" "list_t layers[16];\n    pointer_" "list_t *particles;\n} emitter_"
                           "t;\n"
                           "typedef struct {\n    void *(*new_entity)(int k);\n    void (*kj)(void *" "self);\n} k_t;\n"
                           "const pointer_" "list_t *live_" "entities(void);\n"
                           "layout_row_t *research_display_" "planet_resource_row(const pointer_" "list_t *is,\n    int o);\n"
                           "typedef int (*pico8_" "fn)(const pointer_" "list_t *p);\n");
    write_file(SRC "/r.c", "static pointer_" "list_t g_live_entities;\n"
                           "static pointer_" "list_t *g_list = 0;\n"
                           "pointer_" "list_t collect_" "filter_pointer_list(const pointer_" "list_t *qk) {\n    pointer_"
                           "list_t f;\n    return f;\n}\n");
    write_file(SRC "/engine/pointer_list.h", "typedef struct {\n    const pointer_" "list_t *ptrs;\n} pointer_" "list_iter_t;\n"
                                    "void pointer_" "list_add(pointer_" "list_t *p, void *e);\n");
    write_file(TST "/t.c", "static pointer_" "list_t g_t;\n");
    assert(run_residue("--check registries " TREE) == 1);
    assert(strcmp(read_file(OUT), "registries: " SRC "/e.h:6:    pointer_" "list_t layers[16];\n"
                                  "registries: " SRC "/e.h:10:    void *(*new_entity)(int k);\n"
                                  "registries: " SRC "/e.h:13:const pointer_" "list_t *live_" "entities(void);\n"
                                  "registries: " SRC "/r.c:1:static pointer_" "list_t g_live_entities;\n"
                                  "registries: " SRC "/r.c:2:static pointer_" "list_t *g_list = 0;\n"
                                  "guarded residue (registries): 5 line(s)\n") == 0);
}

/* Formes d'un registre que le lecteur voit (F) et limites qu'il ne voit pas
 * (L), chacune a sa ligne : une limite qui se met a compter change
 * l'attendu. */
static void test_registries_sees_hidden_forms_and_not_its_named_limits(void) {
    setup();
    write_file(SRC "/f.h", "typedef pointer_" "list_t reg_t;\n"
                           "typedef reg_t reg2_t;\n"
                           "typedef struct {\n"
                           "    pointer_" "list_t *a, *b;\n"
                           "    pointer_" "list_t **pp;\n"
                           "    const pointer_" "list_t *const x;\n"
                           "    void **items;\n"
                           "    pointer_" "list_t (*get)(void);\n"
                           "    void *slots[8];\n"
                           "    void *owner;\n"
                           "    struct {\n"
                           "        pointer_" "list_t l;\n"
                           "    } inner;\n"
                           "} holder_t;\n"
                           "static inline const pointer_" "list_t *reg_get(void) { return 0; }\n"
                           "void **pico8_" "reg_items(void);\n"
                           "extern pointer_" "list_t g_reg;\n"
                           "typedef struct { pointer_" "list_t l; } one_t;\n");
    write_file(SRC "/f.c", "static void reg_f(void) {\n"
                           "    static pointer_" "list_t reg;\n"
                           "    pointer_" "list_t l;\n"
                           "    static struct {\n"
                           "        pointer_" "list_t l;\n"
                           "    } g;\n"
                           "    (void)reg;\n"
                           "}\n"
                           "static reg_t g_alias;\n"
                           "static reg2_t g_alias2;\n"
                           "static struct {\n"
                           "    pointer_" "list_t list;\n"
                           "} g_anon;\n"
                           "static struct reg_s {\n"
                           "    pointer_" "list_t list;\n"
                           "} g_named;\n"
                           "static struct { pointer_" "list_t l; } g_one;\n"
                           "static void *g_void[64];\n"
                           "static pointer_" "list_t g_a, g_b;\n"
                           "pointer_" "list_t g_pub;\n"
                           "typedef void *handle_t;\n"
                           "static handle_t g_h[64];\n"
                           "#define REG(n) pointer_" "list_t n\n"
                           "static REG(g_m);\n"
                           "int reg_n(const pointer_" "list_t *p) {\n"
                           "    return p->count;\n"
                           "}\n");
    write_file(TST "/t.c", "static void *g_t[4];\n");
    assert(run_residue("--check registries " TREE) == 1);
    assert(strcmp(read_file(OUT), "registries: " SRC "/f.c:2:    static pointer_" "list_t reg;\n"
                                  "registries: " SRC "/f.c:5:        pointer_" "list_t l;\n"
                                  "registries: " SRC "/f.c:9:static reg_t g_alias;\n"
                                  "registries: " SRC "/f.c:12:    pointer_" "list_t list;\n"
                                  "registries: " SRC "/f.c:15:    pointer_" "list_t list;\n"
                                  "registries: " SRC "/f.c:17:static struct { pointer_" "list_t l; } g_one;\n"
                                  "registries: " SRC "/f.c:18:static void *g_void[64];\n"
                                  "registries: " SRC "/f.c:19:static pointer_" "list_t g_a, g_b;\n"
                                  "registries: " SRC "/f.c:20:pointer_" "list_t g_pub;\n"
                                  "registries: " SRC "/f.h:4:    pointer_" "list_t *a, *b;\n"
                                  "registries: " SRC "/f.h:5:    pointer_" "list_t **pp;\n"
                                  "registries: " SRC "/f.h:6:    const pointer_" "list_t *const x;\n"
                                  "registries: " SRC "/f.h:7:    void **items;\n"
                                  "registries: " SRC "/f.h:8:    pointer_" "list_t (*get)(void);\n"
                                  "registries: " SRC "/f.h:9:    void *slots[8];\n"
                                  "registries: " SRC "/f.h:15:static inline const pointer_" "list_t *reg_get(void) { return 0; }\n"
                                  "registries: " SRC "/f.h:16:void **pico8_" "reg_items(void);\n"
                                  "registries: " SRC "/f.h:17:extern pointer_" "list_t g_reg;\n"
                                  "registries: " SRC "/f.h:18:typedef struct { pointer_" "list_t l; } one_t;\n"
                                  "guarded residue (registries): 19 line(s)\n") == 0);
}

/* ptrs-entité lit les membres d'un type d'entité comme registres lit ceux des
 * autres structs, avec les mêmes limites (void *owner, :15). needs est exempté
 * seul, pas other_needs sur sa ligne (:16) ; (*mk) n'a pas de nom lisible
 * (:17). */
static void test_pointer_list_entity_reads_entity_members_like_registries(void) {
    setup();
    write_file(SRC "/a.h", "typedef pointer_" "list_t reg17_t;\n");
    write_file(SRC "/e.h", "typedef struct {\n    entity" "_t header;\n} entity_" "root_t;\n"
                           "typedef struct {\n"
                           "    entity_" "root_t ke;\n"
                           "    void **items;\n"
                           "    void *slots[8];\n"
                           "    reg17_t l;\n"
                           "    pointer_" "list_t arr[4];\n"
                           "    pointer_" "list_t **pp;\n"
                           "    pointer_" "list_t *a, *b;\n"
                           "    const pointer_" "list_t *const x;\n"
                           "    void *(*get)(void);\n"
                           "    _Alignas(8) void **al;\n"
                           "    void *owner;\n"
                           "    pointer_" "list_t *needs, *other_needs;\n"
                           "    pointer_" "list_t (*mk)(void);\n"
                           "    _Alignas (8) void **al2;\n"
                           "} planet_" "t;\n"
                           "typedef struct pico8_" "nl_s\n"
                           "{\n"
                           "    planet_" "t jo;\n"
                           "    void **more;\n"
                           "} pico8_" "nl_t;\n");
    assert(run_residue("--check registries " TREE) == 0);
    assert(run_residue("--check entity-ptrs " TREE) == 1);
    assert(strcmp(read_file(OUT), "entity-ptrs: " SRC "/e.h:6:    void **items;\n"
                                  "entity-ptrs: " SRC "/e.h:7:    void *slots[8];\n"
                                  "entity-ptrs: " SRC "/e.h:8:    reg17_t l;\n"
                                  "entity-ptrs: " SRC "/e.h:9:    pointer_" "list_t arr[4];\n"
                                  "entity-ptrs: " SRC "/e.h:10:    pointer_" "list_t **pp;\n"
                                  "entity-ptrs: " SRC "/e.h:11:    pointer_" "list_t *a, *b;\n"
                                  "entity-ptrs: " SRC "/e.h:12:    const pointer_" "list_t *const x;\n"
                                  "entity-ptrs: " SRC "/e.h:13:    void *(*get)(void);\n"
                                  "entity-ptrs: " SRC "/e.h:14:    _Alignas(8) void **al;\n"
                                  "entity-ptrs: " SRC "/e.h:16:    pointer_" "list_t *needs, *other_needs;\n"
                                  "entity-ptrs: " SRC "/e.h:17:    pointer_" "list_t (*mk)(void);\n"
                                  "entity-ptrs: " SRC "/e.h:18:    _Alignas (8) void **al2;\n"
                                  "entity-ptrs: " SRC "/e.h:23:    void **more;\n"
                                  "guarded residue (entity-ptrs): 13 line(s)\n") == 0);
}

/* Deux types d'entité de deux fichiers, un membre au même numéro de ligne :
 * deux lignes, la clé qui les dédoublonne tient au type, pas au numéro. */
static void test_pointer_list_entity_keeps_one_line_number_in_two_files(void) {
    setup();
    write_file(SRC "/e.h", "typedef struct {\n    entity" "_t header;\n} entity_" "root_t;\n"
                           "typedef struct {\n    entity_" "root_t ke;\n    void **items;\n} pico8_" "jo_t;\n");
    write_file(SRC "/g.h", "/* g */\n\n\n"
                           "typedef struct {\n    entity_" "root_t ke;\n    void **more;\n} pico8_" "qh_t;\n");
    assert(run_residue("--check entity-ptrs " TREE) == 1);
    assert(strcmp(read_file(OUT), "entity-ptrs: " SRC "/e.h:6:    void **items;\n"
                                  "entity-ptrs: " SRC "/g.h:6:    void **more;\n"
                                  "guarded residue (entity-ptrs): 2 line(s)\n") == 0);
}

/* const et volatile autour de void et des *, plusieurs déclarations sur une
 * ligne (comptée une fois), déclarateur à la ligne suivante, attribut à deux
 * niveaux : vus. Limites (attendu 0) : attribut à trois niveaux (e.h:18),
 * second nom d'une ligne de portée fichier (f.c:3). Formes voisines à 0 :
 * e.h:19 à :27, f.c:2, f.h:3. */
static void test_readers_see_qualifiers_split_declarations_and_nested_attributes(void) {
    setup();
    write_file(SRC "/e.h", "typedef struct {\n    entity" "_t header;\n} entity_" "root_t;\n"
                           "typedef struct {\n"
                           "    entity_" "root_t ke;\n"
                           "    void const **q1;\n"
                           "    volatile void **q2;\n"
                           "    void * volatile *q3;\n"
                           "    int k2; void **items;\n"
                           "    pointer_" "list_t *needs; pointer_" "list_t *hidden;\n"
                           "    void **d1; void **d2;\n"
                           "    void **\n"
                           "        split;\n"
                           "    pointer_" "list_t\n"
                           "        *next_line;\n"
                           "    void **nested __attribute__((aligned(sizeof(int))));\n"
                           "    _Alignas(sizeof(int)) void **al;\n"
                           "    void **deep __attribute__((aligned(sizeof(sizeof(int)))));\n"
                           "    const char **names;\n"
                           "    void *owner;\n"
                           "    int *(*get)(void);\n"
                           "    void (*cb)(void *, void **);\n"
                           "    void *volatile_ptr;\n"
                           "    char *const *chars;\n"
                           "    int k3; void *other;\n"
                           "    void (*cb2)(void *a,\n"
                           "        int b);\n"
                           "} planet_" "t;\n");
    write_file(SRC "/f.c", "static void const *g_c[4];\n"
                           "static const char **g_names;\n"
                           "static pointer_" "list_t collect_" "filter_pointer_list; static pointer_" "list_t g_second;\n");
    write_file(SRC "/f.h", "void const **pico8_" "reg_c(void);\n"
                           "void *volatile *pico8_" "reg_v(void);\n"
                           "const char **pico8_" "names(void);\n");
    assert(run_residue("--check entity-ptrs,registries " TREE) == 1);
    assert(strcmp(read_file(OUT), "entity-ptrs: " SRC "/e.h:6:    void const **q1;\n"
                                  "entity-ptrs: " SRC "/e.h:7:    volatile void **q2;\n"
                                  "entity-ptrs: " SRC "/e.h:8:    void * volatile *q3;\n"
                                  "entity-ptrs: " SRC "/e.h:9:    int k2; void **items;\n"
                                  "entity-ptrs: " SRC "/e.h:10:    pointer_" "list_t *needs; pointer_" "list_t *hidden;\n"
                                  "entity-ptrs: " SRC "/e.h:11:    void **d1; void **d2;\n"
                                  "entity-ptrs: " SRC "/e.h:12:    void **\n"
                                  "entity-ptrs: " SRC "/e.h:14:    pointer_" "list_t\n"
                                  "entity-ptrs: " SRC "/e.h:16:    void **nested __attribute__((aligned(sizeof(int))));\n"
                                  "entity-ptrs: " SRC "/e.h:17:    _Alignas(sizeof(int)) void **al;\n"
                                  "registries: " SRC "/f.c:1:static void const *g_c[4];\n"
                                  "registries: " SRC "/f.h:1:void const **pico8_" "reg_c(void);\n"
                                  "registries: " SRC "/f.h:2:void *volatile *pico8_" "reg_v(void);\n"
                                  "guarded residue (entity-ptrs,registries): 13 line(s)\n") == 0);
}

/* Accolade à la ligne, union, __attribute__ et _Alignas : vus. Une union
 * imbriquée sur plusieurs lignes (f.h:19) reste une limite, comme la struct
 * imbriquée. */
static void test_registries_sees_split_braces_unions_and_attributes(void) {
    setup();
    write_file(SRC "/f.h", "typedef struct reg_s\n"
                           "{\n"
                           "    pointer_" "list_t l;\n"
                           "} reg_s_t;\n"
                           "typedef union {\n"
                           "    pointer_" "list_t l;\n"
                           "    int x;\n"
                           "} reg_u_t;\n"
                           "typedef union { pointer_" "list_t l; } reg_u1_t;\n"
                           "typedef struct __attribute__((packed)) {\n"
                           "    pointer_" "list_t l;\n"
                           "} reg_p_t;\n"
                           "struct __attribute__((aligned(8))) reg_q {\n"
                           "    void **items __attribute__((aligned(8)));\n"
                           "};\n"
                           "typedef struct {\n"
                           "    _Alignas(8) void **items;\n"
                           "    union {\n"
                           "        pointer_" "list_t l;\n"
                           "    } inner;\n"
                           "} reg_a_t;\n");
    write_file(SRC "/f.c", "_Alignas(8) static pointer_" "list_t g_al;\n"
                           "_Alignas(8) static void *g_slots[8];\n"
                           "static union {\n"
                           "    pointer_" "list_t l;\n"
                           "} g_u;\n"
                           "static struct\n"
                           "{\n"
                           "    pointer_" "list_t l;\n"
                           "} g_s;\n"
                           "_Alignas(8) static struct {\n"
                           "    pointer_" "list_t l;\n"
                           "} g_as;\n"
                           "static void reg_f(void) {\n"
                           "    static union {\n"
                           "        pointer_" "list_t l;\n"
                           "    } u;\n"
                           "    static struct\n"
                           "    {\n"
                           "        pointer_" "list_t l;\n"
                           "    } s;\n"
                           "    (void)u;\n"
                           "    (void)s;\n"
                           "}\n");
    assert(run_residue("--check registries " TREE) == 1);
    assert(strcmp(read_file(OUT), "registries: " SRC "/f.c:1:_Alignas(8) static pointer_" "list_t g_al;\n"
                                  "registries: " SRC "/f.c:2:_Alignas(8) static void *g_slots[8];\n"
                                  "registries: " SRC "/f.c:4:    pointer_" "list_t l;\n"
                                  "registries: " SRC "/f.c:8:    pointer_" "list_t l;\n"
                                  "registries: " SRC "/f.c:11:    pointer_" "list_t l;\n"
                                  "registries: " SRC "/f.c:15:        pointer_" "list_t l;\n"
                                  "registries: " SRC "/f.c:19:        pointer_" "list_t l;\n"
                                  "registries: " SRC "/f.h:3:    pointer_" "list_t l;\n"
                                  "registries: " SRC "/f.h:6:    pointer_" "list_t l;\n"
                                  "registries: " SRC "/f.h:9:typedef union { pointer_" "list_t l; } reg_u1_t;\n"
                                  "registries: " SRC "/f.h:11:    pointer_" "list_t l;\n"
                                  "registries: " SRC "/f.h:14:    void **items __attribute__((aligned(8)));\n"
                                  "registries: " SRC "/f.h:17:    _Alignas(8) void **items;\n"
                                  "guarded residue (registries): 13 line(s)\n") == 0);
}

static void test_void_ptr_reads_joined_public_prototypes(void) {
    setup();
    write_file(SRC "/v.h", "particle_t *pico8_" "g__mm(void *" "self);\n"
                           "const void *pico8_" "ha_pz(void);\n"
                           "int pico8_" "two(int a,\n"
                           "    void *b);\n");
    assert(run_residue("--check void-ptr " TREE) == 1);
    assert(strcmp(read_file(OUT), "void-ptr: " SRC "/v.h:1:particle_t *pico8_" "g__mm(void *" "self);\n"
                                  "void-ptr: " SRC "/v.h:2:const void *pico8_" "ha_pz(void);\n"
                                  "void-ptr: " SRC "/v.h:3:int pico8_" "two(int a,\n"
                                  "guarded residue (void-ptr): 3 line(s)\n") == 0);
    setup();
    write_file(SRC "/v.h", "void pico8_" "ei_dispatch(void *" "self, int x);\n"
                           "int pico8_jo_get" "_fold_anchor(const void *" "self);\n"
                           "pointer_list_t collect_" "filter_pointer_list(int a, void *ctx);\n"
                           "void pico8_" "gn(int a, void *r_ctx);\n"
                           "typedef int pico8_" "fn(void *e);\n"
                           "typedef struct {\n    void (*kj)(void *" "self, int x);\n    void *(*new_entity)(void *k);\n} k_t;\n");
    write_file(SRC "/engine/pointer_list.h", "void pointer_" "list_add(pointer_list_t *p, void *e);\n");
    write_file(SRC "/p.c", "void pico8_" "x(void *e);\n");
    write_file(SRC "/i.h", "static inline int f(int x) {\nreturn pico8_" "g((void *)0) + x;\n}\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "header-accessors\t1\t1\n"
                                  "void-ptr\t1\t1\n"
                                  "void-self\t3\t1\n"
                                  "registries\t1\t1\n"
                                  "residue 2: 4 line(s), 1 file(s)\n") == 0);
}

/* Un receveur d'objet du jeu non type : membre, typedef, definition, locale,
 * const autour de l'etoile, src/ comme tests/. Limite nommee : un void *
 * d'un autre nom (e, ctx) n'est pas lu. */
static void test_void_self_sees_every_untyped_receiver(void) {
    setup();
    write_file(SRC "/s.h", "typedef void (*pico8_" "state_fn)(void *" "self, int t);\n"
                           "struct k {\n    int (*has_refresh)(const void *" "self);\n};\n");
    write_file(SRC "/s.c", "static void pico8_" "bg_kj_dispatch(void *" "self) {\n}\n"
                           "void (*draw)(void*" "self) = 0;\n"
                           "int pico8_" "ga_mm(void * const " "self_ptr);\n");
    write_file(SRC "/n.c", "static int step(void *e, void *ctx);\n"
                           "static void f(entity" "_t *self);\n");
    write_file(TST "/t.c", "static void spy(void *" "self, int x) {\n}\n");
    assert(run_residue("--check void-self " TREE) == 1);
    assert(strcmp(read_file(OUT), "void-self: " SRC "/s.c:1:static void pico8_" "bg_kj_dispatch(void *" "self) {\n"
                                  "void-self: " SRC "/s.c:3:void (*draw)(void*" "self) = 0;\n"
                                  "void-self: " SRC "/s.c:4:int pico8_" "ga_mm(void * const " "self_ptr);\n"
                                  "void-self: " SRC "/s.h:1:typedef void (*pico8_" "state_fn)(void *" "self, int t);\n"
                                  "void-self: " SRC "/s.h:3:    int (*has_refresh)(const void *" "self);\n"
                                  "void-self: " TST "/t.c:1:static void spy(void *" "self, int x) {\n"
                                  "guarded residue (void-self): 6 line(s)\n") == 0);
}

static void test_detail_prints_r2_sites_under_void_ptr_and_the_info_line(void) {
    setup();
    write_file(RESIDUE_2_SITES, "pico8_" "f\tsrc/x.c\tupstream(void *" "self)\tproof\n");
    write_file(SRC "/game/game_data.h", "int a;\nint b;\n");
    write_file(SRC "/v.h", "void pico8_" "g(void *e);\n");
    assert(run_residue("--detail " TREE) == 0);
    assert(strcmp(read_file(OUT), "void-ptr\t" SRC "/v.h\t1\n"
                                  "void-ptr-r2\tpico8_" "f\tupstream(void *" "self)\n"
                                  "info\t" SRC "/game/game_data.h\t2 lines\n"
                                  "residue 2: 1 line(s), 1 file(s)\n") == 0);
}

static void test_attainment_list_counts_only_useful_lines(void) {
    setup();
    write_file(ATT, "# c\n\nq1\tx\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "reachability-to-prove\t1\t1\n"
                                  "residue 2: 1 line(s), 1 file(s)\n") == 0);
    assert(run_residue("--check reachability-to-prove " TREE) == 1);
    assert(strstr(read_file(OUT), "reachability-to-prove: " ATT ":3:q1\tx\n") != NULL);
}

static void test_mutual_inclusion_counts_its_include_lines_in_the_footer(void) {
    setup();
    write_file(SRC "/a.h", "#include \"b.h\"\n#include \"a.h\"\n#include \"c.h\"\n");
    write_file(SRC "/a.c", "#include \"b.h\"\n");
    write_file(SRC "/b.c", "#include <stdio.h>\n#include \"a.h\"\n");
    write_file(SRC "/c.h", "#include <a.h>\n");
    write_file(TST "/d.c", "#include \"e.h\"\n");
    write_file(TST "/e.h", "#include \"d.h\"\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "mutual-inclusion\t1\t2\n"
                                  "residue 2: 3 line(s), 3 file(s)\n") == 0);
    assert(run_residue("--detail " TREE) == 0);
    assert(strcmp(read_file(OUT), "mutual-inclusion\ta\tb\n"
                                  "residue 2: 3 line(s), 3 file(s)\n") == 0);
    assert(run_residue("--check mutual-inclusion " TREE) == 1);
    assert(strcmp(read_file(OUT), "mutual-inclusion: " SRC "/a.c:1:#include \"b.h\"\n"
                                  "mutual-inclusion: " SRC "/a.h:1:#include \"b.h\"\n"
                                  "mutual-inclusion: " SRC "/b.c:2:#include \"a.h\"\n"
                                  "guarded residue (mutual-inclusion): 3 line(s)\n") == 0);
    setup();
    write_file(SRC "/a.h", "#include \"b.h\"\n");
    assert(run_residue("--check mutual-inclusion " TREE) == 0);
    assert(strcmp(read_file(OUT), "guarded residue (mutual-inclusion): 0 line(s)\n") == 0);
    setup();
    write_file(SRC "/game/a.h", "#include \"game/b.h\"\n");
    write_file(SRC "/game/b.h", "#include \"game/a.h\"\n");
    assert(run_residue("--check mutual-inclusion " TREE) == 1);
    assert(strcmp(read_file(OUT), "mutual-inclusion: " SRC "/game/a.h:1:#include \"game/b.h\"\n"
                                  "mutual-inclusion: " SRC "/game/b.h:1:#include \"game/a.h\"\n"
                                  "guarded residue (mutual-inclusion): 2 line(s)\n") == 0);
}

/* a et b s'emploient, mais a.c ne recoit b.h que par c.h : inclusion-mutuelle
 * n'y voit rien. d et e emploient u, qui ne les emploie que par s_t, que d.h
 * et e.h declarent tous deux ; f n'emploie g que dans un commentaire et une
 * chaine : ni u-d, ni u-e, ni f-g n'est une paire. */
static void test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment(void) {
    setup();
    write_file(SRC "/a.h", "int a_f(void);\n");
    write_file(SRC "/b.h", "#include \"a.h\"\nint b_g(void);\n");
    write_file(SRC "/c.h", "#include \"b.h\"\n");
    write_file(SRC "/a.c", "#include \"a.h\"\n#include \"c.h\"\nint a_f(void) {\n    return b_g();\n}\n");
    write_file(SRC "/b.c", "#include \"b.h\"\nint b_g(void) {\n    return a_f();\n}\n");
    write_file(SRC "/d.h", "struct s_tag;\ntypedef struct s_tag s_t;\n");
    write_file(SRC "/e.h", "struct s_tag;\ntypedef struct s_tag s_t;\n");
    write_file(SRC "/u.h", "int u_f(void);\n");
    write_file(SRC "/u.c", "int u_g(s_t *s) { return s != 0; }\n");
    write_file(SRC "/d.c", "#include \"u.h\"\nint d = u_f();\n");
    write_file(SRC "/e.c", "#include \"u.h\"\nint e = u_f();\n");
    write_file(SRC "/f.h", "int f_k(void);\n");
    write_file(SRC "/g.h", "int g_h(void);\n");
    write_file(SRC "/f.c", "/* g_h */\nconst char *f_n = \"g_h\";\nint f_k(void) { return 0; }\n");
    write_file(SRC "/g.c", "#include \"f.h\"\nint g_h(void) { return f_k(); }\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "mutual-usage\t1\t2\n"
                                  "residue 2: 2 line(s), 2 file(s)\n") == 0);
    assert(run_residue("--detail " TREE) == 0);
    assert(strcmp(read_file(OUT), "mutual-usage\ta\tb\n"
                                  "residue 2: 2 line(s), 2 file(s)\n") == 0);
    assert(run_residue("--check mutual-inclusion,mutual-usage " TREE) == 1);
    assert(strcmp(read_file(OUT), "mutual-usage: " SRC "/a.c:4:    return b_g();\n"
                                  "mutual-usage: " SRC "/b.c:3:    return a_f();\n"
                                  "guarded residue (mutual-inclusion,mutual-usage): 2 line(s)\n") == 0);
    assert(system("rm " SRC "/a.c") == 0);
    assert(run_residue("--check mutual-usage " TREE) == 0);
    assert(strcmp(read_file(OUT), "guarded residue (mutual-usage): 0 line(s)\n") == 0);
}

/* Une fonction static inline definie dans un en-tete est un nom qu'il
 * declare, comme un prototype. */
static void test_usage_pairs_read_a_static_inline_of_a_header(void) {
    setup();
    write_file(SRC "/a.h", "typedef struct {\n    int n;\n} s_t;\n"
                           "static inline int a_f(const s_t *s) {\n    if (s) {\n        return s->n;\n    }\n"
                           "    return 0;\n}\n");
    write_file(SRC "/b.h", "int b_g(void);\n");
    write_file(SRC "/a.c", "#include \"a.h\"\n#include \"b.h\"\nint a_k(void) {\n    return b_g();\n}\n");
    write_file(SRC "/b.c", "#include \"a.h\"\n#include \"b.h\"\nint b_g(void) {\n    return a_f(0);\n}\n");
    assert(run_residue("--check mutual-usage " TREE) == 1);
    assert(strcmp(read_file(OUT), "mutual-usage: " SRC "/a.c:4:    return b_g();\n"
                                  "mutual-usage: " SRC "/b.c:4:    return a_f(0);\n"
                                  "guarded residue (mutual-usage): 2 line(s)\n") == 0);
}

static void test_detail_sorts_by_category_then_file(void) {
    setup();
    write_file(SRC "/b.c", "q = pico8_" "qh_of(b);\n");
    write_file(SRC "/a.c", "q = pico8_" "qh_of(a);\nq = pico8_" "qh_of(a);\ne->pk_" "is_bool = 0;\n");
    assert(run_residue("--detail " TREE) == 0);
    const char *expected = "parent-link\t" SRC "/a.c\t2\n"
                           "parent-link\t" SRC "/b.c\t1\n"
                           "one-or-false\t" SRC "/a.c\t1\n"
                           "residue 2: 4 line(s), 2 file(s)\n";
    assert(strcmp(read_file(OUT), expected) == 0);
    assert(system("cp " OUT " " DIR "/one.txt") == 0);
    assert(run_residue("--detail " TREE) == 0);
    assert(system("cmp -s " OUT " " DIR "/one.txt") == 0);
}

static void test_check_fails_only_on_a_named_category_present(void) {
    setup();
    write_file(SRC "/r.c", "int a;\ne->fp_" "is_bool = 1;\n");
    assert(run_residue("--check one-or-false " TREE) == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "one-or-false: " SRC "/r.c:2:e->fp_" "is_bool = 1;\n") != NULL);
    assert(strstr(out, "guarded residue (one-or-false): 1 line(s)\n") != NULL);
    assert(run_residue("--check defaults " TREE) == 0);
    assert(strcmp(read_file(OUT), "guarded residue (defaults): 0 line(s)\n") == 0);
    assert(run_residue("--check defaults,one-or-false " TREE) == 1);
    assert(run_residue("--check nope " TREE) == 2);
    assert(strstr(read_file(ERR), "nope") != NULL);
    assert(run_residue("--check '' " TREE) == 2);
    assert(run_residue("--check defaults,,one-or-false " TREE) == 2);
    assert(run_residue("--check") == 2);
}

static void test_measure_on_nothing_is_an_error_not_an_empty_residue(void) {
    setup();
    assert(run_residue(SRC " " DIR "/absent") == 2);
    assert(strstr(read_file(OUT), "residue 2: 0") == NULL);
    assert(system("mkdir -p " DIR "/bare && echo x > " DIR "/bare/notes.txt") == 0);
    assert(run_residue(DIR "/bare") == 2);
    assert(strstr(read_file(OUT), "residue 2: 0") == NULL);
    assert(run_residue("--check defaults " DIR "/bare") == 2);
    assert(run_residue("--bogus " TREE) == 2);
    setup();
    assert(system("rm " ATT) == 0);
    assert(run_residue(TREE) == 2);
    assert(strstr(read_file(OUT), "residue 2: 0") == NULL);
    assert(run_residue("--check defaults " TREE) == 2);
    setup();
    assert(system("rm " RESIDUE_2_SITES) == 0);
    assert(run_residue("--detail " TREE) == 2);
    assert(strstr(read_file(OUT), "residue 2: 0") == NULL);
}

static void test_fields_counts_writes_and_writes_outside_constructors(void) {
    setup();
    write_file(SRC "/e.h", "typedef struct {\n    entity" "_t header;\n} entity_" "root_t;\n"
                           "typedef struct {\n    entity_" "root_t ke;\n    int a;\n    int b;\n"
                           "    int c; /* (x) */\n    void (*f)(int k);\n} pico8_" "jo_t;\n");
    write_file(TST "/fake.h", "typedef struct {\n    entity_" "root_t ke;\n    int z;\n} fake_t;\n");
    write_file(SRC "/e.c", "void planet_init_instance(pico8_" "jo_t *self,\n"
                           "                          int k) {\n"
                           "    self->a = pico8_" "jo_start.a;\n"
                           "    self->b = 1;\n"
                           "}\n"
                           "static void planet_new_year(pico8_" "jo_t *self) {\n"
                           "    self->c += 2;\n"
                           "    if (self->b == 1) {}\n"
                           "}\n");
    write_file(TST "/e.c", "void t(void) { q->a = 1; q->z = 1; }\n");
    assert(run_residue("--fields " TREE) == 0);
    assert(strcmp(read_file(OUT), "pico8_" "jo_t\ta\t0\t0\n"
                                  "pico8_" "jo_t\tb\t1\t0\n"
                                  "pico8_" "jo_t\tc\t1\t1\n"
                                  "fields: 3; no write: 1; no write outside a constructor: 2\n") == 0);
    write_file(SRC "/k.c", "static pico8_" "jo_t k = { .b = 2 };\n");
    assert(run_residue("--fields " TREE) == 0);
    assert(strstr(read_file(OUT), "pico8_" "jo_t\tb\t2\t1\n") != NULL);
}

static void test_accessors_bite_on_every_header_field_name(void) {
    setup();
    write_file(SRC "/w.c", "a = x_get" "_layer(self);\n"
                           "a = x_get" "_pos(self);\n"
                           "a = x_get" "_map_pos(self);\n"
                           "a = x_get" "_hit_box(self);\n"
                           "a = x_get" "_cursor_sprite(self);\n"
                           "a = x_get" "_gpio_signal(self);\n"
                           "a = x_get" "_infraspace(self);\n"
                           "a = x_get" "_fold_anchor(self);\n"
                           "a = x_get" "_is_dead(self);\n"
                           "x_set" "_is_hovered(self, 1);\n");
    assert(run_residue("--check header-accessors " TREE) == 1);
    assert(strstr(read_file(OUT), "guarded residue (header-accessors): 10 line(s)\n") != NULL);
}

int main(void) {
    test_clean_tree_prints_only_the_empty_footer();
    test_one_occurrence_per_category_in_table_order();
    test_other_witnesses_of_each_pattern_bite();
    test_accessors_bite_on_every_header_field_name();
    test_named_false_positives_do_not_bite();
    test_exclusions_edit_the_text_not_the_whole_line();
    test_overlapping_categories_count_each_but_footer_counts_distinct_lines();
    test_entity_reader_finds_embedded_parents_by_closure();
    test_entity_reader_ignores_what_is_not_an_embedded_parent();
    test_pointer_list_members_count_only_on_entity_types();
    test_registries_reads_declaration_points_outside_entity_types();
    test_registries_sees_hidden_forms_and_not_its_named_limits();
    test_pointer_list_entity_reads_entity_members_like_registries();
    test_pointer_list_entity_keeps_one_line_number_in_two_files();
    test_readers_see_qualifiers_split_declarations_and_nested_attributes();
    test_registries_sees_split_braces_unions_and_attributes();
    test_void_ptr_reads_joined_public_prototypes();
    test_void_self_sees_every_untyped_receiver();
    test_detail_prints_r2_sites_under_void_ptr_and_the_info_line();
    test_attainment_list_counts_only_useful_lines();
    test_mutual_inclusion_counts_its_include_lines_in_the_footer();
    test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment();
    test_usage_pairs_read_a_static_inline_of_a_header();
    test_detail_sorts_by_category_then_file();
    test_check_fails_only_on_a_named_category_present();
    test_measure_on_nothing_is_an_error_not_an_empty_residue();
    test_fields_counts_writes_and_writes_outside_constructors();
    assert(system("rm -rf " DIR) == 0);
    printf("test_composition_residue: all tests passed\n");
    return 0;
}
