/* tools/float_inventory.sh, le corps de `make float-inventory`, sur des
 * sources C fabriquees : il se teste par ce qu'il doit empecher. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/float_inventory.sh"
#define DIR "tmp/test_float_inventory_work"
#define SRC DIR "/pico8_specimen.c"
#define OUT DIR "/out.txt"

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

static int run_inventory(const char *sources, const char *out) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " %s > %s 2>&1", sources, out);
    return system(cmd);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
}

static void test_constant_carries_function_rank_and_literal(void) {
    reset();
    write_file(SRC,
               "static pico8_fixed_t pico8_one(void) {\n"
               "    return pico8_fixed_from_double(0.27);\n"
               "}\n");
    assert(run_inventory(SRC, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC " | pico8_one | constante | 1 | ligne 2 | - | 0.27\n") != NULL);
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

static void test_each_token_gets_one_category_and_a_rank_in_its_function(void) {
    reset();
    write_file(SRC, SPECIMEN);
    assert(run_inventory(SRC, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC " | pico8_one | constante | 1 | ligne 2 | - | 0.27\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | constante | 1 | ligne 8 | - | 1.5\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | cast | 1 | ligne 7 | a auditer | pos.x\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | cast | 2 | ligne 7 | a auditer | f(pos.y)\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | autre | 1 | ligne 10 | - | double\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | reecrit | 1 | ligne 10 | - | pow\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | autre | 2 | ligne 10 | - | pico8_fixed_to_double\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | reecrit | 2 | ligne 11 | - | pico8_fixed_from_double(d)\n") != NULL);
    assert(strstr(out, SRC " | pico8_two | reecrit | 3 | ligne 12 | - | %.4f\n") != NULL);
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
    assert(strstr(out, "| pico8_fixed_sin | autre | 1 | ligne 2 | - | pico8_fixed_from_double(-sin(radians))\n") != NULL);
    assert(strstr(out, "| pico8_fixed_sin | reecrit | 1 | ligne 2 | - | sin\n") != NULL);
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

static void test_lines_added_above_change_only_line_numbers(void) {
    reset();
    write_file(SRC, SPECIMEN);
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
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_inventory: all tests passed\n");
    return 0;
}
