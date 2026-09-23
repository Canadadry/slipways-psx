/* tools/float_audit.sh, le corps de `make float-audit`, sur des sources, une
 * liste C1 et des enregistrements fabriques : il se teste par ce qu'il doit
 * empecher, une porte verte alors qu'un site C1 a change de valeur, a recu un
 * negatif, n'a jamais ete atteint, qu'un site D rend autre chose que flr,
 * qu'une liste C1 est perimee, qu'un enregistrement est mal forme ou absent,
 * ou qu'aucun site n'a ete trouve. */

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
#define RECORDS_B DIR "/aud_b.txt"
#define EVENTS_A "events/a.json"
#define EVENTS_B "events/b.json"
#define C1_LIST DIR "/c1.txt"
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

/* pairs : "<events> <enregistrements>..." */
static int run_audit_pairs(const char *pairs, const char *sources, const char *out) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " " C1_LIST " %s -- %s > %s 2>&1", pairs, sources, out);
    return system(cmd);
}

static int run_audit(const char *records, const char *sources, const char *out) {
    char pairs[256];
    snprintf(pairs, sizeof pairs, EVENTS_A " %s", records);
    return run_audit_pairs(pairs, sources, out);
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
    "}\n" \
    "\n" \
    "int pico8_four(pico8_fixed_t n) {\n" \
    "    int a = pico8_fixed_flr_int(n);\n" \
    "    int b = pico8_fixed_flr_int(n);\n" \
    "    return a + pico8_fixed_flr_int((n + 1));\n" \
    "}\n"

#define SRC_FIXED_TEXT \
    "int32_t pico8_fixed_flr_int(pico8_fixed_t x) {\n" \
    "    return pico8_fixed_flr(x) / 65536;\n" \
    "}\n"

/* AUD fichier fonction ligne passages negatifs ecarts premier-fautif
 * fractionnaires nouveau!=flr frame-du-premier-ecart. */
#define RECORDS_FOUR \
    "AUD\t" SRC_B "\tpico8_four\t8\t1\t0\t0\t-\t0\t0\t-\n" \
    "AUD\t" SRC_B "\tpico8_four\t9\t1\t0\t0\t-\t0\t0\t-\n" \
    "AUD\t" SRC_B "\tpico8_four\t10\t1\t0\t0\t-\t0\t0\t-\n"

#define RECORDS_TEXT \
    "AUD\t" SRC_A "\tpico8_one\t4\t3\t0\t0\t-\t0\t0\t-\n" \
    "AUD\t" SRC_A "\tpico8_two\t8\t10\t0\t0\t-\t0\t0\t-\n" \
    "AUD\t" SRC_B "\tpico8_three\t3\t2\t0\t0\t-\t0\t0\t-\n" \
    RECORDS_FOUR

#define C1_ALL \
    "# fichier | fonction | extrait | occurrence\n" \
    "\n" \
    SRC_A " | pico8_one | n | 1\n" \
    SRC_A " | pico8_two | a | 1\n" \
    SRC_A " | pico8_two | b | 1\n" \
    SRC_B " | pico8_three | n | 1\n" \
    SRC_B " | pico8_four | n | 1\n" \
    SRC_B " | pico8_four | n | 2\n" \
    SRC_B " | pico8_four | (n + 1) | 1\n"

#define C1_WITHOUT_THREE \
    SRC_A " | pico8_one | n | 1\n" \
    SRC_A " | pico8_two | a | 1\n" \
    SRC_A " | pico8_two | b | 1\n" \
    SRC_B " | pico8_four | n | 1\n" \
    SRC_B " | pico8_four | n | 2\n" \
    SRC_B " | pico8_four | (n + 1) | 1\n"

#define C1_WITHOUT_THREE_AND_TWO \
    SRC_B " | pico8_three | n | 1\n" \
    SRC_B " | pico8_four | n | 1\n" \
    SRC_B " | pico8_four | n | 2\n" \
    SRC_B " | pico8_four | (n + 1) | 1\n"

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR "/src") == 0);
    write_file(SRC_A, SRC_A_TEXT);
    write_file(SRC_B, SRC_B_TEXT);
    write_file(SRC_FIXED, SRC_FIXED_TEXT);
    write_file(C1_LIST, C1_ALL);
}

