/* tools/float_inventory_check.sh, la verification de fraicheur de `make test`,
 * sur un inventaire versionne et des sources fabriques : il se teste par ce
 * qu'il doit empecher. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_float_inventory_check_work"
#define SRC DIR "/pico8_specimen.c"
#define VERSIONED DIR "/float_inventory.txt"
#define FRESH DIR "/regenere.txt"
#define LOG DIR "/log.txt"

#define SPECIMEN \
    "static void pico8_two(pico8_vec_t pos) {\n" \
    "    pico8_spr((int)pico8_fixed_to_double(pos.x), 0);\n" \
    "}\n"

/* Le meme coeur, avec un cast de plus. */
#define SPECIMEN_WITH_EXTRA_CAST \
    "static void pico8_two(pico8_vec_t pos) {\n" \
    "    pico8_spr((int)pico8_fixed_to_double(pos.x), 0);\n" \
    "    pico8_spr((int)pico8_fixed_to_double(pos.y), 0);\n" \
    "}\n"

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

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
}

static void version_inventory_of(const char *source_text) {
    write_file(SRC, source_text);
    assert(system("sh tools/float_inventory.sh " SRC " > " VERSIONED) == 0);
}

static int run_check(void) {
    return system("sh tools/float_inventory_check.sh " VERSIONED " " FRESH " " SRC
                  " > " LOG " 2>&1");
}

static void test_fresh_inventory_passes(void) {
    reset();
    version_inventory_of(SPECIMEN);
    assert(run_check() == 0);
}

static void test_added_cast_fails_and_names_the_command(void) {
    reset();
    version_inventory_of(SPECIMEN);
    write_file(SRC, SPECIMEN_WITH_EXTRA_CAST);
    assert(run_check() != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "make float-inventory") != NULL);
    assert(strstr(log, "pos.y") != NULL);
}

static void test_versioned_inventory_is_never_rewritten(void) {
    reset();
    version_inventory_of(SPECIMEN);
    char before[1 << 16];
    strcpy(before, read_file(VERSIONED));
    write_file(SRC, SPECIMEN_WITH_EXTRA_CAST);
    assert(run_check() != 0);
    assert(strcmp(read_file(VERSIONED), before) == 0);
}

static void test_missing_versioned_inventory_fails(void) {
    reset();
    write_file(SRC, SPECIMEN);
    assert(run_check() != 0);
    assert(strstr(read_file(LOG), "make float-inventory") != NULL);
}

/* Un jeton hors categorie arrete l'inventaire (code 2) : la verification
 * echoue aussi, meme si la sortie partielle egalait la copie versionnee. */
static void test_inventory_failure_fails_the_check(void) {
    reset();
    version_inventory_of(SPECIMEN);
    write_file(SRC,
               "static void pico8_two(pico8_vec_t pos) {\n"
               "    pico8_spr((int)pico8_fixed_to_double(pos.x), 0);\n"
               "}\n"
               "/* doubled */\n");
    assert(run_check() != 0);
}

int main(void) {
    test_fresh_inventory_passes();
    test_added_cast_fails_and_names_the_command();
    test_versioned_inventory_is_never_rewritten();
    test_missing_versioned_inventory_fails();
    test_inventory_failure_fails_the_check();
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_inventory_check: all tests passed\n");
    return 0;
}
