/* tmp/pico8_replay --from-title rejoue un enregistrement raylib depuis
 * l'ecran titre : le clic de difficulte de l'enregistrement choisit la
 * difficulte. Sans le drapeau, le pilote choisit "reasonable" avant la
 * frame 0 et ce clic tombe sur une galaxie deja generee.
 *
 * Les deux fixtures ne different que par la ligne cliquee a la frame 60
 * (forgiving, reasonable) ; elles ouvrent ensuite le panneau de score,
 * dont le titre porte dk.px (lua.lua:885). */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DRIVER "./tmp/pico8_replay"
#define FORGIVING "tests/fixtures/title_click_forgiving.json"
#define REASONABLE "tests/fixtures/title_click_reasonable.json"
#define CAPTURE "tmp/test_replay_from_title.bin"
#define DUMP_A "tmp/test_replay_from_title_a.txt"
#define DUMP_B "tmp/test_replay_from_title_b.txt"

/* Tampon alloue, que l'appelant libere. */
static char *read_whole_file(const char *path, size_t *out_len) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    long size = ftell(file);
    assert(size > 0);
    rewind(file);
    char *buf = malloc((size_t)size);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, file) == (size_t)size);
    fclose(file);
    *out_len = (size_t)size;
    return buf;
}

static void replay(const char *flag, const char *events, const char *dump) {
    char cmd[512];
    remove(dump);
    snprintf(cmd, sizeof cmd, DRIVER " %s %s " CAPTURE " %s > /dev/null", flag, events, dump);
    assert(system(cmd) == 0);
}

static int same_final_screen(const char *flag) {
    replay(flag, FORGIVING, DUMP_A);
    replay(flag, REASONABLE, DUMP_B);
    size_t a_len, b_len;
    char *a = read_whole_file(DUMP_A, &a_len);
    char *b = read_whole_file(DUMP_B, &b_len);
    int same = a_len == b_len && memcmp(a, b, a_len) == 0;
    free(a);
    free(b);
    return same;
}

static void test_recording_click_picks_the_difficulty(void) {
    assert(!same_final_screen("--from-title") &&
           "the title-screen click must choose the difficulty shown in the score panel");
}

/* Temoin : sans le drapeau, les deux rejeus finissent sur le meme ecran,
 * donc l'ecart ci-dessus vient de la difficulte et non de la position du
 * clic. */
static void test_scripted_replay_keeps_the_driver_difficulty(void) {
    assert(same_final_screen(""));
}

int main(void) {
    test_recording_click_picks_the_difficulty();
    test_scripted_replay_keeps_the_driver_difficulty();
    remove(CAPTURE);
    remove(DUMP_A);
    remove(DUMP_B);
    printf("test_pico8_replay_from_title: all tests passed\n");
    return 0;
}
