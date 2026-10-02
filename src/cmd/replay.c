/* The headless replay driver: loads the cart's assets, boots the cart, steps it
 * one frame per events.json entry, and dumps the final framebuffer as text.
 * It asserts nothing about the game itself -- that is
 * what tests/ is for.
 *
 * argv[1], required, names the script to replay; without it the driver
 * prints its usage and exits 2.
 *
 * argv[2], optional, is where each frame's pixels are written. Absent means no
 * capture, and the driver then behaves exactly as it did before capture
 * existed -- tests/test_headless_capture.c asserts that.
 *
 * argv[3], optional, is where the text dump goes; absent means
 * tmp/pico8_replay_dump.txt. Giving it requires giving argv[2].
 *
 * Every replay starts on the title screen: the file itself clicks the
 * difficulty. The driver takes no option; an argument starting with '-'
 * gets the usage and exit 2.
 *
 * PICO8_REMPLISSAGE=<file>, optional, is where the live places of each
 * interface sort, then of each particle sort, are written after the last
 * frame, one "<sort> <count>" line each, read after flushing the exit
 * queue; then the session's maxima of each particle sort, one
 * "max_vivantes|max_octets|max_elements <sort> <n>" line each; then the
 * bytes taken in the game zone, "partie <n>", and their session maximum,
 * "max_octets partie <n>"; then the same two for the loading sub-zone,
 * "chargement <n>" and "max_octets chargement <n>". Capture, dump and exit
 * code stay the same. */

#include "p8/mem.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/cheat.h"
#include "devtools/dump.h"
#include "devtools/screencast.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "game/ui_zone.h"
#include "game/particle_zone.h"
#include "engine/game_zone.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const char *const SLOT_KIND_NAMES[SLOT_KIND_COUNT] = {"button", "modal_backdrop", "message_box", "toast", "row", "ephemerals"};
static const char *const PARTICLE_NAMES[PARTICLE_SLOT_COUNT] = {"particle", "particles", "particles_help_screen", "burst", "transit_dots"};

static void write_fill(const char *path) {
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        fprintf(stderr, "replay: cannot open the fill file '%s'\n", path);
        return;
    }
    cart_flush_exit_queue();
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        fprintf(out, "%s %d\n", SLOT_KIND_NAMES[s], ui_zone_live((slot_kind_t)s));
    }
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++) {
        fprintf(out, "%s %d\n", PARTICLE_NAMES[s], particle_zone_live((particle_slot_kind_t)s));
    }
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++) {
        fprintf(out, "max_live %s %d\n", PARTICLE_NAMES[s],
                particle_zone_max_live((particle_slot_kind_t)s));
    }
    for (int s = PARTICLE_SLOT_LIST; s < PARTICLE_SLOT_COUNT; s++) {
        fprintf(out, "max_bytes %s %lu\n", PARTICLE_NAMES[s],
                (unsigned long)particle_zone_max_bytes((particle_slot_kind_t)s));
    }
    for (int s = PARTICLE_SLOT_LIST; s < PARTICLE_SLOT_COUNT; s++) {
        if (s == PARTICLE_SLOT_BURST) continue;
        fprintf(out, "max_elements %s %d\n", PARTICLE_NAMES[s],
                particle_zone_max_elements((particle_slot_kind_t)s));
    }
    fprintf(out, "game %lu\n", (unsigned long)game_zone_used());
    fprintf(out, "max_bytes game %lu\n", (unsigned long)game_zone_max());
    fprintf(out, "load %lu\n", (unsigned long)load_zone_used());
    fprintf(out, "max_bytes load %lu\n", (unsigned long)game_zone_load_zone_max());
    fclose(out);
}

int main(int argc, char **argv) {
    if (argc < 2 || argv[1][0] == '-') {
        fprintf(stderr, "usage: replay <events.json> [<capture.bin> [<dump.txt>]]\n");
        return 2;
    }
    const char *events_path = argv[1];
    const char *capture_path = (argc > 2) ? argv[2] : NULL;
    const char *dump_path = (argc > 3) ? argv[3] : "tmp/replay_dump.txt";

    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_graphics_t gfx;
    p8_gfx_init(&gfx);

    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));

    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, events_path));

    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    cart_load();
    cart_init();

    /* Un echec tue le processus : une capture absente ou tronquee
     * produirait une empreinte fausse, indiscernable d'une regression.
     * fprintf plutot qu'un assert, pour que le message nomme le chemin. */
    screencast_writer_t capture = { NULL };
    if (capture_path && !screencast_writer_open(&capture, capture_path)) {
        fprintf(stderr, "replay: cannot open the capture file '%s'\n",
                capture_path);
        return 1;
    }

    int frame_count = input.frame_count;
    cheat_t cheat;
    cheat_init(&cheat);
    for (int i = 0; i < frame_count; i++) {
        dispatch_context_set_input(&input);
        cheat_step(&cheat, &input);
        cart_update60();
        cart_draw();
        /* APRES pico8__draw() et avant la frame suivante : c'est le seul
         * instant ou la memoire ecran tient la frame i complete. En tete
         * de boucle, la capture entiere serait decalee d'une frame --
         * tests/test_headless_capture.c le detecte en comparant la
         * derniere frame capturee au dump texte. */
        if (capture_path && !screencast_write_frame(&capture, &mem)) {
            fprintf(stderr, "replay: cannot write frame %d to '%s'\n",
                    i, capture_path);
            screencast_writer_close(&capture);
            return 1;
        }
        if (i < frame_count - 1) {
            p8_input_step(&input);
        }
    }

    /* La fermeture est verifiee au meme titre que les ecritures : fclose
     * est le dernier endroit ou une ecriture tamponnee peut echouer.
     * Elle a lieu AVANT le dump texte, et un echec renonce donc a
     * l'ecrire : un dump frais ferait passer pour
     * normale une execution dont la capture est perdue. */
    if (capture_path && !screencast_writer_close(&capture)) {
        fprintf(stderr, "replay: cannot close the capture file '%s'\n",
                capture_path);
        return 1;
    }

    FILE *dump_out = fopen(dump_path, "w");
    assert(dump_out != NULL);
    int dump_ok = dump_screen(&mem, dump_out);
    fclose(dump_out);
    assert(dump_ok);

    const char *fill_path = getenv("REPLAY_FILL");
    if (fill_path != NULL) {
        write_fill(fill_path);
    }

    p8_input_free(&input);

    printf("replay: %d frame(s) run against %s; framebuffer dumped to %s\n",
           frame_count, events_path, dump_path);
    if (capture_path) {
        printf("replay: %d frame(s) captured to %s\n", frame_count, capture_path);
    }
    return 0;
}
