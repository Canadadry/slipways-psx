/* tools/constant_variants.sh, le harnais d'attribution, sur un arbre source
 * fabrique : un pilote minuscule de meme interface que pico8_replay
 * (evenements, capture, dump), lie au vrai pico8_fixed.c, dont la frame 3 ne
 * change que si pico8_fixed_from_double(0.27) vaut la valeur tronquee. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_constant_variants_work"
#define ROOT DIR "/src"
#define OUTDIR DIR "/sortie"
#define INV DIR "/inventaire.txt"
#define PROBE DIR "/releve.txt"
#define LOG "tmp/test_constant_variants_log.txt"
#define TOOL "./tmp/pico8_frames"
#define SNAPSHOT DIR "/src.orig"

#define A_EVENTS DIR "/a_events.txt"
#define B_EVENTS DIR "/b_events.txt"
#define BAD_EVENTS DIR "/bad_events.txt"

#define SOURCES ROOT "/driver.c " ROOT "/pico8_screencast.c " ROOT "/pico8_mem.c " ROOT "/pico8_fixed.c " ROOT "/pico8_fixed_test_double.c"
#define PAIRS A_EVENTS " " A_EVENTS ".frames.hash " B_EVENTS " " B_EVENTS ".frames.hash"
#define SCRIPT "sh tools/constant_variants.sh " ROOT " '" SOURCES "' " INV " " PROBE " " TOOL " " OUTDIR

#define DRIVER_TEXT \
    "#include \"pico8_fixed.h\"\n" \
    "#include \"pico8_fixed_test_double.h\"\n" \
    "#include \"pico8_mem.h\"\n" \
    "#include \"pico8_screencast.h\"\n" \
    "#include <stdio.h>\n" \
    "int main(int argc, char **argv) {\n" \
    "    int frames = 0;\n" \
    "    FILE *ev = (argc > 3) ? fopen(argv[1], \"r\") : NULL;\n" \
    "    if (!ev || fscanf(ev, \"%d\", &frames) != 1 || frames < 0) return 1;\n" \
    "    fclose(ev);\n" \
    "    pico8_fixed_t tenth = pico8_fixed_from_double(0.1);\n" \
    "    pico8_fixed_t eighth = pico8_fixed_from_double(0.125);\n" \
    "    pico8_fixed_t half = pico8_fixed_from_double(0.5);\n" \
    "    pico8_fixed_t shade = pico8_fixed_from_double(0.27);\n" \
    "    pico8_mem_t mem;\n" \
    "    pico8_mem_init(&mem);\n" \
    "    pico8_screencast_writer_t w = { NULL };\n" \
    "    if (!pico8_screencast_writer_open(&w, argv[2])) return 1;\n" \
    "    for (int i = 0; i < frames; i++) {\n" \
    "        int flip = (i == 3 && shade == 0x451e) || half != 0x8000 || eighth != 0x2000 || tenth == 0;\n" \
    "        pico8_poke(&mem, PICO8_ADDR_SCREEN, flip ? 2 : 1);\n" \
    "        pico8_poke(&mem, PICO8_ADDR_SCREEN + 1, (unsigned char)(i & 0x0f));\n" \
    "        if (!pico8_screencast_write_frame(&w, &mem)) return 1;\n" \
    "    }\n" \
    "    if (!pico8_screencast_writer_close(&w)) return 1;\n" \
    "    FILE *dump = fopen(argv[3], \"w\");\n" \
    "    if (!dump) return 1;\n" \
    "    fprintf(dump, \"%d frames\\n\", frames);\n" \
    "    fclose(dump);\n" \
    "    return 0;\n" \
    "}\n"

#define SITE(lit, rank) ROOT "/driver.c | main | constante | " rank " | ligne 1 | - | - | " lit "\n"
#define INV_HEADER "#   constante  pico8_fixed_from_double(<litteral numerique>)\n"
#define INV_TEXT INV_HEADER SITE("0.1", "1") SITE("0.125", "2") SITE("0.5", "3") SITE("0.27", "4")

/* 0.1 et 0.27 : PICO-8 tronque, l'arrondi au plus proche donne un LSB de plus. */
#define PROBE_TEXT \
    "## a literals\n" \
    "0.1 0x0000.1999\n" \
    "0.125 0x0000.2000\n" \
    "0.27 0x0000.451e\n" \
    "0.5 0x0000.8000\n" \
    "## b rounding\n"

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

static int run(const char *args) {
    char cmd[2048];
    snprintf(cmd, sizeof cmd, SCRIPT " %s > " LOG " 2>&1", args);
    return system(cmd);
}

/* Les references sont celles du pilote non modifie, comme les
 * <events>.frames.hash du filet sont celles de src/. */
