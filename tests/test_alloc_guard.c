/* tools/alloc_guard.sh, le corps de `make alloc-guard`, sur des objets
 * fabriques : il se teste par ce qu'il doit empecher, une allocation libc
 * qui reviendrait dans le coeur sans changer la liste d'exceptions. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_alloc_guard_work"
#define OBJ DIR "/obj"
#define LIST DIR "/exceptions.txt"
#define OUT DIR "/out.txt"
#define ALLOCS OBJ "/src/pico8_allocs.o"
#define CLEAN OBJ "/src/pico8_clean.o"
#define TRANSIENT_USER OBJ "/src/pico8_transient_user.o"
#define GUARD "sh tools/alloc_guard.sh " LIST " " OBJ

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
static int run_guard_with(const char *env, const char *objects) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "%s " GUARD " %s > " OUT " 2>&1; echo $? > " DIR "/code", env, objects);
    assert(system(cmd) == 0);
    return atoi(read_file(DIR "/code"));
}

static int run_guard(const char *objects) {
    return run_guard_with("", objects);
}

/* pico8_allocs.c : 3 appels (malloc, realloc, free), plus un commentaire et
 * un identifiant qui contiennent « malloc » sans en etre un appel. */
static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR "/src " OBJ "/src") == 0);
    write_file(DIR "/src/pico8_allocs.c",
               "#include <stdlib.h>\n"
               "/* malloc free calloc */\n"
               "int pico8_malloc_count;\n"
               "void *pico8_grow(void *p, size_t n) {\n"
               "    void *q = malloc(n);\n"
               "    free(q);\n"
               "    return realloc(p, n);\n"
               "}\n");
    write_file(DIR "/src/pico8_clean.c", "int pico8_half(int x) { return x / 2; }\n");
    assert(system("cc -std=c99 -c -o " ALLOCS " " DIR "/src/pico8_allocs.c") == 0);
    assert(system("cc -std=c99 -c -o " CLEAN " " DIR "/src/pico8_clean.c") == 0);
}

/* pico8_transient_user.c : deux references a la poignee transitoire, dans
 * le code et dans les donnees, et un free. */
static void setup_transient_user(void) {
    setup();
    write_file(DIR "/src/pico8_transient_user.c",
               "#include <stdlib.h>\n"
               "extern const int pico8_zone_transitoire;\n"
               "const int *pico8_kept = &pico8_zone_transitoire;\n"
               "int pico8_use(int *p) {\n"
               "    free(p);\n"
               "    return pico8_zone_transitoire;\n"
               "}\n");
    assert(system("cc -std=c99 -c -o " TRANSIENT_USER " " DIR "/src/pico8_transient_user.c") == 0);
}

static void test_exact_counts_pass(void) {
    setup();
    write_file(LIST, "# commentaire\nsrc/pico8_allocs.c 3\n");
    assert(run_guard(ALLOCS " " CLEAN) == 0);
}

static void test_unlisted_allocating_file_fails_with_the_line_to_add(void) {
    setup();
    write_file(LIST, "");
    assert(run_guard(ALLOCS " " CLEAN) == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "src/pico8_allocs.c 3\n") != NULL);
    assert(strstr(out, "pico8_clean") == NULL);
}

static void test_listed_count_above_actual_fails(void) {
    setup();
    write_file(LIST, "src/pico8_allocs.c 4\n");
    assert(run_guard(ALLOCS " " CLEAN) == 1);
    assert(strstr(read_file(OUT), "src/pico8_allocs.c 3\n") != NULL);
}

static void test_listed_count_below_actual_fails(void) {
    setup();
    write_file(LIST, "src/pico8_allocs.c 2\n");
    assert(run_guard(ALLOCS " " CLEAN) == 1);
    assert(strstr(read_file(OUT), "src/pico8_allocs.c 3\n") != NULL);
}

static void test_listed_file_that_no_longer_allocates_fails(void) {
    setup();
    write_file(LIST, "src/pico8_allocs.c 3\nsrc/pico8_clean.c 1\n");
    assert(run_guard(ALLOCS " " CLEAN) == 1);
    assert(strstr(read_file(OUT), "src/pico8_clean.c") != NULL);
}

