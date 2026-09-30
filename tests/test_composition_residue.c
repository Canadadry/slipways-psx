/* tools/composition_residue.sh, le corps de `make composition-residue`, sur un
 * mini-arbre fabrique.
 * Le script lit tests/ : chaque jeton de fixture est coupe en deux litteraux
 * ("get" "_ra", "n" "u"), sinon ce fichier entrerait dans le residu qu'il
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
#define ATT DIR "/atteinte.txt"
#define R2 DIR "/r2.txt"
#define EMPTY_FOOTER "résidu 2 : 0 ligne(s), 0 fichier(s)\n"

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
             "COMPOSITION_ATTEINTE=" ATT " COMPOSITION_R2=" R2
             " sh tools/composition_residue.sh %s > " OUT " 2> " ERR "; echo $? > " DIR "/code",
             args);
    assert(system(cmd) == 0);
    return atoi(read_file(DIR "/code"));
}

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " SRC " " TST) == 0);
    write_file(SRC "/clean.c", "int main(void) { return 0; }\n");
    write_file(TST "/clean_test.c", "int helper(int x) { return x + 1; }\n");
    write_file(ATT, "");
    write_file(R2, "");
}

/* Deux types d'entite : ke (en-tete en premier membre, pas une parente) et
 * jo (ke en premier membre, une parente). */
#define KE_JO "typedef struct {\n    pico8_struct_" "header_t header;\n} pico8_" "ke_t;\n" \
              "typedef struct {\n    pico8_" "ke_t ke;\n    int a;\n} pico8_" "jo_t;\n"

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
    write_file(SRC "/c2.c", "int r = self->get" "_ra(self);\n");
    write_file(SRC "/c3.c", "self->rr = pico8_" "nl_start.rr;\n");
    write_file(SRC "/c4.c", "return self->n" "u;\n");
    write_file(TST "/c5.c", "hp_" "pose(self, p);\n");
    write_file(TST "/c6.c", "e->pk_" "is_bool = 1;\n");
    write_file(SRC "/pico8_struct.h", "#include \"pico8_" "game_data.h\"\n");
    write_file(SRC "/e.h", "typedef struct {\n    pico8_struct_" "header_t header;\n    pico8_" "ptrs_t *oe;\n} pico8_"
                           "jo_t;\n");
    write_file(SRC "/p.h", "typedef struct {\n    pico8_" "jo_t jo;\n} pico8_" "qh_t;\n");
    write_file(SRC "/v.h", "pico8_qy_t *pico8_" "g__mm(void *self);\n");
    write_file(ATT, "q1\tx\n");
    write_file(SRC "/a.h", "#include \"b.h\"\n");
    write_file(SRC "/b.h", "#include \"a.h\"\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "parente\t1\t1\n"
                                  "lien-parent\t1\t1\n"
                                  "accesseurs-q6\t1\t1\n"
                                  "défauts\t1\t1\n"
                                  "n" "u\t1\t1\n"
                                  "aiguillage-nom\t1\t1\n"
                                  "un-ou-faux\t1\t1\n"
                                  "void-ptr\t1\t1\n"
                                  "ptrs-entité\t1\t1\n"
                                  "noyau-inclut\t1\t1\n"
                                  "atteinte-à-prouver\t1\t1\n"
                                  "inclusion-mutuelle\t1\t2\n"
                                  "résidu 2 : 13 ligne(s), 13 fichier(s)\n") == 0);
}