static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " ROOT) == 0);
    assert(system("cp src/*.h src/pico8_screencast.c src/pico8_mem.c src/pico8_fixed.c src/pico8_fixed_test_double.c " ROOT) == 0);
    write_file(ROOT "/driver.c", DRIVER_TEXT);
    write_file(INV, INV_TEXT);
    write_file(PROBE, PROBE_TEXT);
    write_file(A_EVENTS, "6\n");
    write_file(B_EVENTS, "3\n");
    write_file(BAD_EVENTS, "-1\n");
    assert(system("${CC:-cc} -std=c99 -Wall -Wextra -Werror -o " DIR "/driver " SOURCES " -lm") == 0);
    assert(system(DIR "/driver " A_EVENTS " " DIR "/a.bin " DIR "/a.dump") == 0);
    assert(system(DIR "/driver " B_EVENTS " " DIR "/b.bin " DIR "/b.dump") == 0);
    assert(system(TOOL " hashes " DIR "/a.bin " A_EVENTS ".frames.hash") == 0);
    assert(system(TOOL " hashes " DIR "/b.bin " B_EVENTS ".frames.hash") == 0);
    assert(system("rm -f " DIR "/driver " DIR "/a.bin " DIR "/b.bin " DIR "/a.dump " DIR "/b.dump") == 0);
    assert(system("cp -R " ROOT " " SNAPSHOT) == 0);
}

static void test_truncated_constant_flips_exactly_the_known_frame(void) {
    reset();
    assert(run("'' " PAIRS) == 0);
    const char *out = read_file(OUTDIR "/synthese.txt");
    assert(strstr(out, "variante lit-0.27 : 0.27 -> 0x0000.451e\n") != NULL);
    assert(strstr(out, "  " A_EVENTS " : premiere 3 | frames 1 | plages 3\n") != NULL);
}

/* Le tronque de 0.1 n'est lu par aucune frame : consigne, pas un echec. Sa
 * variante ne touche pas 0.125, sinon elle compterait deux remplacements. */
static void test_unreached_constant_is_recorded_as_zero_frame(void) {
    reset();
    assert(run("'' " PAIRS) == 0);
    const char *out = read_file(OUTDIR "/synthese.txt");
    assert(strstr(out, "variante lit-0.1 : 0.1 -> 0x0000.1999\n"
                       "  site : " ROOT "/driver.c | main | 1\n"
                       "  " A_EVENTS " : 0 frame\n"
                       "  " B_EVENTS " : 0 frame\n") != NULL);
    assert(strstr(out, "lit-0.125") == NULL);
    assert(strstr(out, "lit-0.5") == NULL);
    assert(strstr(read_file(OUTDIR "/lit-0.1/src/driver.c"), "pico8_fixed_from_double(0.125)") != NULL);
}

static void test_named_exact_literal_diverges_nowhere(void) {
    reset();
    assert(run("0.5 " PAIRS) == 0);
    const char *out = read_file(OUTDIR "/synthese.txt");
    assert(strstr(out, "variante lit-0.5 : 0.5 -> 0x0000.8000\n") != NULL);
    assert(strstr(out, "  " A_EVENTS " : 0 frame\n") != NULL);
    assert(strstr(out, "  " B_EVENTS " : 0 frame\n") != NULL);
    assert(strstr(out, "lit-0.27") == NULL);
}

/* Rien n'est ecrit hors de la sortie, l'arbre d'origine compris, et chaque
 * variante a sa propre capture et son propre dump. */
static void test_variants_write_only_in_their_own_directory(void) {
    reset();
    assert(system("cd " DIR " && find . -path ./sortie -prune -o -print | sort > ../cv_before.txt") == 0);
    assert(run("'' " PAIRS) == 0);
    assert(system("cd " DIR " && find . -path ./sortie -prune -o -print | sort > ../cv_after.txt") == 0);
    assert(system("cmp -s tmp/cv_before.txt tmp/cv_after.txt") == 0);
    assert(system("diff -r " ROOT " " SNAPSHOT " > /dev/null") == 0);
    assert(strstr(read_file(OUTDIR "/lit-0.27/frames/tmp_test_constant_variants_work_a_events.txt.dump.txt"), "6 frames") != NULL);
    assert(strstr(read_file(OUTDIR "/lit-0.1/frames/tmp_test_constant_variants_work_a_events.txt.dump.txt"), "6 frames") != NULL);
    assert(system("rm -f tmp/cv_before.txt tmp/cv_after.txt") == 0);
}

static void test_site_count_mismatch_fails_the_variant(void) {
    reset();
    write_file(INV, INV_TEXT SITE("0.27", "5"));
    assert(run("'' " PAIRS) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "variante lit-0.27 : 1 remplacement(s), 2 site(s)") != NULL);
}

static void test_failing_replay_fails_the_pass_naming_the_variant(void) {
    reset();
    assert(run("0.27 " PAIRS " " BAD_EVENTS " " BAD_EVENTS ".frames.hash") != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "variante lit-0.27 : " BAD_EVENTS " : rejeu en echec") != NULL);
}

int main(void) {
    test_truncated_constant_flips_exactly_the_known_frame();
    test_unreached_constant_is_recorded_as_zero_frame();
    test_named_exact_literal_diverges_nowhere();
    test_variants_write_only_in_their_own_directory();
    test_site_count_mismatch_fails_the_variant();
    test_failing_replay_fails_the_pass_naming_the_variant();
    puts("test_constant_variants: ok");
    return 0;
}