static void test_clean_site_is_ok_and_passes(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_A " | pico8_one | ligne 4 | C1 | appels 3 | negatifs 0 | fractionnaires 0 | ancien!=nouveau 0 | nouveau!=flr 0 | ok\n") != NULL);
}

/* L'ancienne troncature et le nouveau flr divergent : le site a change de
 * valeur, meme si aucun pixel ne le montre. */
static void test_mismatch_is_ecart_and_fails(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT "AUD\t" SRC_A "\tpico8_one\t4\t1\t0\t1\t0xFFFF8000\t0\t0\t-\n");
    assert(run_audit(RECORDS, SOURCES, OUT) == 3 << 8);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_A " | pico8_one | ligne 4 | C1 | appels 4 | negatifs 0 | fractionnaires 0 | ancien!=nouveau 1 | nouveau!=flr 0 | ECART | premier fautif 0xFFFF8000\n") != NULL);
}

/* Une preuve de signe (b) refutee par la mesure. */
static void test_negative_argument_is_negatif_and_fails(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT "AUD\t" SRC_B "\tpico8_three\t3\t1\t1\t0\t0xFFFF0000\t0\t0\t-\n");
    assert(run_audit(RECORDS, SOURCES, OUT) == 3 << 8);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_B " | pico8_three | ligne 3 | C1 | appels 3 | negatifs 1 | fractionnaires 0 | ancien!=nouveau 0 | nouveau!=flr 0 | NEGATIF | premier fautif 0xFFFF0000\n") != NULL);
}

/* Un site jamais atteint n'a rien verifie : il ne doit jamais passer pour
 * sans ecart. */
static void test_unreached_site_is_non_atteint_and_fails(void) {
    reset();
    write_file(RECORDS, "AUD\t" SRC_A "\tpico8_one\t4\t3\t0\t0\t-\t0\t0\t-\n"
                        "AUD\t" SRC_A "\tpico8_two\t8\t10\t0\t0\t-\t0\t0\t-\n" RECORDS_FOUR);
    assert(run_audit(RECORDS, SOURCES, OUT) == 3 << 8);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_B " | pico8_three | ligne 3 | C1 | appels 0 | negatifs 0 | fractionnaires 0 | ancien!=nouveau 0 | nouveau!=flr 0 | NON ATTEINT\n") != NULL);
}

/* Hors de la liste C1, le meme site est de classe D : son negatif et son
 * ecart sont la correction voulue, des mesures et pas des echecs. */
static void test_site_absent_from_c1_list_is_d_and_measured(void) {
    reset();
    write_file(C1_LIST, C1_WITHOUT_THREE);
    write_file(RECORDS, RECORDS_TEXT "AUD\t" SRC_B "\tpico8_three\t3\t4\t2\t3\t0xFFFF8000\t1\t0\t12\n");
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_B " | pico8_three | ligne 3 | D | appels 6 | negatifs 2 | fractionnaires 1 | ancien!=nouveau 3 | nouveau!=flr 0 | ok | premier fautif 0xFFFF8000 | premier ancien!=nouveau " EVENTS_A " frame 12\n") != NULL);
    assert(strstr(out, "audit sites C1 6\n") != NULL);
    assert(strstr(out, "audit sites D 1\n") != NULL);
}

static void test_d_site_whose_new_value_is_not_flr_fails(void) {
    reset();
    write_file(C1_LIST, C1_WITHOUT_THREE);
    write_file(RECORDS, RECORDS_TEXT "AUD\t" SRC_B "\tpico8_three\t3\t4\t2\t3\t0xFFFF8000\t1\t2\t12\n");
    assert(run_audit(RECORDS, SOURCES, OUT) == 3 << 8);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_B " | pico8_three | ligne 3 | D | appels 6 | negatifs 2 | fractionnaires 1 | ancien!=nouveau 3 | nouveau!=flr 2 | NOUVEAU != FLR | premier fautif 0xFFFF8000 | premier ancien!=nouveau " EVENTS_A " frame 12\n") != NULL);
    assert(strstr(out, "audit nouveau!=flr 1\n") != NULL);
}