static void test_other_witnesses_of_each_pattern_bite(void) {
    setup();
    write_file(SRC "/w.c", "static k_t k = { .pa" "rent = x };\n"
                           "q = (*pa" "rent)(self);\n"
                           "q = self->pa" "rent;\n"
                           "q = emitter" "_of(self);\n"
                           "q = g__probe" "_of(self);\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "lien-parent\t5\t1\n"
                                  "résidu 2 : 5 ligne(s), 1 fichier(s)\n") == 0);
    setup();
    write_file(SRC "/w.c", "int a = pico8_jo_get" "_fx(self);\n"
                           "int b = X_get" "_p_(self);\n"
                           "pico8_jo_set" "_m_(self, v);\n"
                           "pico8_" "nl_start_t s;\n"
                           "pico8_" "jo_kw_start;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "accesseurs-q6\t3\t1\n"
                                  "défauts\t2\t1\n"
                                  "résidu 2 : 5 ligne(s), 1 fichier(s)\n") == 0);
    setup();
    write_file(SRC "/w.c", "extern const pico8_str_seq_t pico8_nl_n" "u_tags;\n"
                           "int k = x_n" "u_items;\n"
                           "if (pico8_ho_" "gp_has(self->g" "p, name)) {}\n"
                           "q = pico8_hp_" "mr_names;\n"
                           "q = self.m" "r;\n"
                           "hp_" "ho(self);\n"
                           "self->probe->fp_" "is_bool = 1;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "n" "u\t2\t1\n"
                                  "aiguillage-nom\t4\t1\n"
                                  "un-ou-faux\t1\t1\n"
                                  "résidu 2 : 7 ligne(s), 1 fichier(s)\n") == 0);
    setup();
    write_file(SRC "/pico8_ke.h", "#include \"pico8_" "map_tables.h\"\n");
    write_file(SRC "/pico8_entity.h", "#include \"pico8_" "data_init.h\"\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "noyau-inclut\t2\t2\n"
                                  "résidu 2 : 2 ligne(s), 2 fichier(s)\n") == 0);
}

static void test_named_false_positives_do_not_bite(void) {
    assert_alone_is_empty(SRC "/f.c", "e = pico8_" "state_of(me);\n");
    assert_alone_is_empty(SRC "/f.c", "/* the parent struct */\n");
    assert_alone_is_empty(SRC "/f.c", "int budget_p;\n");
    assert_alone_is_empty(SRC "/f.c", "int get_pa;\n");
    assert_alone_is_empty(SRC "/f.c", "int get_p_x;\n");
    assert_alone_is_empty(SRC "/f.c", "int run_start;\n");
    assert_alone_is_empty(SRC "/f.c", "int number;\n");
    assert_alone_is_empty(SRC "/f.c", "pico8_pi_number(e, 1);\n");
    assert_alone_is_empty(TST "/f.c", "\"pico8_jo_" "default(\\\"n" "u\\\", &k);\\n\"\n");
    assert_alone_is_empty(SRC "/f.c", "int mr = 0;\n");
    assert_alone_is_empty(SRC "/f.c", "self->qe_is_bool = 0;\n");
    assert_alone_is_empty(SRC "/f.c", "self->bg_is_bool = 0;\n");
    assert_alone_is_empty(SRC "/f.c", "k.h_is_false = 1;\n");
    assert_alone_is_empty(SRC "/pico8_struct.h", "#include \"pico8_vec.h\"\n");
    assert_alone_is_empty(SRC "/pico8_planet.h", "#include \"pico8_" "game_data.h\"\n");
}

static void test_exclusions_edit_the_text_not_the_whole_line(void) {
    setup();
    write_file(SRC "/x.c", "if (pico8_" "state_of(me) && pico8_" "qh_of(pt)) {}\n");
    write_file(TST "/x.c", "\"k(\\\"n" "u\\\")\" ; q = self->n" "u;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "lien-parent\t1\t1\n"
                                  "n" "u\t1\t1\n"
                                  "résidu 2 : 2 ligne(s), 2 fichier(s)\n") == 0);
}

static void test_overlapping_categories_count_each_but_footer_counts_distinct_lines(void) {
    setup();
    write_file(SRC "/o.h", "const pico8_str_seq_t *pico8_jo_get" "_n" "u(const void *self);\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "accesseurs-q6\t1\t1\n"
                                  "n" "u\t1\t1\n"
                                  "résidu 2 : 1 ligne(s), 1 fichier(s)\n") == 0);
}

