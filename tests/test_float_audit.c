/* tools/float_audit.sh, le corps de `make float-audit`, sur des sources et des
 * enregistrements fabriques : il se teste par ce qu'il doit empecher, une
 * porte verte alors qu'un site a change de valeur, a recu un negatif, n'a
 * jamais ete atteint, ou qu'aucun site n'a ete trouve. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/float_audit.sh"
#define DIR "tmp/test_float_audit_work"
#define SRC_A DIR "/src/pico8_a.c"
#define SRC_B DIR "/src/pico8_b.c"
#define SRC_FIXED DIR "/src/pico8_fixed.c"
#define SOURCES SRC_A " " SRC_B " " SRC_FIXED
#define RECORDS DIR "/aud.txt"
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

static int run_audit(const char *records, const char *sources, const char *out) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " %s %s > %s 2>&1", records, sources, out);
    return system(cmd);
}

#define SRC_A_TEXT \
    "#include \"pico8_fixed.h\"\n" \
    "\n" \
    "int pico8_one(pico8_fixed_t n) {\n" \
    "    return pico8_fixed_flr_int(n);\n" \
    "}\n" \
    "\n" \
    "int pico8_two(pico8_fixed_t a, pico8_fixed_t b) {\n" \
    "    return pico8_fixed_flr_int(a) + pico8_fixed_flr_int(b);\n" \
    "}\n"

#define SRC_B_TEXT \
    "static int pico8_three(pico8_fixed_t n) {\n" \
    "    int k = 0;\n" \
    "    k += pico8_fixed_flr_int(n);\n" \
    "    return k;\n" \
    "}\n"

#define SRC_FIXED_TEXT \
    "int32_t pico8_fixed_flr_int(pico8_fixed_t x) {\n" \
    "    return pico8_fixed_flr(x) / 65536;\n" \
    "}\n"

/* AUD fichier fonction ligne passages negatifs ecarts premier-fautif. */
#define RECORDS_TEXT \
    "AUD\t" SRC_A "\tpico8_one\t4\t3\t0\t0\t-\n" \
    "AUD\t" SRC_A "\tpico8_two\t8\t10\t0\t0\t-\n" \
    "AUD\t" SRC_B "\tpico8_three\t3\t2\t0\t0\t-\n"

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR "/src") == 0);
    write_file(SRC_A, SRC_A_TEXT);
    write_file(SRC_B, SRC_B_TEXT);
    write_file(SRC_FIXED, SRC_FIXED_TEXT);
}

static void test_clean_site_is_ok_and_passes(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_A " | pico8_one | ligne 4 | passages 3 | negatifs 0 | ecarts 0 | ok\n") != NULL);
}

/* L'ancienne troncature et le nouveau flr divergent : le site a change de
 * valeur, meme si aucun pixel ne le montre. */
static void test_mismatch_is_ecart_and_fails(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT "AUD\t" SRC_A "\tpico8_one\t4\t1\t0\t1\t0xFFFF8000\n");
    assert(run_audit(RECORDS, SOURCES, OUT) != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_A " | pico8_one | ligne 4 | passages 4 | negatifs 0 | ecarts 1 | ECART | premier fautif 0xFFFF8000\n") != NULL);
}

/* Une preuve de signe (b) refutee par la mesure. */
static void test_negative_argument_is_negatif_and_fails(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT "AUD\t" SRC_B "\tpico8_three\t3\t1\t1\t0\t0xFFFF0000\n");
    assert(run_audit(RECORDS, SOURCES, OUT) != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_B " | pico8_three | ligne 3 | passages 3 | negatifs 1 | ecarts 0 | NEGATIF | premier fautif 0xFFFF0000\n") != NULL);
}

/* Un site jamais atteint n'a rien verifie : il ne doit jamais passer pour
 * sans ecart. */
