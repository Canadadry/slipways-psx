/* tools/float_observe.sh, le corps de `make float-observe`, sur un inventaire
 * annote et des enregistrements fabriques : il se teste par ce qu'il doit
 * empecher, un cast non atteint qui passerait pour jamais negatif et une
 * preuve (b) que la mesure refute. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/float_observe.sh"
#define DIR "tmp/test_float_observe_work"
#define INVENTORY DIR "/inventaire.txt"
#define RECORDS DIR "/obs.txt"
#define RECORDS2 DIR "/obs2.txt"
#define OUT DIR "/out.txt"
#define COMPILER "'cc (Fabrique) 1.2.3'"

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

static int run_observe(const char *records, const char *out) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " " INVENTORY " " COMPILER " %s > %s 2>&1", records, out);
    return system(cmd);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
}

#define INVENTORY_TEXT \
    "# en-tete de l'inventaire\n" \
    "src/pico8_a.c | pico8_one | constante | 1 | ligne 2 | - | - | 0.27\n" \
    "src/pico8_a.c | pico8_one | cast | 1 | ligne 4 | (c) | D1 | dx\n" \
    "src/pico8_a.c | pico8_two | cast | 1 | ligne 7 | a auditer | - | pos.x\n" \
    "src/pico8_a.c | pico8_two | cast | 2 | ligne 7 | a auditer | - | f(pos.y)\n" \
    "src/pico8_a.c | pico8_two | cast | 3 | ligne 9 | (b) | C1 | n\n" \
    "src/pico8_b.c | pico8_three | cast | 1 | ligne 3 | a auditer | - | never\n" \
    "src/pico8_b.c | pico8_three | autre | 1 | ligne 5 | - | - | pico8_fixed_to_double\n" \
    "#\n" \
    "# Comptes\n" \
    "compte cast 5\n"

/* OBS fichier fonction ligne minimum-brut passages. La ligne 5 de pico8_b.c
 * est un appel hors cast : enregistre, mais ecarte par la jointure. */
#define RECORDS_TEXT \
    "OBS\tsrc/pico8_a.c\tpico8_one\t4\t-65536\t3\n" \
    "OBS\tsrc/pico8_a.c\tpico8_two\t7\t-1\t10\n" \
    "OBS\tsrc/pico8_a.c\tpico8_two\t9\t131072\t4\n" \
    "OBS\tsrc/pico8_b.c\tpico8_three\t5\t-8\t2\n"

static void test_negative_minimum_is_seen_negative_in_raw_hex(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_observe(RECORDS, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "src/pico8_a.c | pico8_one | 1 | ligne 4 | (c) | D1 | min 0xFFFF0000 | passages 3 | vu negatif | dx\n") !=
           NULL);
}

static void test_positive_minimum_is_never_negative(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_observe(RECORDS, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "src/pico8_a.c | pico8_two | 3 | ligne 9 | (b) | C1 | min 0x00020000 | passages 4 | jamais negatif | n\n") !=
           NULL);
}

/* Un cast jamais atteint n'a pas de minimum : il ne doit jamais passer pour
 * jamais negatif. */
static void test_unreached_cast_is_not_covered(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_observe(RECORDS, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "src/pico8_b.c | pico8_three | 1 | ligne 3 | a auditer | - | min - | passages 0 | NON COUVERT | never\n") != NULL);
}

static void test_two_casts_on_one_line_share_and_say_so(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_observe(RECORDS, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "| pico8_two | 1 | ligne 7 | a auditer | - | min 0xFFFFFFFF | passages 10 | vu negatif (ligne partagee) | pos.x\n") !=
           NULL);
    assert(strstr(out, "| pico8_two | 2 | ligne 7 | a auditer | - | min 0xFFFFFFFF | passages 10 | vu negatif (ligne partagee) | f(pos.y)\n") !=
           NULL);
    assert(strstr(out, "| pico8_one | 1 | ligne 4 | (c) | D1 | min 0xFFFF0000 | passages 3 | vu negatif | dx\n") != NULL);
}

