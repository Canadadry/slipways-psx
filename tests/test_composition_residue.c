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
    snprintf(cmd, sizeof cmd, "sh tools/composition_residue.sh %s > " OUT " 2> " ERR "; echo $? > " DIR "/code",
             args);
    assert(system(cmd) == 0);
    return atoi(read_file(DIR "/code"));
}

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " SRC " " TST) == 0);
    write_file(SRC "/clean.c", "int main(void) { return 0; }\n");
    write_file(TST "/clean_test.c", "int helper(int x) { return x + 1; }\n");
}

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
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "lien-parent\t1\t1\n"
                                  "accesseurs-q6\t1\t1\n"
                                  "défauts\t1\t1\n"
                                  "n" "u\t1\t1\n"
                                  "aiguillage-nom\t1\t1\n"
                                  "un-ou-faux\t1\t1\n"
                                  "noyau-inclut\t1\t1\n"
                                  "résidu 2 : 7 ligne(s), 7 fichier(s)\n") == 0);
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
}

int main(void) {
    test_clean_tree_prints_only_the_empty_footer();
    test_one_occurrence_per_category_in_table_order();
    test_other_witnesses_of_each_pattern_bite();
    test_named_false_positives_do_not_bite();
    test_exclusions_edit_the_text_not_the_whole_line();
    test_overlapping_categories_count_each_but_footer_counts_distinct_lines();
    test_detail_sorts_by_category_then_file();
    test_check_fails_only_on_a_named_category_present();
    test_measure_on_nothing_is_an_error_not_an_empty_residue();
    assert(system("rm -rf " DIR) == 0);
    printf("test_composition_residue: all tests passed\n");
    return 0;
}