static void test_entity_reader_finds_embedded_parents_by_closure(void) {
    setup();
    write_file(SRC "/c.h", "typedef struct {\n    pico8_" "nl_t nl;\n} pico8_" "os_t;\n");
    write_file(SRC "/d.h", "typedef struct {\n    pico8_" "qh_t qh;\n} pico8_" "nl_t;\n");
    write_file(SRC "/e.h", KE_JO "typedef struct {\n    pico8_" "jo_t jo;\n} pico8_" "qh_t;\n"
                              "struct pico8_" "mh {\n    /* c */\n\n    pico8_" "ke_t ke;\n};\n"
                              "typedef struct pico8_" "mh pico8_" "mh_t;\n"
                              "struct pico8_" "x {\n    pico8_" "mh_t mh;\n};\n");
    assert(run_residue("--check parente " TREE) == 1);
    assert(strcmp(read_file(OUT), "parente: " SRC "/c.h:2:    pico8_" "nl_t nl;\n"
                                  "parente: " SRC "/d.h:2:    pico8_" "qh_t qh;\n"
                                  "parente: " SRC "/e.h:5:    pico8_" "ke_t ke;\n"
                                  "parente: " SRC "/e.h:9:    pico8_" "jo_t jo;\n"
                                  "parente: " SRC "/e.h:14:    pico8_" "ke_t ke;\n"
                                  "parente: " SRC "/e.h:18:    pico8_" "mh_t mh;\n"
                                  "résidu gardé (parente) : 6 ligne(s)\n") == 0);
}

static void test_entity_reader_ignores_what_is_not_an_embedded_parent(void) {
    setup();
    write_file(SRC "/e.h", KE_JO);
    write_file(SRC "/f.c", "typedef struct {\n    pico8_" "jo_t *jo;\n} ptr_t;\n"
                           "typedef struct {\n    int a;\n    pico8_" "jo_t jo;\n} second_t;\n"
                           "int f(void) {\n    pico8_" "jo_t jo;\n    return 0;\n}\n"
                           "typedef struct {\n    pico8_fixed_t x;\n} fixed_first_t;\n"
                           "void g(void) {\nstruct local {\n    pico8_" "jo_t jo;\n} l;\n}\n"
                           "typedef struct {\n    pico8_" "jo_t jo;\n} after_t;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "parente\t2\t2\n"
                                  "résidu 2 : 2 ligne(s), 2 fichier(s)\n") == 0);
}

static void test_ptrs_members_count_only_on_entity_types(void) {
    setup();
    write_file(SRC "/e.h", "typedef struct {\n    pico8_struct_" "header_t header;\n} pico8_" "ke_t;\n"
                           "typedef struct {\n    pico8_" "ke_t ke;\n    pico8_" "ptrs_t *oe;\n    pico8_"
                           "ptrs_t *jy;\n} pico8_" "jo_t;\n"
                           "typedef struct {\n    int a;\n    pico8_" "ptrs_t *qy;\n} other_t;\n"
                           "void f(void) {\n    pico8_" "ptrs_t list;\n}\n");
    assert(run_residue("--check ptrs-entité " TREE) == 1);
    assert(strcmp(read_file(OUT), "ptrs-entité: " SRC "/e.h:6:    pico8_" "ptrs_t *oe;\n"
                                  "résidu gardé (ptrs-entité) : 1 ligne(s)\n") == 0);
}

static void test_registres_reads_declaration_points_outside_entity_types(void) {
    setup();
    write_file(SRC "/e.h", "typedef struct {\n    pico8_struct_" "header_t header;\n    pico8_"
                           "ptrs_t *oe;\n} pico8_" "ke_t;\n"
                           "typedef struct {\n    pico8_" "ptrs_t layers[16];\n    pico8_" "ptrs_t *qy;\n} pico8_"
                           "emitter_t;\n"
                           "typedef struct {\n    void *(*new_entity)(int k);\n    void (*kj)(void *self);\n} k_t;\n"
                           "const pico8_" "ptrs_t *pico8_" "hv(void);\n"
                           "pico8_lq_t *pico8_" "jo_ex(const pico8_" "ptrs_t *is,\n    int o);\n"
                           "typedef int (*pico8_" "fn)(const pico8_" "ptrs_t *p);\n");
    write_file(SRC "/r.c", "static pico8_" "ptrs_t g_hv;\n"
                           "static pico8_" "ptrs_t *g_list = 0;\n"
                           "pico8_" "ptrs_t pico8_" "jp_ptrs(const pico8_" "ptrs_t *qk) {\n    pico8_"
                           "ptrs_t f;\n    return f;\n}\n");
    write_file(SRC "/pico8_ptrs.h", "typedef struct {\n    const pico8_" "ptrs_t *ptrs;\n} pico8_" "ptrs_iter_t;\n"
                                    "void pico8_" "ptrs_add(pico8_" "ptrs_t *p, void *e);\n");
    write_file(TST "/t.c", "static pico8_" "ptrs_t g_t;\n");
    assert(run_residue("--check registres " TREE) == 1);
    assert(strcmp(read_file(OUT), "registres: " SRC "/e.h:6:    pico8_" "ptrs_t layers[16];\n"
                                  "registres: " SRC "/e.h:10:    void *(*new_entity)(int k);\n"
                                  "registres: " SRC "/e.h:13:const pico8_" "ptrs_t *pico8_" "hv(void);\n"
                                  "registres: " SRC "/r.c:1:static pico8_" "ptrs_t g_hv;\n"
                                  "registres: " SRC "/r.c:2:static pico8_" "ptrs_t *g_list = 0;\n"
                                  "résidu gardé (registres) : 5 ligne(s)\n") == 0);
}

