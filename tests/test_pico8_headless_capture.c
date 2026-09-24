/* La capture d'ecran faite par le pilote de rejeu headless : autant de
 * frames ecrites que de frames rejouees, et les memes pixels que ceux
 * que le jeu a dessines.
 *
 * CE TEST LANCE LE VRAI PILOTE, PAS UNE COPIE. tmp/pico8_replay est
 * invoque par system() sur une fixture courte. Refaire ici la sequence
 * d'amorcage (pico8_cart_load / pico8__init / pico8_eu / boucle de
 * frames) serait une troisieme copie de cette sequence, donc un second
 * rejeu subtilement different : le test prouverait qu'un rejeu ecrit
 * les bons pixels, sans rien dire de celui que `make replay` execute.
 * Le prix -- un test qui depend d'un binaire construit et passe par le
 * shell -- est paye sciemment.
 *
 * Effet de bord assume : chaque rejeu lance ici reecrit le dump du
 * pilote avec celui de la fixture courte ; test_pico8_frames_tool le
 * reecrit aussi, et `make replay` le reecrit ensuite avec celui de
 * la session humaine (tests/fixtures/session_events.json). */

#include "../src/pico8_dump.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_screencast.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIXTURE "tests/fixtures/screencast_capture_events.json"
#define DRIVER "./tmp/pico8_replay"
/* Le chemin par defaut du dump de src/main.c, quand argv[3] manque.
 * C'est la sortie sur laquelle portent la fidelite ET la neutralite. */
#define DRIVER_DUMP "tmp/pico8_replay_dump.txt"
#define CUSTOM_DUMP "tmp/test_headless_capture_custom_dump.txt"
#define DUMP_WITNESS "temoin: ce dump ne doit pas etre reecrit\n"
#define CAPTURE "tmp/test_headless_capture.bin"
#define STDERR_LOG "tmp/test_headless_capture_stderr.txt"
/* Un chemin dont le repertoire parent n'existe pas : le cas "chemin
 * invalide", obtenu sans dependre des droits du systeme de fichiers ni
 * d'un disque plein. */
#define BAD_CAPTURE "tmp/no_such_directory_for_capture/capture.bin"

/* Le contenu entier de `path`, avec un octet nul de plus a la fin pour
 * que strstr() soit utilisable sur les sorties texte. `*out_len` est la
 * taille reelle, sans ce nul : les comparaisons de dumps passent par
 * memcmp sur cette longueur et ne dependent donc pas de lui. Tampon
 * alloue : l'appelant le libere. */
static char *read_whole_file(const char *path, size_t *out_len) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    long size = ftell(file);
    assert(size > 0);
    rewind(file);

    char *buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, file) == (size_t)size);
    fclose(file);
    buf[size] = '\0';

    *out_len = (size_t)size;
    return buf;
}

/* Le texte que pico8_dump_screen produit pour un ecran dont les pixels
 * sont ceux de `pixels`. La comparaison passe par LE VRAI dumper et non
 * par un decodeur hexadecimal ecrit ici : un test qui reconstruirait
 * lui-meme le format du dump comparerait sa propre idee de ce format a
 * celle du pilote -- deux choses qui peuvent deriver ensemble sans que
 * rien ne rougisse.
 *
 * Tampon alloue, que l'appelant libere. Il n'est PAS termine par un
 * NUL (a la difference de read_whole_file ci-dessus) : seul memcmp sur
 * `*out_len` est licite dessus. */
static char *dump_text_of_pixels(const unsigned char *pixels, size_t *out_len) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);
    for (int i = 0; i < PICO8_SCREENCAST_FRAME_PIXELS; i++) {
        pico8_poke(&mem, PICO8_ADDR_SCREEN + i, pixels[i]);
    }

    FILE *tmp = tmpfile();
    assert(tmp != NULL);
    assert(pico8_dump_screen(&mem, tmp));
    long size = ftell(tmp);
    assert(size > 0);
    rewind(tmp);

    char *buf = malloc((size_t)size);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, tmp) == (size_t)size);
    fclose(tmp);

    *out_len = (size_t)size;
    return buf;
}

/* Combien de frames la fixture contient, lu par le meme chargeur que le
 * pilote plutot qu'ecrit en dur ici. La propriete visee est "autant de
 * frames ecrites que de frames rejouees" ; une constante en dur dirait
 * seulement "huit", et cesserait de parler de rejeu le jour ou la
 * fixture change. */
static int fixture_frame_count(void) {
    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, FIXTURE));
    int count = input.frame_count;
    pico8_input_free(&input);
    return count;
}

static void test_capture_holds_exactly_the_replayed_frames(void) {
    remove(CAPTURE);
    assert(system(DRIVER " " FIXTURE " " CAPTURE " > /dev/null") == 0);

    pico8_screencast_reader_t reader;
    assert(pico8_screencast_reader_open(&reader, CAPTURE) == PICO8_SCREENCAST_OK);
    assert(pico8_screencast_frame_count(&reader) == fixture_frame_count());
    pico8_screencast_reader_close(&reader);

    remove(CAPTURE);
}

/* FIDELITE DE LA CAPTURE. La derniere frame est le seul point de
 * comparaison possible : le dump texte du pilote ne decrit que
 * celle-la. Si elle coincide, la capture ecrit bien ce que le jeu a
 * dessine, au bon moment -- ni la frame d'avant, ni un ecran a demi
 * dessine. */