static void test_unreached_site_is_non_atteint_and_fails(void) {
    reset();
    write_file(RECORDS, "AUD\t" SRC_A "\tpico8_one\t4\t3\t0\t0\t-\n"
                        "AUD\t" SRC_A "\tpico8_two\t8\t10\t0\t0\t-\n");
    assert(run_audit(RECORDS, SOURCES, OUT) != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_B " | pico8_three | ligne 3 | passages 0 | negatifs 0 | ecarts 0 | NON ATTEINT\n") != NULL);
}

/* Une porte qui ne voit aucun site ne passe pas. */
static void test_sources_without_calls_fail_with_aucun_site(void) {
    reset();
    write_file(RECORDS, "");
    assert(run_audit(RECORDS, SRC_FIXED, OUT) != 0);
    assert(strstr(read_file(OUT), "aucun site") != NULL);
}

static void test_definition_in_fixed_module_is_not_a_site(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    assert(strstr(read_file(OUT), "pico8_fixed.c") == NULL);
}

static void test_two_calls_on_one_line_share_and_say_so(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_A " | pico8_two | ligne 8 | passages 10 | negatifs 0 | ecarts 0 | ok (ligne partagee)\n") != NULL);
    assert(strstr(out, SRC_A " | pico8_one | ligne 4 | passages 3 | negatifs 0 | ecarts 0 | ok\n") != NULL);
}

/* Chaque rejeu ajoute sa part au meme fichier : tout s'additionne, le
 * premier fautif reste le premier vu. */
static void test_records_of_two_replays_are_summed(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT "AUD\t" SRC_A "\tpico8_one\t4\t1\t1\t1\t0xFFFF8000\n"
                                     "AUD\t" SRC_A "\tpico8_one\t4\t5\t2\t0\t0xFFFE0000\n");
    assert(run_audit(RECORDS, SOURCES, OUT) != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "| pico8_one | ligne 4 | passages 9 | negatifs 3 | ecarts 1 | ECART NEGATIF | premier fautif 0xFFFF8000\n") != NULL);
}

/* Une table d'enregistrement pleine a perdu des sites, qui sortiraient NON
 * ATTEINT sans l'etre. */
static void test_overflowed_records_stop_the_report(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT "OVERFLOW\tPICO8_FLOAT_AUDIT_MAX_SITES trop petit\n");
    assert(run_audit(RECORDS, SOURCES, OUT) != 0);
    assert(strstr(read_file(OUT), "OVERFLOW") != NULL);
}

static void test_report_ends_with_counts(void) {
    reset();
    write_file(RECORDS, "AUD\t" SRC_A "\tpico8_one\t4\t3\t1\t1\t0xFFFF8000\n"
                        "AUD\t" SRC_A "\tpico8_two\t8\t10\t0\t0\t-\n");
    assert(run_audit(RECORDS, SOURCES, OUT) != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "audit sites 4\n") != NULL);
    assert(strstr(out, "audit atteint 3\n") != NULL);
    assert(strstr(out, "audit non atteint 1\n") != NULL);
    assert(strstr(out, "audit ecart 1\n") != NULL);
    assert(strstr(out, "audit negatif 1\n") != NULL);
    assert(strstr(out, "audit ligne partagee 2\n") != NULL);
    assert(strstr(out, "audit passages 13\n") != NULL);
}

static void test_two_runs_give_identical_reports(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, DIR "/one.txt") == 0);
    assert(run_audit(RECORDS, SOURCES, DIR "/two.txt") == 0);
    assert(system("cmp -s " DIR "/one.txt " DIR "/two.txt") == 0);
}

int main(void) {
    test_clean_site_is_ok_and_passes();
    test_mismatch_is_ecart_and_fails();
    test_negative_argument_is_negatif_and_fails();
    test_unreached_site_is_non_atteint_and_fails();
    test_sources_without_calls_fail_with_aucun_site();
    test_definition_in_fixed_module_is_not_a_site();
    test_two_calls_on_one_line_share_and_say_so();
    test_records_of_two_replays_are_summed();
    test_overflowed_records_stop_the_report();
    test_report_ends_with_counts();
    test_two_runs_give_identical_reports();
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_audit: all tests passed\n");
    return 0;
}