/* Un site D sans ancien != nouveau sur le filet est liste, pas en echec :
 * c'est l'etape D qui decide du test au niveau valeur. */
static void test_d_site_without_change_is_listed_unseen_without_failing(void) {
    reset();
    write_file(C1_LIST, C1_WITHOUT_THREE);
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_B " | pico8_three | ligne 3 | D | appels 2 | negatifs 0 | fractionnaires 0 | ancien!=nouveau 0 | nouveau!=flr 0 | correction non vue par le filet\n") != NULL);
    assert(strstr(out, "audit correction non vue 1\n") != NULL);
}

static void test_unreached_d_site_is_listed_unseen_without_failing(void) {
    reset();
    write_file(C1_LIST, C1_WITHOUT_THREE);
    write_file(RECORDS, "AUD\t" SRC_A "\tpico8_one\t4\t3\t0\t0\t-\t0\t0\t-\n"
                        "AUD\t" SRC_A "\tpico8_two\t8\t10\t0\t0\t-\t0\t0\t-\n" RECORDS_FOUR);
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_B " | pico8_three | ligne 3 | D | appels 0 | negatifs 0 | fractionnaires 0 | ancien!=nouveau 0 | nouveau!=flr 0 | NON ATTEINT, correction non vue par le filet\n") != NULL);
    assert(strstr(out, "audit non atteint 1\n") != NULL);
    assert(strstr(out, "audit correction non vue 1\n") != NULL);
}

/* Une liste perimee ne passe jamais pour une garde. */
static void test_c1_entry_without_site_fails(void) {
    reset();
    write_file(C1_LIST, C1_ALL SRC_A " | pico8_one | gone | 1\n");
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, OUT) == 5 << 8);
    assert(strstr(read_file(OUT), "liste C1 : entree sans site : " SRC_A " | pico8_one | gone | 1\n") != NULL);
}

static void test_duplicated_c1_key_fails(void) {
    reset();
    write_file(C1_LIST, C1_ALL SRC_B " | pico8_three | n | 1\n");
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, OUT) == 5 << 8);
    assert(strstr(read_file(OUT), "liste C1 : cle dupliquee : " SRC_B " | pico8_three | n | 1\n") != NULL);
}

/* L'occurrence, pas la ligne, distingue deux appels de meme extrait : la
 * liste peut n'en designer qu'un. */
static void test_same_extract_twice_takes_occurrences_one_and_two(void) {
    reset();
    write_file(C1_LIST, SRC_A " | pico8_one | n | 1\n"
                        SRC_A " | pico8_two | a | 1\n"
                        SRC_A " | pico8_two | b | 1\n"
                        SRC_B " | pico8_three | n | 1\n"
                        SRC_B " | pico8_four | n | 2\n"
                        SRC_B " | pico8_four | (n + 1) | 1\n");
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, SRC_B " | pico8_four | ligne 8 | D | ") != NULL);
    assert(strstr(out, SRC_B " | pico8_four | ligne 9 | C1 | ") != NULL);
    assert(strstr(out, SRC_B " | pico8_four | ligne 10 | C1 | ") != NULL);
}

/* Une porte qui ne voit aucun site ne passe pas. */
static void test_sources_without_calls_fail_with_aucun_site(void) {
    reset();
    write_file(C1_LIST, "");
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
    assert(strstr(out, SRC_A " | pico8_two | ligne 8 | C1 | appels 10 | negatifs 0 | fractionnaires 0 | ancien!=nouveau 0 | nouveau!=flr 0 | ok (ligne partagee)\n") != NULL);
    assert(strstr(out, SRC_A " | pico8_one | ligne 4 | C1 | appels 3 | negatifs 0 | fractionnaires 0 | ancien!=nouveau 0 | nouveau!=flr 0 | ok\n") != NULL);
}

/* Un fichier d'enregistrements par fichier d'evenements : les totaux par
 * site s'additionnent, la frame du premier ecart reste propre a chacun. */