/* Formes d'un registre que le lecteur voit (F) et limites qu'il ne voit pas
 * (L), chacune a sa ligne : une limite qui se met a compter change l'attendu. */
static void test_registres_sees_hidden_forms_and_not_its_named_limits(void) {
    setup();
    write_file(SRC "/f.h", "typedef pico8_" "ptrs_t reg_t;\n"
                           "typedef reg_t reg2_t;\n"
                           "typedef struct {\n"
                           "    pico8_" "ptrs_t *a, *b;\n"
                           "    pico8_" "ptrs_t **pp;\n"
                           "    const pico8_" "ptrs_t *const x;\n"
                           "    void **items;\n"
                           "    pico8_" "ptrs_t (*get)(void);\n"
                           "    void *slots[8];\n"
                           "    void *owner;\n"
                           "    struct {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } inner;\n"
                           "} holder_t;\n"
                           "static inline const pico8_" "ptrs_t *reg_get(void) { return 0; }\n"
                           "void **pico8_" "reg_items(void);\n"
                           "extern pico8_" "ptrs_t g_reg;\n"
                           "typedef struct { pico8_" "ptrs_t l; } one_t;\n");
    write_file(SRC "/f.c", "static void reg_f(void) {\n"
                           "    static pico8_" "ptrs_t reg;\n"
                           "    pico8_" "ptrs_t l;\n"
                           "    static struct {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } g;\n"
                           "    (void)reg;\n"
                           "}\n"
                           "static reg_t g_alias;\n"
                           "static reg2_t g_alias2;\n"
                           "static struct {\n"
                           "    pico8_" "ptrs_t list;\n"
                           "} g_anon;\n"
                           "static struct reg_s {\n"
                           "    pico8_" "ptrs_t list;\n"
                           "} g_named;\n"
                           "static struct { pico8_" "ptrs_t l; } g_one;\n"
                           "static void *g_void[64];\n"
                           "static pico8_" "ptrs_t g_a, g_b;\n"
                           "pico8_" "ptrs_t g_pub;\n"
                           "typedef void *handle_t;\n"
                           "static handle_t g_h[64];\n"
                           "#define REG(n) pico8_" "ptrs_t n\n"
                           "static REG(g_m);\n"
                           "int reg_n(const pico8_" "ptrs_t *p) {\n"
                           "    return p->count;\n"
                           "}\n");
    write_file(TST "/t.c", "static void *g_t[4];\n");
    assert(run_residue("--check registres " TREE) == 1);
    assert(strcmp(read_file(OUT), "registres: " SRC "/f.c:2:    static pico8_" "ptrs_t reg;\n"
                                  "registres: " SRC "/f.c:5:        pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:9:static reg_t g_alias;\n"
                                  "registres: " SRC "/f.c:12:    pico8_" "ptrs_t list;\n"
                                  "registres: " SRC "/f.c:15:    pico8_" "ptrs_t list;\n"
                                  "registres: " SRC "/f.c:17:static struct { pico8_" "ptrs_t l; } g_one;\n"
                                  "registres: " SRC "/f.c:18:static void *g_void[64];\n"
                                  "registres: " SRC "/f.c:19:static pico8_" "ptrs_t g_a, g_b;\n"
                                  "registres: " SRC "/f.c:20:pico8_" "ptrs_t g_pub;\n"
                                  "registres: " SRC "/f.h:4:    pico8_" "ptrs_t *a, *b;\n"
                                  "registres: " SRC "/f.h:5:    pico8_" "ptrs_t **pp;\n"
                                  "registres: " SRC "/f.h:6:    const pico8_" "ptrs_t *const x;\n"
                                  "registres: " SRC "/f.h:7:    void **items;\n"
                                  "registres: " SRC "/f.h:8:    pico8_" "ptrs_t (*get)(void);\n"
                                  "registres: " SRC "/f.h:9:    void *slots[8];\n"
                                  "registres: " SRC "/f.h:15:static inline const pico8_" "ptrs_t *reg_get(void) { return 0; }\n"
                                  "registres: " SRC "/f.h:16:void **pico8_" "reg_items(void);\n"
                                  "registres: " SRC "/f.h:17:extern pico8_" "ptrs_t g_reg;\n"
                                  "registres: " SRC "/f.h:18:typedef struct { pico8_" "ptrs_t l; } one_t;\n"
                                  "résidu gardé (registres) : 19 ligne(s)\n") == 0);
}