static void test_non_cast_calls_are_left_out(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_observe(RECORDS, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "ligne 5") == NULL);
    assert(strstr(out, "0xFFFFFFF8") == NULL);
}

/* Un (b) est une preuve de signe : un negatif mesure la refute. */
static void test_b_seen_negative_is_a_contradiction_and_fails(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT "OBS\tsrc/pico8_a.c\tpico8_two\t9\t-131072\t1\n");
    assert(run_observe(RECORDS, OUT) != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "| pico8_two | 3 | ligne 9 | (b) | C1 | min 0xFFFE0000 | passages 5 | vu negatif | n\n") != NULL);
    assert(strstr(out, "CONTRADICTION src/pico8_a.c | pico8_two | 3 | ligne 9") != NULL);
    assert(strstr(out, "observation contradiction 1\n") != NULL);
}

static void test_inventory_without_casts_stops_the_report(void) {
    reset();
    write_file(INVENTORY, "# rien\nsrc/pico8_a.c | pico8_one | constante | 1 | ligne 2 | - | - | 0.27\n");
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_observe(RECORDS, OUT) != 0);
    assert(strstr(read_file(OUT), "observation total") == NULL);
}

/* Deux rejeux deversent chacun leur part : les passages s'additionnent, le
 * minimum est le plus petit des deux. */
static void test_records_of_two_replays_are_summed_and_minimised(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT);
    write_file(RECORDS2, "OBS\tsrc/pico8_a.c\tpico8_one\t4\t-196608\t2\n"
                         "OBS\tsrc/pico8_a.c\tpico8_two\t9\t65536\t1\n");
    assert(run_observe(RECORDS " " RECORDS2, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "| pico8_one | 1 | ligne 4 | (c) | D1 | min 0xFFFD0000 | passages 5 | vu negatif | dx\n") != NULL);
    assert(strstr(out, "| pico8_two | 3 | ligne 9 | (b) | C1 | min 0x00010000 | passages 5 | jamais negatif | n\n") != NULL);
}

/* Une table d'enregistrement pleine a perdu des casts, qui sortiraient NON
 * COUVERT sans l'etre. */
static void test_overflowed_records_stop_the_report(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT "OVERFLOW\tPICO8_FLOAT_OBSERVE_MAX_SITES trop petit\n");
    assert(run_observe(RECORDS, OUT) != 0);
    assert(strstr(read_file(OUT), "OVERFLOW") != NULL);
}

static void test_report_ends_with_counts(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_observe(RECORDS, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "observation vu negatif 3\n") != NULL);
    assert(strstr(out, "observation jamais negatif 1\n") != NULL);
    assert(strstr(out, "observation non couvert 1\n") != NULL);
    assert(strstr(out, "observation ligne partagee 2\n") != NULL);
    assert(strstr(out, "observation contradiction 0\n") != NULL);
    assert(strstr(out, "observation total 5\n") != NULL);
}

static void test_header_names_the_compiler_and_the_line_granularity(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_observe(RECORDS, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "# Compilateur : cc (Fabrique) 1.2.3\n") != NULL);
    assert(strstr(out, "granularite") != NULL);
}

static void test_two_runs_give_identical_reports(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_observe(RECORDS, DIR "/one.txt") == 0);
    assert(run_observe(RECORDS, DIR "/two.txt") == 0);
    assert(system("cmp -s " DIR "/one.txt " DIR "/two.txt") == 0);
}

int main(void) {
    test_negative_minimum_is_seen_negative_in_raw_hex();
    test_positive_minimum_is_never_negative();
    test_unreached_cast_is_not_covered();
    test_two_casts_on_one_line_share_and_say_so();
    test_non_cast_calls_are_left_out();
    test_b_seen_negative_is_a_contradiction_and_fails();
    test_inventory_without_casts_stops_the_report();
    test_records_of_two_replays_are_summed_and_minimised();
    test_overflowed_records_stop_the_report();
    test_report_ends_with_counts();
    test_header_names_the_compiler_and_the_line_granularity();
    test_two_runs_give_identical_reports();
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_observe: all tests passed\n");
    return 0;
}