static void test_two_events_files_sum_totals_and_keep_frames_apart(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT "AUD\t" SRC_A "\tpico8_one\t4\t1\t1\t1\t0xFFFF8000\t1\t0\t7\n");
    write_file(RECORDS_B, "AUD\t" SRC_A "\tpico8_one\t4\t5\t2\t2\t0xFFFE8000\t3\t0\t30\n");
    assert(run_audit_pairs(EVENTS_A " " RECORDS " " EVENTS_B " " RECORDS_B, SOURCES, OUT) == 3 << 8);
    const char *out = read_file(OUT);
    assert(strstr(out, "| pico8_one | ligne 4 | C1 | appels 9 | negatifs 3 | fractionnaires 4 | ancien!=nouveau 3 | nouveau!=flr 0 | ECART NEGATIF | premier fautif 0xFFFF8000"
                       " | premier ancien!=nouveau " EVENTS_A " frame 7, " EVENTS_B " frame 30\n") != NULL);
    assert(strstr(out, "audit passages 24\n") != NULL);
}

/* Entree de la regle d'attribution : par fichier d'evenements, la plus petite
 * frame et tous les sites qui l'atteignent. */
static void test_events_section_names_smallest_frame_and_every_tied_site(void) {
    reset();
    write_file(C1_LIST, C1_WITHOUT_THREE);
    write_file(RECORDS, "AUD\t" SRC_A "\tpico8_one\t4\t3\t0\t1\t0x00018000\t1\t0\t9\n"
                        "AUD\t" SRC_A "\tpico8_two\t8\t10\t0\t0\t-\t0\t0\t-\n"
                        "AUD\t" SRC_B "\tpico8_three\t3\t2\t1\t1\t0xFFFF8000\t1\t0\t4\n" RECORDS_FOUR);
    write_file(RECORDS_B, "AUD\t" SRC_A "\tpico8_one\t4\t3\t0\t0\t-\t0\t0\t-\n"
                          "AUD\t" SRC_B "\tpico8_three\t3\t2\t1\t1\t0xFFFF8000\t1\t0\t6\n"
                          "AUD\t" SRC_B "\tpico8_four\t10\t1\t0\t1\t0xFFFF8000\t1\t0\t6\n");
    assert(run_audit_pairs(EVENTS_A " " RECORDS " " EVENTS_B " " RECORDS_B, SOURCES, OUT) == 3 << 8);
    const char *out = read_file(OUT);
    assert(strstr(out, EVENTS_A " | premier ancien!=nouveau frame 4 | " SRC_B " | pico8_three | ligne 3\n") != NULL);
    assert(strstr(out, EVENTS_B " | premier ancien!=nouveau frame 6 | " SRC_B " | pico8_three | ligne 3 ; " SRC_B " | pico8_four | ligne 10\n") != NULL);
}

static void test_events_section_says_aucun_without_change(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    assert(strstr(read_file(OUT), EVENTS_A " | premier ancien!=nouveau aucun\n") != NULL);
}

/* Une ligne partagee est une seule ligne d'attribution, pas deux sites. */
static void test_events_section_names_a_shared_line_once(void) {
    reset();
    write_file(C1_LIST, SRC_A " | pico8_one | n | 1\n" C1_WITHOUT_THREE_AND_TWO);
    write_file(RECORDS, "AUD\t" SRC_A "\tpico8_one\t4\t3\t0\t0\t-\t0\t0\t-\n"
                        "AUD\t" SRC_A "\tpico8_two\t8\t10\t1\t1\t0xFFFF8000\t1\t0\t2\n"
                        "AUD\t" SRC_B "\tpico8_three\t3\t2\t0\t0\t-\t0\t0\t-\n" RECORDS_FOUR);
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    assert(strstr(read_file(OUT), EVENTS_A " | premier ancien!=nouveau frame 2 | " SRC_A " | pico8_two | ligne 8\n") != NULL);
}