/* ptrs-entité lit les membres d'un type d'entité comme registres lit ceux des
 * autres structs : aucun conteneur non typé n'échappe aux deux. */
static void test_ptrs_entite_reads_entity_members_like_registres(void) {
    setup();
    write_file(SRC "/a.h", "typedef pico8_" "ptrs_t reg17_t;\n");
    write_file(SRC "/e.h", "typedef struct {\n    pico8_struct_" "header_t header;\n} pico8_" "ke_t;\n"
                           "typedef struct {\n"
                           "    pico8_" "ke_t ke;\n"
                           "    void **items;\n"
                           "    void *slots[8];\n"
                           "    reg17_t l;\n"
                           "    pico8_" "ptrs_t arr[4];\n"
                           "    pico8_" "ptrs_t **pp;\n"
                           "    pico8_" "ptrs_t *a, *b;\n"
                           "    const pico8_" "ptrs_t *const x;\n"
                           "    void *(*get)(void);\n"
                           "    _Alignas(8) void **al;\n"
                           "    void *owner;\n"
                           "    pico8_" "ptrs_t *jy;\n"
                           "} pico8_" "jo_t;\n"
                           "typedef struct pico8_" "nl_s\n"
                           "{\n"
                           "    pico8_" "jo_t jo;\n"
                           "    void **more;\n"
                           "} pico8_" "nl_t;\n");
    assert(run_residue("--check registres " TREE) == 0);
    assert(run_residue("--check ptrs-entité " TREE) == 1);
    assert(strcmp(read_file(OUT), "ptrs-entité: " SRC "/e.h:6:    void **items;\n"
                                  "ptrs-entité: " SRC "/e.h:7:    void *slots[8];\n"
                                  "ptrs-entité: " SRC "/e.h:8:    reg17_t l;\n"
                                  "ptrs-entité: " SRC "/e.h:9:    pico8_" "ptrs_t arr[4];\n"
                                  "ptrs-entité: " SRC "/e.h:10:    pico8_" "ptrs_t **pp;\n"
                                  "ptrs-entité: " SRC "/e.h:11:    pico8_" "ptrs_t *a, *b;\n"
                                  "ptrs-entité: " SRC "/e.h:12:    const pico8_" "ptrs_t *const x;\n"
                                  "ptrs-entité: " SRC "/e.h:13:    void *(*get)(void);\n"
                                  "ptrs-entité: " SRC "/e.h:14:    _Alignas(8) void **al;\n"
                                  "ptrs-entité: " SRC "/e.h:21:    void **more;\n"
                                  "résidu gardé (ptrs-entité) : 10 ligne(s)\n") == 0);
}

/* Accolade à la ligne, union, __attribute__ et _Alignas : vus. Une union
 * imbriquée sur plusieurs lignes (f.h:19) reste une limite, comme la struct
 * imbriquée. */