static void test_last_captured_frame_matches_the_text_dump(void) {
    remove(CAPTURE);
    assert(system(DRIVER " " FIXTURE " " CAPTURE " > /dev/null") == 0);

    size_t dump_len;
    char *dump = read_whole_file(DRIVER_DUMP, &dump_len);

    pico8_screencast_reader_t reader;
    assert(pico8_screencast_reader_open(&reader, CAPTURE) == PICO8_SCREENCAST_OK);
    int last = pico8_screencast_frame_count(&reader) - 1;
    assert(last > 0);

    unsigned char last_pixels[PICO8_SCREENCAST_FRAME_PIXELS];
    assert(pico8_screencast_read_frame(&reader, last, last_pixels));

    /* L'avant-derniere frame sert de temoin, et pas de sujet : elle
     * etablit que cette fixture dessine deux images DIFFERENTES a la
     * fin. Sans cela, une capture decalee d'une frame passerait la
     * comparaison ci-dessous par accident, et ce test ne prouverait
     * plus rien du moment ou l'ecriture a lieu. */
    unsigned char previous_pixels[PICO8_SCREENCAST_FRAME_PIXELS];
    assert(pico8_screencast_read_frame(&reader, last - 1, previous_pixels));
    assert(memcmp(previous_pixels, last_pixels, sizeof(last_pixels)) != 0);
    pico8_screencast_reader_close(&reader);

    size_t captured_len;
    char *captured = dump_text_of_pixels(last_pixels, &captured_len);

    assert(captured_len == dump_len);
    assert(memcmp(captured, dump, dump_len) == 0);

    free(captured);
    free(dump);
    remove(CAPTURE);
}

/* NEUTRALITE DE LA CAPTURE : un rejeu avec capture et un rejeu sans
 * capture produisent le meme dump texte final, la garantie que
 * l'instrument n'altere pas ce qu'il mesure. Le dump final est la seule
 * observation que ce pilote publie ; s'il est identique des deux cotes,
 * alors la capture n'a rien change a ce qui a ete dessine.
 *
 * Le meme test couvre l'autre moitie de l'exigence -- le comportement
 * sans argument de capture est inchange -- puisque le rejeu sans
 * capture est ici execute tel quel et doit produire ce dump. */
static void test_capturing_changes_nothing_the_replay_produces(void) {
    remove(DRIVER_DUMP);
    assert(system(DRIVER " " FIXTURE " > /dev/null") == 0);
    size_t without_len;
    char *without = read_whole_file(DRIVER_DUMP, &without_len);

    remove(DRIVER_DUMP);
    remove(CAPTURE);
    assert(system(DRIVER " " FIXTURE " " CAPTURE " > /dev/null") == 0);
    size_t with_len;
    char *with = read_whole_file(DRIVER_DUMP, &with_len);

    assert(with_len == without_len);
    assert(memcmp(with, without, without_len) == 0);

    free(with);
    free(without);
    remove(CAPTURE);
}

/* Une capture qui ne peut pas commencer TUE le processus, et le dit.
 * C'est la politique inverse de pico8_session_recorder, et elle
 * s'inverse ici parce que personne ne joue : un fichier tronque
 * produirait silencieusement une empreinte fausse, c'est-a-dire un faux
 * rouge indiscernable d'une regression. Un pilote qui aurait continue
 * sans capturer aurait rendu 0, et `make screencast-check` aurait
 * calcule une empreinte sur le fichier d'un rejeu precedent.
 *
 * Le message est assertionne, pas seulement le code de sortie : il est
 * la seule interface humaine de cet echec, et un chemin mal tape ne se
 * corrige qu'en le relisant. */
static void test_unopenable_capture_path_is_fatal_and_names_the_path(void) {
    remove(STDERR_LOG);
    assert(system(DRIVER " " FIXTURE " " BAD_CAPTURE " > /dev/null 2> " STDERR_LOG) != 0);

    size_t log_len;
    char *log = read_whole_file(STDERR_LOG, &log_len);
    assert(strstr(log, BAD_CAPTURE) != NULL);

    free(log);
    remove(STDERR_LOG);
}

/* Des rejeux de variantes lances cote a cote ne doivent pas s'ecraser
 * leur dump : avec argv[3], le dump va la, et le chemin par defaut --
 * relu par ce test et par test_pico8_frames_tool -- garde son temoin. */
static void test_third_argument_redirects_the_text_dump(void) {
    remove(DRIVER_DUMP);
    assert(system(DRIVER " " FIXTURE " > /dev/null") == 0);
    size_t default_len;
    char *default_dump = read_whole_file(DRIVER_DUMP, &default_len);

    FILE *witness = fopen(DRIVER_DUMP, "w");
    assert(witness != NULL);
    assert(fputs(DUMP_WITNESS, witness) >= 0);
    assert(fclose(witness) == 0);

    remove(CUSTOM_DUMP);
    remove(CAPTURE);
    assert(system(DRIVER " " FIXTURE " " CAPTURE " " CUSTOM_DUMP " > /dev/null") == 0);

    size_t custom_len;
    char *custom = read_whole_file(CUSTOM_DUMP, &custom_len);
    assert(custom_len == default_len);
    assert(memcmp(custom, default_dump, default_len) == 0);

    size_t untouched_len;
    char *untouched = read_whole_file(DRIVER_DUMP, &untouched_len);
    assert(untouched_len == strlen(DUMP_WITNESS));
    assert(memcmp(untouched, DUMP_WITNESS, untouched_len) == 0);

    free(untouched);
    free(custom);
    free(default_dump);
    remove(CUSTOM_DUMP);
    remove(CAPTURE);
    remove(DRIVER_DUMP);
}

int main(void) {
    test_capture_holds_exactly_the_replayed_frames();
    test_last_captured_frame_matches_the_text_dump();
    test_capturing_changes_nothing_the_replay_produces();
    test_unopenable_capture_path_is_fatal_and_names_the_path();
    test_third_argument_redirects_the_text_dump();
    printf("test_pico8_headless_capture: all tests passed\n");
    return 0;
}