/* Un enregistrement a l'ancien format serait compte a zero en silence. */
static void test_old_format_record_fails_with_message(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT "AUD\t" SRC_A "\tpico8_one\t4\t1\t0\t1\t0xFFFF8000\n");
    assert(run_audit(RECORDS, SOURCES, OUT) == 6 << 8);
    assert(strstr(read_file(OUT), "enregistrement mal forme") != NULL);
}

/* Un rejeu mort ne deverse rien : son fichier manque. */
static void test_missing_records_file_fails(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT);
    assert(run_audit_pairs(EVENTS_A " " RECORDS " " EVENTS_B " " DIR "/absent.txt", SOURCES, OUT) == 6 << 8);
    assert(strstr(read_file(OUT), "absent.txt") != NULL);
}

/* Les paires D2/D3 enregistrent a des positions qui ne sont pas des appels
 * de pico8_fixed_flr_int : classe D, sans lecture statique. */
static void test_record_outside_flr_calls_is_reported_as_d(void) {
    reset();
    write_file(RECORDS, RECORDS_TEXT "AUD\t" DIR "/src/pico8_c.c\tpico8_tostr\t42\t5\t0\t2\t0x00008000\t5\t0\t3\n");
    assert(run_audit(RECORDS, SOURCES, OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, DIR "/src/pico8_c.c | pico8_tostr | ligne 42 | D | appels 5 | negatifs 0 | fractionnaires 5 | ancien!=nouveau 2 | nouveau!=flr 0 | ok | premier fautif 0x00008000"
                       " | premier ancien!=nouveau " EVENTS_A " frame 3\n") != NULL);
    assert(strstr(out, "audit sites D 1\n") != NULL);
    assert(strstr(out, EVENTS_A " | premier ancien!=nouveau frame 3 | " DIR "/src/pico8_c.c | pico8_tostr | ligne 42\n") != NULL);
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
    write_file(RECORDS, "AUD\t" SRC_A "\tpico8_one\t4\t3\t1\t1\t0xFFFF8000\t0\t0\t-\n"
                        "AUD\t" SRC_A "\tpico8_two\t8\t10\t0\t0\t-\t0\t0\t-\n" RECORDS_FOUR);
    assert(run_audit(RECORDS, SOURCES, OUT) != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "audit sites 7\n") != NULL);
    assert(strstr(out, "audit sites C1 7\n") != NULL);
    assert(strstr(out, "audit sites D 0\n") != NULL);
    assert(strstr(out, "audit atteint 6\n") != NULL);
    assert(strstr(out, "audit non atteint 1\n") != NULL);
    assert(strstr(out, "audit ecart 1\n") != NULL);
    assert(strstr(out, "audit negatif 1\n") != NULL);
    assert(strstr(out, "audit nouveau!=flr 0\n") != NULL);
    assert(strstr(out, "audit correction non vue 0\n") != NULL);
    assert(strstr(out, "audit ligne partagee 2\n") != NULL);
    assert(strstr(out, "audit passages 16\n") != NULL);
}

static void test_two_runs_give_identical_reports(void) {
    reset();
    write_file(C1_LIST, C1_WITHOUT_THREE);
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
    test_site_absent_from_c1_list_is_d_and_measured();
    test_d_site_whose_new_value_is_not_flr_fails();
    test_d_site_without_change_is_listed_unseen_without_failing();
    test_unreached_d_site_is_listed_unseen_without_failing();
    test_c1_entry_without_site_fails();
    test_duplicated_c1_key_fails();
    test_same_extract_twice_takes_occurrences_one_and_two();
    test_sources_without_calls_fail_with_aucun_site();
    test_definition_in_fixed_module_is_not_a_site();
    test_two_calls_on_one_line_share_and_say_so();
    test_two_events_files_sum_totals_and_keep_frames_apart();
    test_events_section_names_smallest_frame_and_every_tied_site();
    test_events_section_says_aucun_without_change();
    test_events_section_names_a_shared_line_once();
    test_old_format_record_fails_with_message();
    test_missing_records_file_fails();
    test_record_outside_flr_calls_is_reported_as_d();
    test_overflowed_records_stop_the_report();
    test_report_ends_with_counts();
    test_two_runs_give_identical_reports();
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_audit: all tests passed\n");
    return 0;
}