static void test_registres_sees_split_braces_unions_and_attributes(void) {
    setup();
    write_file(SRC "/f.h", "typedef struct reg_s\n"
                           "{\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} reg_s_t;\n"
                           "typedef union {\n"
                           "    pico8_" "ptrs_t l;\n"
                           "    int x;\n"
                           "} reg_u_t;\n"
                           "typedef union { pico8_" "ptrs_t l; } reg_u1_t;\n"
                           "typedef struct __attribute__((packed)) {\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} reg_p_t;\n"
                           "struct __attribute__((aligned(8))) reg_q {\n"
                           "    void **items __attribute__((aligned(8)));\n"
                           "};\n"
                           "typedef struct {\n"
                           "    _Alignas(8) void **items;\n"
                           "    union {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } inner;\n"
                           "} reg_a_t;\n");
    write_file(SRC "/f.c", "_Alignas(8) static pico8_" "ptrs_t g_al;\n"
                           "_Alignas(8) static void *g_slots[8];\n"
                           "static union {\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} g_u;\n"
                           "static struct\n"
                           "{\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} g_s;\n"
                           "_Alignas(8) static struct {\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} g_as;\n"
                           "static void reg_f(void) {\n"
                           "    static union {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } u;\n"
                           "    static struct\n"
                           "    {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } s;\n"
                           "    (void)u;\n"
                           "    (void)s;\n"
                           "}\n");
    assert(run_residue("--check registres " TREE) == 1);
    assert(strcmp(read_file(OUT), "registres: " SRC "/f.c:1:_Alignas(8) static pico8_" "ptrs_t g_al;\n"
                                  "registres: " SRC "/f.c:2:_Alignas(8) static void *g_slots[8];\n"
                                  "registres: " SRC "/f.c:4:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:8:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:11:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:15:        pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:19:        pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.h:3:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.h:6:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.h:9:typedef union { pico8_" "ptrs_t l; } reg_u1_t;\n"
                                  "registres: " SRC "/f.h:11:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.h:14:    void **items __attribute__((aligned(8)));\n"
                                  "registres: " SRC "/f.h:17:    _Alignas(8) void **items;\n"
                                  "résidu gardé (registres) : 13 ligne(s)\n") == 0);
}

static void test_void_ptr_reads_joined_public_prototypes(void) {
    setup();
    write_file(SRC "/v.h", "pico8_qy_t *pico8_" "g__mm(void *self);\n"
                           "const void *pico8_" "ha_pz(void);\n"
                           "int pico8_" "two(int a,\n"
                           "    void *b);\n");
    assert(run_residue("--check void-ptr " TREE) == 1);
    assert(strcmp(read_file(OUT), "void-ptr: " SRC "/v.h:1:pico8_qy_t *pico8_" "g__mm(void *self);\n"
                                  "void-ptr: " SRC "/v.h:2:const void *pico8_" "ha_pz(void);\n"
                                  "void-ptr: " SRC "/v.h:3:int pico8_" "two(int a,\n"
                                  "résidu gardé (void-ptr) : 3 ligne(s)\n") == 0);
    setup();
    write_file(SRC "/v.h", "void pico8_" "ei_dispatch(void *self, int x);\n"
                           "int pico8_jo_get" "_fx(const void *self);\n"
                           "pico8_ptrs_t pico8_" "jp_ptrs(int a, void *ctx);\n"
                           "void pico8_" "gn(int a, void *r_ctx);\n"
                           "typedef int pico8_" "fn(void *e);\n"
                           "typedef struct {\n    void (*kj)(void *self, int x);\n    void *(*new_entity)(void *k);\n} k_t;\n");
    write_file(SRC "/pico8_ptrs.h", "void pico8_" "ptrs_add(pico8_ptrs_t *p, void *e);\n");
    write_file(SRC "/p.c", "void pico8_" "x(void *e);\n");
    write_file(SRC "/i.h", "static inline int f(int x) {\nreturn pico8_" "g((void *)0) + x;\n}\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "accesseurs-q6\t1\t1\n"
                                  "registres\t1\t1\n"
                                  "résidu 2 : 2 ligne(s), 1 fichier(s)\n") == 0);
}

