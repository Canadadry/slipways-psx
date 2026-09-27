/* tools/float_grep.sh, le corps de `make float-grep`, sur un mini-src/
 * fabrique : il se teste par ce qu'il doit empecher, un flottant du coeur
 * que le grep du §6.1 laisserait passer, ou un fichier soustrait en silence. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_float_grep_work"
#define SRC DIR "/src"
#define EXCL DIR "/exclusions.txt"
#define OUT DIR "/out.txt"
#define GREP "sh tools/float_grep.sh cc '-std=c99 -Wall -Wextra -Werror' " EXCL " " SRC

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
 * de system() n'est pas decodable sans <sys/wait.h>, hors C99. */
static int run_grep(const char *sources) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, GREP " %s > " OUT " 2>&1; echo $? > " DIR "/code", sources);
    assert(system(cmd) == 0);
    return atoi(read_file(DIR "/code"));
}

/* Un coeur sans flottant : core.c inclut core.h ; audit.h n'est inclus que
 * sous PICO8_HOST_BUILD, donc hors du coeur, et exclu avec main.c. */
static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " SRC) == 0);
    write_file(SRC "/core.c",
               "#include \"core.h\"\n"
               "#ifdef PICO8_HOST_BUILD\n#include \"audit.h\"\n#endif\n"
               "int core_half(int x) { return x / 2; }\n");
    write_file(SRC "/core.h", "int core_half(int x);\n");
    write_file(SRC "/audit.h", "double audit_value(void);\n");
    write_file(SRC "/main.c", "int main(void) { double d = 1.0; return (int)d; }\n");
    write_file(EXCL, "# commentaire\n\n"
                     SRC "/audit.h | build d'audit\n"
                     SRC "/main.c | point d'entree\n");
}

static void test_core_without_float_passes(void) {
    setup();
    assert(run_grep(SRC "/core.c") == 0);
    assert(strstr(read_file(OUT), "float-grep coeur 2 exclus 2 lignes 0\n") != NULL);
}

static void test_comment_word_in_included_header_is_found(void) {
    setup();
    write_file(SRC "/core.h", "/* no double-free here */\nint core_half(int x);\n");
    assert(run_grep(SRC "/core.c") == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC "/core.h:1:/* no double-free here */\n") != NULL);
    assert(strstr(out, "float-grep coeur 2 exclus 2 lignes 1\n") != NULL);
}

static void test_identifier_suffix_is_not_a_word(void) {
    setup();
    write_file(SRC "/core.h", "int core_half(int x);\nint pico8_fixed_to_double;\n");
    assert(run_grep(SRC "/core.c") == 0);
    assert(strstr(read_file(OUT), "lignes 0\n") != NULL);
}

static void test_lines_are_sorted_by_file_then_line_number(void) {
    setup();
    char body[512] = "#include \"core.h\"\n";
    for (int k = 2; k < 10; k++) {
        strcat(body, "\n");
    }
    strcat(body, "float a;\ndouble b;\n");
    write_file(SRC "/core.c", body);
    write_file(SRC "/core.h", "float f;\n");
    assert(run_grep(SRC "/core.c") == 1);
    const char *out = read_file(OUT);
    const char *nine = strstr(out, SRC "/core.c:10:float a;\n");
    const char *ten = strstr(out, SRC "/core.c:11:double b;\n");
    const char *header = strstr(out, SRC "/core.h:1:float f;\n");
    assert(nine != NULL && ten != NULL && header != NULL);
    assert(nine < ten && ten < header);
    assert(strstr(out, "lignes 3\n") != NULL);
}

static void test_audit_only_header_must_be_excluded(void) {
    setup();
    write_file(EXCL, SRC "/main.c | point d'entree\n");
    assert(run_grep(SRC "/core.c") == 2);
    assert(strstr(read_file(OUT), "ni dans le coeur ni exclu : " SRC "/audit.h\n") != NULL);
}

static void test_core_file_cannot_be_excluded(void) {
    setup();
    write_file(EXCL, SRC "/audit.h | build d'audit\n" SRC "/main.c | point d'entree\n"
                     SRC "/core.h | masque\n");
    assert(run_grep(SRC "/core.c") == 2);
    assert(strstr(read_file(OUT), "dans le coeur et exclu : " SRC "/core.h\n") != NULL);
}

static void test_excluding_a_missing_file_is_an_error(void) {
    setup();
    write_file(EXCL, SRC "/audit.h | build d'audit\n" SRC "/main.c | point d'entree\n"
                     SRC "/gone.c | supprime\n");
    assert(run_grep(SRC "/core.c") == 2);
    assert(strstr(read_file(OUT), "fichier absent : " SRC "/gone.c\n") != NULL);
}

static void test_empty_justification_is_an_error(void) {
    setup();
    write_file(EXCL, SRC "/audit.h |  \n" SRC "/main.c | point d'entree\n");
    assert(run_grep(SRC "/core.c") == 2);
    assert(strstr(read_file(OUT), "justification vide : " SRC "/audit.h\n") != NULL);
    write_file(EXCL, SRC "/audit.h\n" SRC "/main.c | point d'entree\n");
    assert(run_grep(SRC "/core.c") == 2);
}

static void test_duplicate_exclusion_is_an_error(void) {
    setup();
    write_file(EXCL, SRC "/audit.h | a\n" SRC "/main.c | b\n" SRC "/main.c | c\n");
    assert(run_grep(SRC "/core.c") == 2);
    assert(strstr(read_file(OUT), "en double : " SRC "/main.c\n") != NULL);
}

static void test_no_source_is_an_error_not_a_pass(void) {
    setup();
    assert(run_grep("") == 2);
}

static void test_compiler_failure_is_an_error_not_a_pass(void) {
    setup();
    write_file(SRC "/core.c", "#include \"missing.h\"\n");
    assert(run_grep(SRC "/core.c") == 2);
}

static void test_two_runs_give_identical_output(void) {
    setup();
    write_file(SRC "/core.h", "double a;\nfloat b;\n");
    run_grep(SRC "/core.c");
    assert(system("cp " OUT " " DIR "/one.txt") == 0);
    run_grep(SRC "/core.c");
    assert(system("cmp -s " OUT " " DIR "/one.txt") == 0);
}

int main(void) {
    test_core_without_float_passes();
    test_comment_word_in_included_header_is_found();
    test_identifier_suffix_is_not_a_word();
    test_lines_are_sorted_by_file_then_line_number();
    test_audit_only_header_must_be_excluded();
    test_core_file_cannot_be_excluded();
    test_excluding_a_missing_file_is_an_error();
    test_empty_justification_is_an_error();
    test_duplicate_exclusion_is_an_error();
    test_no_source_is_an_error_not_a_pass();
    test_compiler_failure_is_an_error_not_a_pass();
    test_two_runs_give_identical_output();
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_grep: all tests passed\n");
    return 0;
}