static void test_listed_file_missing_from_objects_fails(void) {
    setup();
    write_file(LIST, "src/pico8_allocs.c 3\nsrc/pico8_gone.c 2\n");
    assert(run_guard(ALLOCS " " CLEAN) == 1);
    assert(strstr(read_file(OUT), "src/pico8_gone.c") != NULL);
}

static void test_no_object_is_an_error_not_a_pass(void) {
    setup();
    write_file(LIST, "");
    assert(run_guard("") == 2);
}

/* Le listing de relocations d'un objet Mach-O, simule pour ne pas dependre
 * de la plateforme : les symboles C y portent un `_` en tete. */
static void test_darwin_underscore_symbols_are_counted(void) {
    setup();
    write_file(DIR "/objdump",
               "#!/bin/sh\n"
               "echo 'RELOCATION RECORDS FOR [__text]:'\n"
               "echo '0000000000000012 X86_64_RELOC_BRANCH _malloc'\n"
               "echo '0000000000000020 X86_64_RELOC_BRANCH _calloc'\n"
               "echo '0000000000000028 X86_64_RELOC_BRANCH _pico8_free'\n"
               "echo '0000000000000030 X86_64_RELOC_BRANCH _free'\n");
    assert(system("chmod +x " DIR "/objdump") == 0);
    write_file(LIST, "src/pico8_allocs.c 3\n");
    assert(run_guard_with("OBJDUMP=" DIR "/objdump", ALLOCS) == 0);
    write_file(LIST, "src/pico8_allocs.c 2\n");
    assert(run_guard_with("OBJDUMP=" DIR "/objdump", ALLOCS) == 1);
}

static void test_unlisted_transient_reference_fails_with_the_line_to_add(void) {
    setup_transient_user();
    write_file(LIST, "");
    assert(run_guard(TRANSIENT_USER) == 1);
    assert(strstr(read_file(OUT), "ligne a ajouter : src/pico8_transient_user.c 3\n") != NULL);
}

static void test_two_transient_references_and_a_free_count_three(void) {
    setup_transient_user();
    write_file(LIST, "src/pico8_transient_user.c 3\n");
    assert(run_guard(TRANSIENT_USER) == 0);
    write_file(LIST, "src/pico8_transient_user.c 1\n");
    assert(run_guard(TRANSIENT_USER) == 1);
}

static void test_darwin_underscore_transient_symbol_is_counted(void) {
    setup();
    write_file(DIR "/objdump",
               "#!/bin/sh\n"
               "echo 'RELOCATION RECORDS FOR [__text]:'\n"
               "echo '0000000000000012 X86_64_RELOC_GOT_LOAD _pico8_zone_transitoire'\n"
               "echo '0000000000000020 X86_64_RELOC_GOT_LOAD _pico8_zone_transitoire_bis'\n"
               "echo 'RELOCATION RECORDS FOR [__const]:'\n"
               "echo '0000000000000000 X86_64_RELOC_UNSIGNED _pico8_zone_transitoire'\n");
    assert(system("chmod +x " DIR "/objdump") == 0);
    write_file(LIST, "src/pico8_allocs.c 2\n");
    assert(run_guard_with("OBJDUMP=" DIR "/objdump", ALLOCS) == 0);
}

int main(void) {
    test_exact_counts_pass();
    test_unlisted_allocating_file_fails_with_the_line_to_add();
    test_listed_count_above_actual_fails();
    test_listed_count_below_actual_fails();
    test_listed_file_that_no_longer_allocates_fails();
    test_listed_file_missing_from_objects_fails();
    test_no_object_is_an_error_not_a_pass();
    test_darwin_underscore_symbols_are_counted();
    test_unlisted_transient_reference_fails_with_the_line_to_add();
    test_two_transient_references_and_a_free_count_three();
    test_darwin_underscore_transient_symbol_is_counted();
    assert(system("rm -rf " DIR) == 0);
    printf("test_alloc_guard: all tests passed\n");
    return 0;
}