static void test_detail_prints_r2_sites_under_void_ptr_and_the_info_line(void) {
    setup();
    write_file(R2, "pico8_" "f\tsrc/x.c\tamont(void *self)\tpreuve\n");
    write_file(SRC "/pico8_game_data.h", "int a;\nint b;\n");
    write_file(SRC "/v.h", "void pico8_" "g(void *e);\n");
    assert(run_residue("--detail " TREE) == 0);
    assert(strcmp(read_file(OUT), "void-ptr\t" SRC "/v.h\t1\n"
                                  "void-ptr-r2\tpico8_" "f\tamont(void *self)\n"
                                  "info\t" SRC "/pico8_game_data.h\t2 lignes\n"
                                  "résidu 2 : 1 ligne(s), 1 fichier(s)\n") == 0);
}

static void test_attainment_list_counts_only_useful_lines(void) {
    setup();
    write_file(ATT, "# c\n\nq1\tx\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "atteinte-à-prouver\t1\t1\n"
                                  "résidu 2 : 1 ligne(s), 1 fichier(s)\n") == 0);
    assert(run_residue("--check atteinte-à-prouver " TREE) == 1);
    assert(strstr(read_file(OUT), "atteinte-à-prouver: " ATT ":3:q1\tx\n") != NULL);
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
    assert(strcmp(read_file(OUT), "inclusion-mutuelle\t1\t2\n"
                                  "résidu 2 : 3 ligne(s), 3 fichier(s)\n") == 0);
    assert(run_residue("--detail " TREE) == 0);
    assert(strcmp(read_file(OUT), "inclusion-mutuelle\ta\tb\n"
                                  "résidu 2 : 3 ligne(s), 3 fichier(s)\n") == 0);
    assert(run_residue("--check inclusion-mutuelle " TREE) == 1);
    assert(strcmp(read_file(OUT), "inclusion-mutuelle: " SRC "/a.c:1:#include \"b.h\"\n"
                                  "inclusion-mutuelle: " SRC "/a.h:1:#include \"b.h\"\n"
                                  "inclusion-mutuelle: " SRC "/b.c:2:#include \"a.h\"\n"
                                  "résidu gardé (inclusion-mutuelle) : 3 ligne(s)\n") == 0);
    setup();
    write_file(SRC "/a.h", "#include \"b.h\"\n");
    assert(run_residue("--check inclusion-mutuelle " TREE) == 0);
    assert(strcmp(read_file(OUT), "résidu gardé (inclusion-mutuelle) : 0 ligne(s)\n") == 0);
}

static void test_detail_sorts_by_category_then_file(void) {
    setup();
    write_file(SRC "/b.c", "q = pico8_" "qh_of(b);\n");
    write_file(SRC "/a.c", "q = pico8_" "qh_of(a);\nq = pico8_" "qh_of(a);\ne->pk_" "is_bool = 0;\n");
    assert(run_residue("--detail " TREE) == 0);
    const char *expected = "lien-parent\t" SRC "/a.c\t2\n"
                           "lien-parent\t" SRC "/b.c\t1\n"
                           "un-ou-faux\t" SRC "/a.c\t1\n"
                           "résidu 2 : 4 ligne(s), 2 fichier(s)\n";
    assert(strcmp(read_file(OUT), expected) == 0);
    assert(system("cp " OUT " " DIR "/one.txt") == 0);
    assert(run_residue("--detail " TREE) == 0);
    assert(system("cmp -s " OUT " " DIR "/one.txt") == 0);
}

static void test_check_fails_only_on_a_named_category_present(void) {
    setup();
    write_file(SRC "/r.c", "int a;\ne->fp_" "is_bool = 1;\n");
    assert(run_residue("--check un-ou-faux " TREE) == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "un-ou-faux: " SRC "/r.c:2:e->fp_" "is_bool = 1;\n") != NULL);
    assert(strstr(out, "résidu gardé (un-ou-faux) : 1 ligne(s)\n") != NULL);
    assert(run_residue("--check défauts " TREE) == 0);
    assert(strcmp(read_file(OUT), "résidu gardé (défauts) : 0 ligne(s)\n") == 0);
    assert(run_residue("--check défauts,un-ou-faux " TREE) == 1);
    assert(run_residue("--check nope " TREE) == 2);
    assert(strstr(read_file(ERR), "nope") != NULL);
    assert(run_residue("--check '' " TREE) == 2);
    assert(run_residue("--check défauts,,un-ou-faux " TREE) == 2);
    assert(run_residue("--check") == 2);
}

static void test_measure_on_nothing_is_an_error_not_an_empty_residue(void) {
    setup();
    assert(run_residue(SRC " " DIR "/absent") == 2);
    assert(strstr(read_file(OUT), "résidu 2 : 0") == NULL);
    assert(system("mkdir -p " DIR "/bare && echo x > " DIR "/bare/notes.txt") == 0);
    assert(run_residue(DIR "/bare") == 2);
    assert(strstr(read_file(OUT), "résidu 2 : 0") == NULL);
    assert(run_residue("--check défauts " DIR "/bare") == 2);
    assert(run_residue("--bogus " TREE) == 2);
    setup();
    assert(system("rm " ATT) == 0);
    assert(run_residue(TREE) == 2);
    assert(strstr(read_file(OUT), "résidu 2 : 0") == NULL);
    assert(run_residue("--check défauts " TREE) == 2);
    setup();
    assert(system("rm " R2) == 0);
    assert(run_residue("--detail " TREE) == 2);
    assert(strstr(read_file(OUT), "résidu 2 : 0") == NULL);
}

static void test_champs_counts_writes_and_writes_outside_constructors(void) {
    setup();
    write_file(SRC "/e.h", "typedef struct {\n    pico8_struct_" "header_t header;\n} pico8_" "ke_t;\n"
                           "typedef struct {\n    pico8_" "ke_t ke;\n    int a;\n    int b;\n"
                           "    int c; /* (x) */\n    void (*f)(int k);\n} pico8_" "jo_t;\n");
    write_file(TST "/fake.h", "typedef struct {\n    pico8_" "ke_t ke;\n    int z;\n} fake_t;\n");
    write_file(SRC "/e.c", "void pico8_jo_struct_init(pico8_" "jo_t *self,\n"
                           "                         int k) {\n"
                           "    self->a = pico8_" "jo_start.a;\n"
                           "    self->b = 1;\n"
                           "}\n"
                           "static void pico8_jo_hk(pico8_" "jo_t *self) {\n"
                           "    self->c += 2;\n"
                           "    if (self->b == 1) {}\n"
                           "}\n");
    write_file(TST "/e.c", "void t(void) { q->a = 1; q->z = 1; }\n");
    assert(run_residue("--champs " TREE) == 0);
    assert(strcmp(read_file(OUT), "pico8_" "jo_t\ta\t0\t0\n"
                                  "pico8_" "jo_t\tb\t1\t0\n"
                                  "pico8_" "jo_t\tc\t1\t1\n"
                                  "champs : 3 ; 0 écriture : 1 ; 0 hors constructeur : 2\n") == 0);
    write_file(SRC "/k.c", "static pico8_" "jo_t k = { .b = 2 };\n");
    assert(run_residue("--champs " TREE) == 0);
    assert(strstr(read_file(OUT), "pico8_" "jo_t\tb\t2\t1\n") != NULL);
}

int main(void) {
    test_clean_tree_prints_only_the_empty_footer();
    test_one_occurrence_per_category_in_table_order();
    test_other_witnesses_of_each_pattern_bite();
    test_named_false_positives_do_not_bite();
    test_exclusions_edit_the_text_not_the_whole_line();
    test_overlapping_categories_count_each_but_footer_counts_distinct_lines();
    test_entity_reader_finds_embedded_parents_by_closure();
    test_entity_reader_ignores_what_is_not_an_embedded_parent();
    test_ptrs_members_count_only_on_entity_types();
    test_registres_reads_declaration_points_outside_entity_types();
    test_registres_sees_hidden_forms_and_not_its_named_limits();
    test_ptrs_entite_reads_entity_members_like_registres();
    test_registres_sees_split_braces_unions_and_attributes();
    test_void_ptr_reads_joined_public_prototypes();
    test_detail_prints_r2_sites_under_void_ptr_and_the_info_line();
    test_attainment_list_counts_only_useful_lines();
    test_mutual_inclusion_counts_its_include_lines_in_the_footer();
    test_detail_sorts_by_category_then_file();
    test_check_fails_only_on_a_named_category_present();
    test_measure_on_nothing_is_an_error_not_an_empty_residue();
    test_champs_counts_writes_and_writes_outside_constructors();
    assert(system("rm -rf " DIR) == 0);
    printf("test_composition_residue: all tests passed\n");
    return 0;
}
