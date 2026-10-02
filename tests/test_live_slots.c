/* Places vivantes des zones interface et particules, sur rejeu d'un e2e de tests/e2e/
 * pilote comme main.c : apres une session, chaque sorte revient a son
 * niveau de depart, et aucune ne monte pendant une phase sans action.
 * Chaque compte est lu apres un vidage de la file de sortie. Un rejeu par
 * processus enfant : le cart n'est charge qu'une fois par processus. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "p8/assets.h"
#include "game/background.h"
#include "game/cart.h"
#include "game/cheat.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "game/main_economy.h"
#include "game/game_data.h"
#include "game/gen.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "game/layout.h"
#include "game/map_tables.h"
#include "p8/mem.h"
#include "game/mouse.h"
#include "game/particle.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/probe.h"
#include "game/structure_icon.h"
#include "game/score_panel.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "game/trade.h"
#include "game/ui.h"
#include "game/wormhole.h"
#include "engine/allocator.h"
#include "engine/frame_zone.h"
#include "game/ui_zone.h"
#include "game/particle_zone.h"
#include "engine/game_zone.h"

#define BEFORE_F0 (-1)
#define MAX_READINGS 4

static const char *const NAMES[SLOT_KIND_COUNT] = {"button", "modal_backdrop", "message_box", "toast", "rangee", "le"};

typedef struct {
    int frame;
    int live[SLOT_KIND_COUNT];
    int particle;
    int particles;
    int particles_help_screen;
    int burst;
    int transit_dots;
    size_t game_zone;
} reading_t;

static void take_reading(reading_t *r, int frame) {
    pico8_cart_vider_file_sortie();
    r->frame = frame;
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        r->live[s] = ui_zone_live((slot_kind_t)s);
    }
    r->particle = particle_zone_live(PARTICLE_SLOT_PARTICLE);
    r->particles = particle_zone_live(PARTICLE_SLOT_LIST);
    r->particles_help_screen = particle_zone_live(PARTICLE_SLOT_HELP_LIST);
    r->burst = particle_zone_live(PARTICLE_SLOT_BURST);
    r->transit_dots = particle_zone_live(PARTICLE_SLOT_TRANSIT_DOT);
    r->game_zone = game_zone_used();
}

/* Particules et listes tenues par les emetteurs inscrits dans `hv`, eclats
 * inscrits, trainees des slipways de `cg.jc` : ce que les comptes de la
 * zone particules doivent dire. */
typedef struct {
    int particle;
    int lists;
    int bursts;
    int trails;
} registered_t;

static registered_t count_registered(void) {
    registered_t n = {0, 0, 0, 0};
    const entity_list_t *hv = live_entities();
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        const entity_t *e = hv->items[i];
        if (e->emitter != NULL && e->emitter->particles != NULL) {
            n.lists++;
            n.particle += e->emitter->particles->count;
        }
        if (e->class == pico8_ga_struct_class()) n.bursts++;
    }
    const entity_list_t *jc = entities_by_tag("slipway");
    for (int i = 0; jc != NULL && i < jc->count; i++) {
        const slipway_t *s = (const slipway_t *)jc->items[i];
        if (s->transit_dots != NULL) n.trails += s->transit_dots->count;
    }
    return n;
}

/* Lu apres un vidage : une particule, une liste ou un eclat perdu le casse
 * a la frame meme ou il l'est. */
static void check_invariant(const char *e2e, int frame) {
    reading_t r;
    take_reading(&r, frame);
    registered_t n = count_registered();
    if (r.particle != n.particle || r.particles + r.particles_help_screen != n.lists || r.burst != n.bursts || r.transit_dots != n.trails) {
        fprintf(stderr, "  %s f%d : particule=%d (inscrites %d) particles+particles_help_screen=%d (listes %d) burst=%d (eclats %d) transit_dots=%d (trainees %d)\n",
                e2e, frame, r.particle, n.particle, r.particles + r.particles_help_screen, n.lists, r.burst, n.bursts, r.transit_dots, n.trails);
        assert(0 && "une particule, une liste qy, un eclat ou une trainee vit hors des emetteurs inscrits ou de cg.jc");
    }
}

static void print_reading(const char *e2e, const reading_t *r) {
    printf("  %s f%d :", e2e, r->frame);
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        printf(" %s=%d", NAMES[s], r->live[s]);
    }
    printf(" particule=%d particles=%d particles_help_screen=%d burst=%d partie=%lu\n", r->particle, r->particles, r->particles_help_screen, r->burst,
           (unsigned long)r->game_zone);
}

/* Charge le cart pour rejouer `e2e` (NULL : aucune entree ; un chemin s'il
 * contient un « / ») et rend ses entrees, statiques. */
static p8_input_t *load_e2e(const char *e2e) {
    static p8_memory_t mem;
    static p8_graphics_t gfx;
    static p8_input_t input;
    char path[256];

    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    if (e2e != NULL) {
        snprintf(path, sizeof path, strchr(e2e, '/') != NULL ? "%s" : "tests/e2e/%s.json", e2e);
        assert(pico8_input_load(&input, path));
    }
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);
    pico8_cart_load();
    pico8__init();
    return &input;
}

/* Releve les comptes apres chaque frame de `frames` (AVANT_F0 : apres
 * _init, avant la frame 0), et rend la derniere frame jouee. */
static void replay_e2e(const char *e2e, const int *frames, int n, reading_t *out) {
    p8_input_t *input = load_e2e(e2e);
    int k = 0;

    if (k < n && frames[k] == BEFORE_F0) {
        take_reading(&out[k], BEFORE_F0);
        print_reading(e2e, &out[k]);
        k++;
    }
    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (int i = 0; i < input->frame_count && k < n; i++) {
        dispatch_context_set_input(input);
        pico8_cheat_step(&cheat, input);
        pico8__update60();
        pico8__draw();
        if (frames[k] == i) {
            take_reading(&out[k], i);
            print_reading(e2e, &out[k]);
            k++;
        }
        if (i < input->frame_count - 1) p8_input_step(input);
    }
    assert(k == n && "une frame de releve est hors du rejeu");
}

/* Rejoue `e2e` en entier, l'invariant verifie a chaque frame. */
static void replay_under_invariant(const char *e2e) {
    p8_input_t *input = load_e2e(e2e);
    check_invariant(e2e, BEFORE_F0);
    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (int i = 0; i < input->frame_count; i++) {
        dispatch_context_set_input(input);
        pico8_cheat_step(&cheat, input);
        pico8__update60();
        pico8__draw();
        check_invariant(e2e, i);
        if (i < input->frame_count - 1) p8_input_step(input);
    }
}

/* Tout sauf `particule`, qu'un emetteur actif fait varier sans rien
 * perdre : l'invariant le suit frame par frame. */
static int same_counts(const reading_t *a, const reading_t *b) {
    return memcmp(a->live, b->live, sizeof a->live) == 0 && a->particles == b->particles && a->particles_help_screen == b->particles_help_screen &&
           a->burst == b->burst;
}

static int none_rises(const reading_t *a, const reading_t *b) {
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        if (b->live[s] > a->live[s]) return 0;
    }
    return 1;
}

static void in_child(void (*test)(void)) {
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0 && "un releve de places vivantes a echoue");
}

#define ASAN_LOG "tmp/test_places_vivantes_asan.txt"

static char *read_file(const char *path, size_t *len);

/* L'enfant doit mourir par ASan en relisant un bloc rendu. */
static void in_child_dying_by_asan(void (*test)(void)) {
    fflush(stdout);
    remove(ASAN_LOG);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        assert(freopen(ASAN_LOG, "w", stderr) != NULL);
        test();
        _exit(0);
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    size_t len;
    char *err = read_file(ASAN_LOG, &len);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        fprintf(stderr, "%s", err);
        assert(0 && "le bloc rendu se relit : la generation n'a pas ete rendue");
    }
    if (strstr(err, "use-after-poison") == NULL) {
        fprintf(stderr, "%s", err);
        assert(0 && "attendu use-after-poison sur le bloc rendu");
    }
    printf("  pause_restart f13 : le bloc relu plante (use-after-poison)\n");
    free(err);
    remove(ASAN_LOG);
}

static char g_e2e[128];

static void invariant_on_current_e2e(void) {
    replay_under_invariant(g_e2e);
}

/* Chaque frame de chaque e2e de tests/e2e/ ; rend le nombre d'e2e. */
static int invariant_on_all_e2e(void) {
    DIR *dir = opendir("tests/e2e");
    struct dirent *ent;
    int n = 0;
    assert(dir != NULL);
    while ((ent = readdir(dir)) != NULL) {
        size_t len = strlen(ent->d_name);
        if (len <= 5 || strcmp(ent->d_name + len - 5, ".json") != 0) continue;
        assert(len - 5 < sizeof g_e2e);
        memcpy(g_e2e, ent->d_name, len - 5);
        g_e2e[len - 5] = '\0';
        in_child(invariant_on_current_e2e);
        n++;
    }
    closedir(dir);
    return n;
}

/* f7 : « restart » ; f11 : ecran titre, avant le clic de f12. */
static void pause_restart_returns_to_title(void) {
    const int frames[] = {BEFORE_F0, 11};
    reading_t r[2];
    replay_e2e("pause_restart", frames, 2, r);
    assert(same_counts(&r[0], &r[1]) && "jl() doit rendre toutes les places qu'il abandonne");
    assert(r[0].particles_help_screen == 1 && r[0].particles == 0 && r[0].burst == 0);
    /* Aucune particule de l'aide ne meurt avant 115 frames : 5 par draw,
     * aucun draw avant f0, ceux de f7 a f11 pour l'aide du « restart ». */
    assert(r[0].particle == 0 && r[1].particle == 5 * 5 && "jl() doit rendre les particules de l'aide");
}

/* volatile : la lecture d'une sentinelle rendue ne doit pas etre retiree
 * par l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char g_spend;

static p8_input_t *g_input;
static pico8_cheat_t g_cheat;
static int g_frame;

/* Sans `relever` : son vidage de la file rendrait la generation lui-meme,
 * a la place de l'update60 de la frame qui suit le clic. */
static void play_until(int last) {
    for (; g_frame <= last && g_frame < g_input->frame_count; g_frame++) {
        dispatch_context_set_input(g_input);
        pico8_cheat_step(&g_cheat, g_input);
        pico8__update60();
        pico8__draw();
        if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    }
    assert(g_frame == last + 1 && "une frame de sentinelle est hors du rejeu");
}

static void start_e2e(const char *e2e) {
    g_input = load_e2e(e2e);
    pico8_cheat_init(&g_cheat);
    g_frame = 0;
}

static unsigned char *sentinel(void) {
    unsigned char *s = allocator_alloc(game_zone(), 64);
    for (int i = 0; i < 64; i++) s[i] = (unsigned char)i;
    return s;
}

static void check_sentinel(const unsigned char *s) {
    for (int i = 0; i < 64; i++) assert(s[i] == i && "la sentinelle de la zone partie a change");
}

/* f12 : clic « challenging », donc `eu` : la zone bascule, la generation
 * de la partie quittee (et du titre de f7) vit jusqu'a la fin de la frame. */
static void pause_restart_keeps_quit_game_during_click(void) {
    start_e2e("pause_restart");
    play_until(11);
    assert(!game_zone_pending());
    unsigned char *s = sentinel();
    size_t grab_arena = game_zone_used();
    play_until(12);
    assert(game_zone_pending() && "le clic de f12 fait basculer la zone partie");
    check_sentinel(s);
    assert(game_zone_used() >= grab_arena && "les octets pris comptent la partie quittee jusqu'a son rendu");
    printf("  pause_restart f12 : partie=%lu, sentinelle lue\n", (unsigned long)game_zone_used());
}

/* f13 : l'update60 vide la file puis rend la generation quittee. */
static void pause_restart_releases_quit_game_next_frame(void) {
    start_e2e("pause_restart");
    play_until(11);
    unsigned char *s = sentinel();
    play_until(13);
    g_spend = s[63];
}

static const planet_t *planet_of_first_game(void) {
    start_e2e("pause_restart");
    play_until(6);
    const entity_list_t *jo = entities_by_tag("planet");
    assert(jo != NULL && jo->count > 0 && "la premiere partie a des planetes");
    return (const planet_t *)jo->items[0];
}

/* f12 : le clic bascule ; les planetes de la partie quittee vivent encore. */
static void pause_restart_keeps_quit_planets_during_click(void) {
    const planet_t *p = planet_of_first_game();
    play_until(12);
    assert(game_zone_pending());
    g_spend = (unsigned char)p->has_production_bonus;
}

/* f13 : elles meurent avec leur generation. */
static void pause_restart_releases_quit_planets(void) {
    const planet_t *p = planet_of_first_game();
    play_until(13);
    g_spend = (unsigned char)p->has_production_bonus;
}

/* f11 : l'ecran titre lit encore l'economie de la partie quittee. */
static const economy_t *economy_of_first_game(void) {
    start_e2e("pause_restart");
    play_until(11);
    const economy_t *fm = main_economy_typed();
    assert(fm != NULL && "entre _init et eu, fm est l'economie de la partie quittee");
    return fm;
}

/* f12 : `eu` la remplace, sa generation vit jusqu'a la fin de la frame. */
static void pause_restart_keeps_quit_economy_during_click(void) {
    const economy_t *fm = economy_of_first_game();
    play_until(12);
    assert(game_zone_pending());
    assert(main_economy_typed() != fm && "le clic de f12 fait une nouvelle economie");
    g_spend = (unsigned char)fm->credits;
}

/* f13 : elle meurt avec sa generation. */
static void pause_restart_releases_quit_economy(void) {
    const economy_t *fm = economy_of_first_game();
    play_until(13);
    g_spend = (unsigned char)fm->credits;
}

/* f6 : `hv` tient les icones de la premiere partie ; le `jl()` de `_init`
 * les en retire avant f11. */
static const structure_icon_t *icon_of_first_game(void) {
    start_e2e("pause_restart");
    play_until(6);
    const entity_list_t *hv = live_entities();
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        if (hv->items[i]->class == structure_icon_class()) return (const structure_icon_t *)hv->items[i];
    }
    assert(0 && "la premiere partie a des icones dans hv");
    return NULL;
}

/* f12 : `eu` en fait de nouvelles, sa generation vit jusqu'a la fin de la frame. */
static void pause_restart_keeps_quit_icons_during_click(void) {
    const structure_icon_t *ie = icon_of_first_game();
    play_until(12);
    assert(game_zone_pending());
    g_spend = (unsigned char)ie->is_revealed;
}

/* f13 : elles meurent avec leur generation. */
static void pause_restart_releases_quit_icons(void) {
    const structure_icon_t *ie = icon_of_first_game();
    play_until(13);
    g_spend = (unsigned char)ie->is_revealed;
}

/* Le `fv` de `hv`, que fait `_init`. */
static const probe_t *help_screen_in_live_entities(void) {
    const entity_list_t *hv = live_entities();
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        if (hv->items[i]->class == pico8_fv_struct_class()) return (const probe_t *)hv->items[i];
    }
    assert(0 && "le titre a son aide dans hv");
    return NULL;
}

static const background_t *background_in_live_entities(void);

enum { SINGLETON_MAIN_CAMERA, SINGLETON_MAIN_MOUSE, SINGLETON_MAIN_UI, SINGLETON_BACKGROUND, SINGLETON_MAIN_SCORE_PANEL, SINGLETON_HELP_SCREEN, SINGLETON_COUNT };
static const char *const NAMES_OF_SINGLETONS[SINGLETON_COUNT] = {"main_camera", "main_mouse", "main_ui", "background", "main_score_panel", "help_screen"};

/* f6 : le `bg` du eu de la premiere partie ; f11 : `mo`, `je`, `ok` et
 * l'aide `fv` du `_init` de f7, `ju` du eu. Tous dans la generation de la
 * premiere partie. */
static void singletons_of_quit_game(const void *out[SINGLETON_COUNT]) {
    start_e2e("pause_restart");
    play_until(6);
    out[SINGLETON_BACKGROUND] = background_in_live_entities();
    play_until(11);
    out[SINGLETON_MAIN_CAMERA] = main_camera_typed();
    out[SINGLETON_MAIN_MOUSE] = main_mouse_typed();
    out[SINGLETON_MAIN_UI] = main_ui();
    out[SINGLETON_MAIN_SCORE_PANEL] = pico8_ju();
    out[SINGLETON_HELP_SCREEN] = help_screen_in_live_entities();
    for (int k = 0; k < SINGLETON_COUNT; k++) assert(out[k] != NULL && "un singleton manque a f11");
}

/* f12 : le clic bascule ; le `ok` quitte finit son lj apres le jl() de `eu`. */
static void pause_restart_keeps_quit_singletons_during_click(void) {
    const void *s[SINGLETON_COUNT];
    singletons_of_quit_game(s);
    play_until(12);
    assert(game_zone_pending());
    assert(main_camera_typed() != s[SINGLETON_MAIN_CAMERA] && main_ui() != s[SINGLETON_MAIN_UI] && pico8_ju() != s[SINGLETON_MAIN_SCORE_PANEL] &&
           "le clic de f12 fait de nouveaux singletons");
    for (int k = 0; k < SINGLETON_COUNT; k++) g_spend = *(const unsigned char *)s[k];
}

static int g_singleton;

/* f13 : chacun meurt avec sa generation. */
static void pause_restart_releases_quit_singleton(void) {
    const void *s[SINGLETON_COUNT];
    singletons_of_quit_game(s);
    play_until(13);
    g_spend = *(const unsigned char *)s[g_singleton];
}

/* f7 : « restart » (`_init`) ne bascule pas. */
static void pause_restart_keeps_game_across_restart(void) {
    start_e2e("pause_restart");
    play_until(6);
    unsigned char *s = sentinel();
    play_until(11);
    assert(!game_zone_pending() && "« restart » ne fait pas basculer la zone partie");
    check_sentinel(s);
}

/* Octets que le bw de chaque eu prend dans sa generation : jo.oc, sa
 * source et jo.ir. */
static size_t populate_galaxy_states(void) {
    return (2 * ZONE_CELLS(sizeof(random_walk_t)) + ZONE_CELLS(sizeof(weighted_picker_t))) *
           sizeof(zone_cell_t);
}

/* Octets qu'une liste a pris dans sa generation depuis un jl() : les
 * capacites 4, 8, ... jusqu'a la sienne, car chaque agrandissement copie et
 * laisse l'ancien bloc mort jusqu'au rendu. NULL (un `cg` jamais vu) : rien. */
static size_t registry_bytes(const entity_list_t *list) {
    size_t bytes = 0;
    for (int k = 4; list != NULL && k <= list->capacity; k *= 2) {
        bytes += ZONE_CELLS((size_t)k * sizeof(entity_t *)) * sizeof(zone_cell_t);
    }
    return bytes;
}

static size_t registries_bytes(void) {
    static const char *const CV[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
    static const char *const CG[] = {"structure", "ascension", "slipgate", "probe", "message_box", "slipway", "wormhole", "planet", "node", "targeting"};
    size_t r = registry_bytes(live_entities());
    for (size_t i = 0; i < sizeof CV / sizeof CV[0]; i++) r += registry_bytes(entities_by_message(CV[i]));
    for (size_t i = 0; i < sizeof CG / sizeof CG[0]; i++) r += registry_bytes(entities_by_tag(CG[i]));
    return r;
}

static size_t block_bytes(size_t size) {
    return ZONE_CELLS(size) * sizeof(zone_cell_t);
}

/* Octets d'une liste de planete faite dans la generation : son conteneur,
 * puis ses blocs comme un registre. */
static size_t planet_list_bytes(const entity_list_t *list) {
    if (list == NULL) return 0;
    return block_bytes(sizeof *list) + registry_bytes(list);
}

/* Empreinte `P` des planetes de `cg.jo` : structure, `p`, `ra`, identifiant
 * et listes `oe`, `by`, `cc`. Sans `jy` ni copies de besoins, que les
 * planetes de `bw` n'ont pas avant une action. */
static size_t planets_bytes(void) {
    const entity_list_t *jo = entities_by_tag("planet");
    size_t bytes = 0;
    for (int i = 0; jo != NULL && i < jo->count; i++) {
        const planet_t *p = (const planet_t *)jo->items[i];
        assert(p->needs == NULL && "une planete a des besoins, que P ne compte pas");
        bytes += block_bytes(sizeof *p) + block_bytes(sizeof *p->map_pos) + block_bytes(sizeof *p->pos) +
                  block_bytes(strlen(p->planet_id) + 1) + planet_list_bytes(p->slipways) +
                  planet_list_bytes(p->outgoing_routes) + planet_list_bytes(p->incoming_routes);
    }
    return bytes;
}

/* Octets de l'economie `fm` faite par `eu` : sa structure et le conteneur
 * `invented`, sans elements tant que rien n'est invente. */
static size_t economy_bytes(void) {
    const economy_t *fm = main_economy_typed();
    assert(fm != NULL && fm->invented != NULL && "eu a fait l'economie de la partie");
    assert(fm->invented->count == 0 && fm->invented->items == NULL && "une invention, que l'economie ne compte pas");
    return block_bytes(sizeof *fm) + block_bytes(sizeof *fm->invented);
}

/* Octets des sept icones `ie` de `hv`, dans l'ordre de `dr` : structure,
 * bulle `jb` refaite dans une arene de la zone frame, et la boite de `ie.gy`
 * de chaque icone revelee. */
static size_t icons_bytes(void) {
    const entity_list_t *hv = live_entities();
    const allocator_t *mesure = arena_take(frame_zone(), "mesure des bulles", 16384);
    const size_t head = arena_used(mesure);
    size_t bytes = 0;
    int k = 0;
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        const structure_icon_t *ie = (const structure_icon_t *)hv->items[i];
        if (ie->header.class != structure_icon_class()) continue;
        assert(k < ICON_TOOLTIPS_COUNT && "plus d'icones que de bulles dans dr");
        (void)tooltip_new(mesure, &icon_tooltips_table[k]);
        bytes += block_bytes(sizeof *ie);
        if (ie->has_is_revealed && ie->is_revealed) bytes += block_bytes(sizeof(vec_t));
        k++;
    }
    assert(k == ICON_TOOLTIPS_COUNT && "eu fait une icone par bulle de dr");
    return bytes + arena_used(mesure) - head;
}

/* Terme « recherche et economie » de la generation : ce que `eu` fait pour
 * `fm` et les icones. */
static size_t research_and_economy_bytes(void) {
    return economy_bytes() + icons_bytes();
}

/* Le `bg` de `hv`, que `eu` fait seul. */
static const background_t *background_in_live_entities(void) {
    const entity_list_t *hv = live_entities();
    const background_t *bg = NULL;
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        if (hv->items[i]->class != background_class()) continue;
        assert(bg == NULL && "hv tient deux bg");
        bg = (const background_t *)hv->items[i];
    }
    assert(bg != NULL && "eu a fait le bg de la partie");
    return bg;
}

/* Terme « singletons » : `mo`, `je`, `ok` du jl() de `eu`, son `bg` et `ju`. */
static size_t singletons_bytes(void) {
    assert(main_camera_typed() != NULL && main_mouse_typed() != NULL && main_ui() != NULL && pico8_ju() != NULL);
    (void)background_in_live_entities();
    return block_bytes(sizeof(camera_t)) + block_bytes(sizeof(mouse_t)) +
           block_bytes(sizeof(ui_t)) + block_bytes(sizeof(background_t)) +
           block_bytes(sizeof(pico8_pj_t));
}

/* Apres le rendu de f13, la generation ne tient que ce qu'a fait le eu de
 * f12 et ce qui a grandi depuis : les etats de bw, les registres, blocs
 * morts compris, ses planetes, son economie et ses singletons. */
static void pause_restart_bounds_dead_registry_blocks(void) {
    const size_t g = populate_galaxy_states();
    start_e2e("pause_restart");
    play_until(13);
    const size_t r = registries_bytes();
    const size_t p = planets_bytes();
    printf("  pause_restart f13 : partie=%lu, etats de bw=%lu, registres=%lu, planetes=%lu\n",
           (unsigned long)game_zone_used(), (unsigned long)g, (unsigned long)r, (unsigned long)p);
    assert(game_zone_used() == g + r + p + research_and_economy_bytes() + singletons_bytes() &&
           "les registres prennent 2C - 4 pointeurs, et rien d'autre n'entre");
}

/* Seul le chargement remplit la sous-zone chargement : deux parties n'y
 * ajoutent rien. */
static void pause_restart_leaves_load_zone_alone(void) {
    start_e2e("pause_restart");
    play_until(1);
    const size_t f1 = load_zone_used();
    assert(f1 > 0 && "gb est dans la sous-zone chargement");
    play_until(g_input->frame_count - 1);
    printf("  pause_restart : chargement=%lu a f1 et en fin de rejeu\n", (unsigned long)f1);
    assert(load_zone_used() == f1 && "une partie prend dans la sous-zone chargement");
    assert(game_zone_load_zone_max() == f1);
}

/* f12 : le bw de la nouvelle partie prend ses etats a cote de ceux de la
 * partie quittee, rendus a f13. */
static void pause_restart_counts_populate_galaxy_states(void) {
    const size_t g = populate_galaxy_states();
    start_e2e("pause_restart");
    play_until(11);
    const size_t f11 = game_zone_used();
    assert(f11 >= g && "une partie : une generation d'etats de bw");
    play_until(12);
    const size_t f12 = game_zone_used();
    assert(f12 >= f11 + g && "les etats des deux parties vivent jusqu'au rendu");
    play_until(13);
    assert(game_zone_used() == g + registries_bytes() + planets_bytes() + research_and_economy_bytes() +
                                             singletons_bytes() &&
           "le rendu de f13 laisse les etats, les registres, les planetes, l'economie et les singletons de la nouvelle partie");
    assert(game_zone_max() >= f12);
    printf("  pause_restart f11/f12/f13 : partie=%lu/%lu/%lu, max=%lu\n", (unsigned long)f11, (unsigned long)f12,
           (unsigned long)game_zone_used(), (unsigned long)game_zone_max());
}

/* Octets de la generation hors registres, que `hv` peut faire grandir en
 * inscrivant des entites. */
static size_t game_without_registries(void) {
    return game_zone_used() - registries_bytes();
}

/* f3-51 : 25 sauts d'annee, donc autant de `bd`, qui ne doivent pas faire
 * grandir la generation : `cc` est videe en place. */
static void game_end_final_score_rebuild_does_not_grow_game(void) {
    start_e2e("game_end_final_score");
    play_until(2);
    const size_t f2 = game_without_registries();
    play_until(51);
    const size_t f51 = game_without_registries();
    printf("  game_end_final_score f2/f51 : partie - registres = %lu/%lu\n", (unsigned long)f2, (unsigned long)f51);
    assert(f2 == f51 && "un bd fait grandir la generation");
}

/* f140-151 : saut d'annee de f140 sur sept slipways poses, donc `jo:ib`,
 * `jo:cn` et `nr:cn`, qui reproposent des routes connues. Le `bd` de f139
 * garde, lui, la route du slipway pose a f136. Le slipway refuse du clic
 * de f142 prend une arene, que celui de f148 reprend. */
static void trade_rich_mine_route_rebuild_does_not_grow_game(void) {
    start_e2e("trade_rich_mine");
    play_until(139);
    const size_t f139 = game_without_registries();
    play_until(151);
    const size_t f151 = game_without_registries();
    printf("  trade_rich_mine f139/f151 : partie - registres = %lu/%lu\n", (unsigned long)f139,
           (unsigned long)f151);
    assert(f151 == f139 + PLANET_GRAB_ARENA_BYTES &&
           "un bd sur des routes fait grandir la generation, ou le refus de f148 ne reprend pas l'arene de f142");
}

static const planet_t *planet_with_id(const char *id) {
    const entity_list_t *np = entities_by_tag("node");
    for (int i = 0; np != NULL && i < np->count; i++) {
        const planet_t *p = (const planet_t *)np->items[i];
        if (p->planet_id != NULL && strcmp(p->planet_id, id) == 0) return p;
    }
    assert(0 && "planete introuvable dans cg.np");
    return NULL;
}

/* Une route de `by` gardee par le `bd` de f139 se relit a f151, apres douze
 * remises a zero de la frame ou ses candidates sont faites. */
static void trade_rich_mine_kept_route_survives_frame(void) {
    entity_t *lp[16];
    entity_t *rl[16];
    start_e2e("trade_rich_mine");
    play_until(139);
    const entity_list_t *by = planet_with_id("p27")->outgoing_routes;
    assert(by != NULL && by->count > 0 && "f139 : p27 garde une route");
    const trade_route_t *t = (const trade_route_t *)by->items[by->count - 1];
    const int nlp = t->planets->count;
    const int nrl = t->slipways->count;
    assert(nlp <= 16 && nrl <= 16);
    memcpy(lp, t->planets->items, (size_t)nlp * sizeof *lp);
    memcpy(rl, t->slipways->items, (size_t)nrl * sizeof *rl);
    play_until(151);
    assert(t->planets->count == nlp && memcmp(lp, t->planets->items, (size_t)nlp * sizeof *lp) == 0);
    assert(t->slipways->count == nrl && memcmp(rl, t->slipways->items, (size_t)nrl * sizeof *rl) == 0);
    printf("  trade_rich_mine : route de p27 (%d planetes, %d slipways) relue a f151\n", nlp, nrl);
}

/* Les trainees encore vivantes en fin de partie reviennent a la zone
 * particules par le `rendre` des slipways, au chargement suivant. */
static void trade_rich_mine_releases_trails_on_reload(void) {
    start_e2e("trade_rich_mine");
    play_until(g_input->frame_count - 1);
    const int end = particle_zone_live(PARTICLE_SLOT_TRANSIT_DOT);
    pico8_cart_load();
    const int after = particle_zone_live(PARTICLE_SLOT_TRANSIT_DOT);
    printf("  trade_rich_mine : transit_dots %d en fin de rejeu, %d apres rechargement (max %d, liste max %d)\n", end, after,
           particle_zone_max_live(PARTICLE_SLOT_TRANSIT_DOT),
           particle_zone_max_elements(PARTICLE_SLOT_TRANSIT_DOT));
    assert(end > 0 && "trade_rich_mine finit avec des trainees vivantes");
    assert(after == 0 && "le rendre du slipway rend ses trainees");
}

#define GRAB_ARENAS "tests/fixtures/grab_arenas_refused.json"

/* Fixture PRISES : f0-20 d'empty_planet_fade ; puis des prises de 7 frames
 * (pression sur l'icone (122,124), glisse, relache, trois d'attente) :
 * vingt refusees sur p34 (f21-160), une posee (f161-167), une refusee
 * (f168-174) ; « restart » et nouvelle partie (clic f185) ; une prise
 * (f189-195). */
static void refused_grab_arenas_do_not_grow_game(void) {
    start_e2e(GRAB_ARENAS);
    play_until(27);
    const size_t a = game_without_registries();
    play_until(160);
    const size_t twenty = game_without_registries();
    printf("  prises refusees 1/20 : partie - registres = %lu/%lu\n", (unsigned long)a, (unsigned long)twenty);
    assert(a == twenty && "une prise refusee laisse sa structure dans la generation");
}

/* La structure posee de f161-164, seule planete « s… » de `cg.jo`. */
static const planet_t *placed_grab_arena(void) {
    const entity_list_t *jo = entities_by_tag("planet");
    const planet_t *placed = NULL;
    for (int i = 0; jo != NULL && i < jo->count; i++) {
        const planet_t *p = (const planet_t *)jo->items[i];
        if (p->planet_id[0] != 's') continue;
        assert(placed == NULL && "une seule prise a ete posee");
        placed = p;
    }
    assert(placed != NULL && strcmp(placed->header.state, "settled") == 0 && "la prise de f161-164 n'a pas ete posee");
    return placed;
}

/* Le `bd` de la pose fait `by` et `cc` dans la generation, hors de l'arene. */
static void placed_grab_arena_reuses_dead_wormhole_arena(void) {
    start_e2e(GRAB_ARENAS);
    play_until(160);
    const size_t before = game_without_registries();
    play_until(174);
    const size_t after = game_without_registries();
    const planet_t *placed = placed_grab_arena();
    const size_t lists = planet_list_bytes(placed->outgoing_routes) + planet_list_bytes(placed->incoming_routes);
    printf("  prise posee : partie - registres = %lu -> %lu (outgoing_routes/incoming_routes %lu)\n", (unsigned long)before,
           (unsigned long)after, (unsigned long)lists);
    assert(after == before + lists &&
           "la prise posee reprend l'arene libre du mz de f4 (rendu a f48), et ne prend que by et cc en plus");
}

/* Une frame de rejeu ; sans `dessiner`, aucun `kj` ne remet `fo` a nil. */
static void play_one_frame(int draw) {
    dispatch_context_set_input(g_input);
    pico8_cheat_step(&g_cheat, g_input);
    pico8__update60();
    if (draw) pico8__draw();
    if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    g_frame++;
}

static planet_t *planet_targeted_by(const entity_t *held) {
    const entity_list_t *np = entities_by_tag("node");
    for (int i = 0; np != NULL && i < np->count; i++) {
        planet_t *p = (planet_t *)np->items[i];
        if (p->targeted_by == held) return p;
    }
    return NULL;
}

/* f23 : la structure tenue sur p34 y accroche son `fo` ; p34 passe hors
 * ecran avant le draw, son `kj` sort sans remettre `fo` a nil. Refusee a
 * f24, la structure est rendue au vidage de f26, et la pression de f28
 * reprend son arene : `fo` ne doit pas viser la nouvelle structure. */
static void offscreen_planet_forgets_refused_grab_arena(void) {
    static vec_t far;
    start_e2e(GRAB_ARENAS);
    play_until(22);
    dispatch_context_set_input(g_input);
    pico8_cheat_step(&g_cheat, g_input);
    pico8__update60();
    const entity_t *held = main_ui()->left_held;
    assert(held != NULL && "f23 : une structure est tenue");
    planet_t *p34 = planet_targeted_by(held);
    assert(p34 != NULL && "f23 : la structure tenue accroche le fo d'une planete");
    far = vec_add(*((const camera_t *)main_camera())->pos, vec_make(p8_fixed_from_int(500), 0));
    p34->pos = &far;
    assert(!entity_geom_is_on_screen(&p34->header) && "p34 est hors ecran");
    pico8__draw();
    assert(p34->targeted_by == held && "hors ecran, kj laisse fo");
    if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    g_frame++;
    while (g_frame <= 27) play_one_frame(1);
    play_one_frame(0);
    assert(main_ui()->left_held == held && "f28 : la prise suivante reprend l'arene, a la meme adresse");
    assert(p34->targeted_by == NULL && "le fo d'une planete hors ecran vise une prise refusee recyclee");
}

static void refused_grab_arena_crosses_new_game(void) {
    start_e2e(GRAB_ARENAS);
    play_until(195);
}

#define SLIPWAYS "tests/fixtures/slipways_refused.json"

/* Fixture SLIPWAYS : f0-34 de slipway_build (« p27 » et « p48 »
 * colonisees) ; puis des cycles de 15 frames : pression sur « p27 » (88,37),
 * dix frames tenues, relache, trois d'attente. Vingt refus tenus en (40,100)
 * (f35-334), un glisse pose de p27 a p48 (f335-338, attente f339-341), un
 * refus tenu en (73,91), au-dela de p48 (f342-356) ; « restart » et nouvelle
 * partie (clic f367) ; une prise d'icone (f371-377). Un refus est relache a
 * la 11e frame du cycle et rendu au vidage de la 13e. */
static const slipway_t *held_slipway(void) {
    const entity_t *held = main_ui()->left_held;
    assert(held != NULL && held->class == slipway_class() && "un slipway est tenu");
    return (const slipway_t *)held;
}

static void held_slipway_does_not_grow_game(void) {
    start_e2e(SLIPWAYS);
    play_until(36);
    const slipway_t *held = held_slipway();
    const size_t f36 = game_zone_used();
    play_until(45);
    assert(held_slipway() == held && held->end == &held->point.header && "f45 : tenu au-dessus du vide");
    printf("  slipway tenu f36/f45 : partie = %lu/%lu\n", (unsigned long)f36, (unsigned long)game_zone_used());
    assert(game_zone_used() == f36 && "un slipway tenu alloue a chaque frame");
}

static void refused_slipways_do_not_grow_game(void) {
    start_e2e(SLIPWAYS);
    play_until(49);
    const size_t un = game_without_registries();
    play_until(334);
    const size_t twenty = game_without_registries();
    printf("  slipways refuses 1/20 : partie - registres = %lu/%lu\n", (unsigned long)un, (unsigned long)twenty);
    assert(un == twenty && "un slipway refuse laisse son arene dans la generation");
}

static size_t slipways_of_p27_and_p48_bytes(void) {
    return registry_bytes(planet_with_id("p27")->slipways) + registry_bytes(planet_with_id("p48")->slipways);
}

/* Le `kf` de la pose fait grandir `oe` aux deux bouts, dans la generation,
 * hors de l'arene. */
static void placed_slipway_reuses_dead_wormhole_arena(void) {
    start_e2e(SLIPWAYS);
    play_until(334);
    const size_t before = game_without_registries();
    const size_t slipways_before = slipways_of_p27_and_p48_bytes();
    play_until(356);
    const size_t after = game_without_registries();
    const size_t oe = slipways_of_p27_and_p48_bytes() - slipways_before;
    assert(entities_by_tag("slipway") != NULL && entities_by_tag("slipway")->count == 1 && "f338 : un slipway pose");
    printf("  slipway pose puis refuse : partie - registres = %lu -> %lu (slipways %lu)\n", (unsigned long)before,
           (unsigned long)after, (unsigned long)oe);
    assert(after == before + oe &&
           "le slipway pose reprend l'arene libre du mz de f4 (rendu a f48), le refuse suivant l'autre");
}

/* f345 : le slipway tenu de p27 vers (73,91) accroche le `fo` de p48, sur
 * son chemin, qu'on pousse hors ecran : son `kj` ne remet plus `fo` a nil.
 * Refuse a f353, le slipway est rendu au vidage de f355. */
static void offscreen_planet_forgets_refused_slipway(void) {
    static vec_t far;
    start_e2e(SLIPWAYS);
    play_until(344);
    dispatch_context_set_input(g_input);
    pico8_cheat_step(&g_cheat, g_input);
    pico8__update60();
    const slipway_t *held = held_slipway();
    planet_t *p = planet_targeted_by(&held->header);
    assert(p != NULL && "f345 : le slipway tenu accroche le fo d'une planete");
    far = vec_add(*((const camera_t *)main_camera())->pos, vec_make(p8_fixed_from_int(500), 0));
    p->pos = &far;
    assert(!entity_geom_is_on_screen(&p->header) && "la planete est hors ecran");
    pico8__draw();
    if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    g_frame++;
    play_until(354);
    assert(p->targeted_by == &held->header && "hors ecran, kj laisse fo");
    play_until(355);
    printf("  slipway refuse : targeted_by de %s coupe au vidage de f355\n", p->planet_id);
    assert(p->targeted_by == NULL && "le fo d'une planete hors ecran vise un slipway refuse rendu");
}

static void refused_slipway_crosses_new_game(void) {
    start_e2e(SLIPWAYS);
    play_until(356);
    assert(entities_by_tag("slipway") != NULL && entities_by_tag("slipway")->count == 1);
    play_until(371);
    assert(entities_by_tag("slipway") == NULL || entities_by_tag("slipway")->count == 0);
    assert(main_ui()->left_held != NULL && "f371 : une prise dans la nouvelle partie");
    play_until(377);
}

#define WORMHOLES "tests/fixtures/wormholes.json"

/* Fixture PLIAGES : f0-3 de slipway_build (sonde a f2) ; puis des cycles de
 * 7 frames en (80,55) : quatre tenues des la pression, trois relachees.
 * Vingt `mz` refuses (f4-143), un tenu seize frames (f144-159) et ouvert au
 * relache de f160, rendu a f188 ; une pression refusee (f193-199) ;
 * « restart » et nouvelle partie (clic f210) ; une sonde en (55,50) (f216),
 * un `mz` refuse (f218-224). Un refus est relache a la 5e frame du cycle et rendu
 * au vidage de la 7e. */
static const wormhole_t *held_wormhole(void) {
    const entity_t *held = main_ui()->left_held;
    assert(held != NULL && held->class == wormhole_class() && "un trou de ver est tenu");
    return (const wormhole_t *)held;
}

static void held_wormhole_does_not_grow_game(void) {
    start_e2e(WORMHOLES);
    play_until(4);
    const wormhole_t *held = held_wormhole();
    const size_t f4 = game_zone_used();
    play_until(7);
    assert(held_wormhole() == held);
    printf("  wormhole tenu f4/f7 : partie = %lu/%lu\n", (unsigned long)f4, (unsigned long)game_zone_used());
    assert(game_zone_used() == f4 && "un trou de ver tenu alloue a chaque frame");
}

static void refused_wormholes_do_not_grow_game(void) {
    start_e2e(WORMHOLES);
    play_until(10);
    const size_t un = game_without_registries();
    play_until(143);
    const size_t twenty = game_without_registries();
    printf("  wormhole refuses 1/20 : partie - registres = %lu/%lu\n", (unsigned long)un, (unsigned long)twenty);
    assert(un == twenty && "un trou de ver refuse laisse son arene dans la generation");
}

static void open_wormhole_releases_arena_on_death(void) {
    start_e2e(WORMHOLES);
    play_until(143);
    const size_t before = game_without_registries();
    play_until(160);
    const entity_list_t *mz = entities_by_tag("wormhole");
    assert(mz != NULL && mz->count == 1 && strcmp(((const wormhole_t *)mz->items[0])->header.state, "unfolding") == 0 &&
           "f160 : le trou de ver est ouvert");
    play_until(193);
    assert(entities_by_tag("wormhole")->count == 1 && "f193 : seul le trou de ver de la pression vit");
    const size_t after = game_without_registries();
    printf("  wormhole ouvert : partie - registres = %lu -> %lu\n", (unsigned long)before, (unsigned long)after);
    assert(after == before && "un trou de ver ouvert garde son arene apres sa mort");
}

static void refused_wormhole_crosses_new_game(void) {
    start_e2e(WORMHOLES);
    play_until(218);
    held_wormhole();
    play_until(224);
}

#define PROBES "tests/fixtures/probes_refused.json"

/* Fixture SONDES : clic « reasonable » (f0-1) ; puis des cycles de 7 frames
 * dans le vide, hors du panneau `hw`, en (5,100) : quatre tenues des la
 * pression, trois relachees. Dix sondes refusees (f2-71) ; une pressee en
 * (5,100) a f72, glissee et relachee sur p34 (31,84) (f73-78) ; un refus
 * (f79-85) ; une sonde posee en (55,50) (f86-87) ; un `mz` tenu (f88-94) ;
 * « restart » et nouvelle partie (clic f105) ; un refus (f109-115). Un
 * refus est relache a la 5e frame du cycle et rendu au vidage de la 7e. */
static const probe_t *held_probe(void) {
    const entity_t *held = main_ui()->left_held;
    assert(held != NULL && held->class == probe_class() && "une sonde est tenue");
    return (const probe_t *)held;
}

static void refused_probes_do_not_grow_game(void) {
    start_e2e(PROBES);
    play_until(8);
    const size_t a = game_without_registries();
    play_until(71);
    const size_t ten = game_without_registries();
    printf("  sondes refusees 1/10 : partie - registres = %lu/%lu\n", (unsigned long)a, (unsigned long)ten);
    assert(a == ten && "une sonde refusee laisse son arene dans la generation");
}

static void each_refused_probe_reuses_previous_arena(void) {
    start_e2e(PROBES);
    play_until(2);
    const probe_t *first = held_probe();
    for (int k = 1; k < 10; k++) {
        play_until(2 + 7 * k);
        assert(held_probe() == first && "la sonde suivante reprend l'arene de la refusee, a la meme adresse");
    }
    printf("  sondes refusees : dix sondes a la meme adresse\n");
}

/* f74 : la sonde tenue sur p34 y accroche son `fo` ; p34 passe hors ecran
 * avant le draw. Refusee a f76, la sonde est rendue au vidage de f78, et
 * la pression de f79 reprend son arene. */
static void offscreen_planet_forgets_refused_probe(void) {
    static vec_t far;
    start_e2e(PROBES);
    play_until(73);
    dispatch_context_set_input(g_input);
    pico8_cheat_step(&g_cheat, g_input);
    pico8__update60();
    const entity_t *held = &held_probe()->header;
    planet_t *p34 = planet_targeted_by(held);
    assert(p34 != NULL && "f74 : la sonde tenue accroche le fo d'une planete");
    far = vec_add(*((const camera_t *)main_camera())->pos, vec_make(p8_fixed_from_int(500), 0));
    p34->pos = &far;
    assert(!entity_geom_is_on_screen(&p34->header) && "p34 est hors ecran");
    pico8__draw();
    assert(p34->targeted_by == held && "hors ecran, kj laisse fo");
    if (g_frame < g_input->frame_count - 1) p8_input_step(g_input);
    g_frame++;
    while (g_frame <= 78) play_one_frame(1);
    play_one_frame(0);
    assert(main_ui()->left_held == held && "f79 : la sonde suivante reprend l'arene, a la meme adresse");
    assert(p34->targeted_by == NULL && "le fo d'une planete hors ecran vise une sonde refusee recyclee");
}

/* f90 : `ox` du `mz` tenu ne garde que des sondes posees ; la sonde posee
 * garde son arene, le `mz` en prend une autre. */
static void wormhole_targets_only_placed_probe(void) {
    start_e2e(PROBES);
    play_until(90);
    const wormhole_t *mz = held_wormhole();
    const probe_t *placed = NULL;
    for (int i = 0; i < mz->anchors->count; i++) {
        const entity_t *e = mz->anchors->items[i];
        if (e->class != probe_class()) continue;
        assert(placed == NULL && strcmp(e->state, "settled") == 0 && "ox ne garde qu'une sonde, posee");
        placed = (const probe_t *)e;
    }
    assert(placed != NULL && "f86 : la sonde posee est une ancre du mz");
    assert(placed->grab_arena != NULL && placed->grab_arena != mz->anchors->zone && "la sonde posee garde son arene");
}

static void refused_probe_crosses_new_game(void) {
    start_e2e(PROBES);
    play_until(109);
    held_probe();
    play_until(115);
}

#define WORMHOLE_ABANDONED "tests/fixtures/wormhole_abandoned.json"

/* Fixture PLIAGE_ABANDONNE : f0-20 de slipway_build (`mz` ouvert a f20),
 * puis f2-17 de pause_restart : « restart » a f26 abandonne le `mz`, le clic
 * « challenging » de f31 bascule la zone partie. */
static const wormhole_t *abandoned_wormhole(void) {
    start_e2e(WORMHOLE_ABANDONED);
    play_until(22);
    const entity_list_t *mz = entities_by_tag("wormhole");
    assert(mz != NULL && mz->count == 1 && "f22 : un trou de ver vit");
    return (const wormhole_t *)mz->items[0];
}

static void abandoned_wormhole_rereads_during_click(void) {
    const wormhole_t *mz = abandoned_wormhole();
    play_until(31);
    assert(game_zone_pending() && "le clic de f31 fait basculer la zone partie");
    g_spend = (unsigned char)mz->is_dead;
}

static void abandoned_wormhole_dies_with_its_game(void) {
    const wormhole_t *mz = abandoned_wormhole();
    play_until(32);
    g_spend = (unsigned char)mz->is_dead;
}

/* f11 : titre apres « restart ». Recharger rend les places des entites de
 * hv, et le jl() d'_init ne relit pas les registres remis a zero. */
static void pause_restart_reloads_after_title(void) {
    reading_t neuf, title, reloads;
    start_e2e("pause_restart");
    take_reading(&neuf, BEFORE_F0);
    play_until(11);
    take_reading(&title, 11);
    print_reading("pause_restart", &title);
    pico8_cart_shutdown();
    pico8_cart_load();
    pico8__init();
    pico8__update60();
    pico8__draw();
    take_reading(&reloads, 0);
    print_reading("pause_restart recharge", &reloads);
    assert(same_counts(&neuf, &reloads) && "recharger doit rendre les places des entites de hv");
}

/* Apres le rendu de f13, jo.ir et jo.oc doivent viser la nouvelle
 * generation : sinon ASan plante a la premiere lecture. */
static void pause_restart_reveals_on_new_game_states(void) {
    start_e2e("pause_restart");
    play_until(13);
    const entity_list_t *jo = entities_by_tag("planet");
    const planet_types_entry_t *ll = planet_types_lookup("vanishing");
    int revealed = 0;
    int from_type_picker = 0;
    for (int i = 0; jo != NULL && i < jo->count && revealed < 5; i++) {
        planet_t *p = (planet_t *)jo->items[i];
        if (p->header.state == NULL || strcmp(p->header.state, "unknown") != 0) continue;
        planet_roll_type(p);
        revealed++;
        from_type_picker += p->planet_type != NULL && p->planet_type != ll;
    }
    assert(revealed > 0 && "la nouvelle partie a des planetes unknown");
    assert(from_type_picker > 0 && "aucun devoilement n'a lu jo.oc");
    printf("  pause_restart f13 : %d planetes devoilees, %d par jo.oc\n", revealed, from_type_picker);
}

/* f42-90 : la boite de l'annee retenue par son lh, sans clic. */
static void message_box_hold_idle_does_not_grow(void) {
    const int frames[] = {45, 90};
    reading_t r[2];
    replay_e2e("message_box_hold", frames, 2, r);
    assert(same_counts(&r[0], &r[1]) && r[0].particle == r[1].particle && "une sorte prend une place par tick");
    assert(r[0].game_zone == r[1].game_zone && "la zone partie prend des octets par tick");
}

/* f44-45 : la boite de l'annee retenue, sans action. Les boites refaites
 * par tick sont reecrites en place : elles gardent leur adresse. */
static void message_box_hold_rewrites_boxes_in_place(void) {
    start_e2e("message_box_hold");
    play_until(44);
    const camera_t *mo = main_camera_typed();
    const mouse_t *je = main_mouse_typed();
    const ui_t *ok = main_ui();
    assert(mo != NULL && je != NULL && ok != NULL);
    const void *before[5] = {mo->pos_before_shake, mo->velocity, mo->pos, je->pos, ok->mouse_world};
    size_t grab_arena = game_zone_used();
    play_until(45);
    assert(main_camera_typed() == mo && main_mouse_typed() == je && main_ui() == ok);
    const void *after[5] = {mo->pos_before_shake, mo->velocity, mo->pos, je->pos, ok->mouse_world};
    int stables = 0;
    for (int i = 0; i < 5; i++) stables += before[i] == after[i];
    printf("  message_box_hold f44-45 : %d / 5 boites par tick a la meme adresse\n", stables);
    fflush(stdout);
    assert(stables == 5 && "mo.p, mo.v, mo.ra, je.ra et ok.qa doivent etre reecrits en place");
    assert(game_zone_used() == grab_arena && "la zone partie prend des octets par tick");
}

static int no_button(const p8_input_frame_t *f) {
    if (f->mouse_buttons != 0) return 0;
    for (int b = 0; b < 6; b++) {
        if (f->dpad[b]) return 0;
    }
    return 1;
}

static int idle_frame(const p8_input_t *input, int i) {
    const p8_input_frame_t *f = &input->frames[i];
    const p8_input_frame_t *before = &input->frames[i - (i > 0)];
    return i > 0 && no_button(f) && no_button(before) && f->mouse_x == before->mouse_x &&
           f->mouse_y == before->mouse_y;
}

/* Une fenetre : au moins deux frames sans bouton ni D-pad, souris immobile,
 * la frame d'avant comprise. La generation de sa deuxieme frame (la premiere
 * peut encore vider ce que l'action d'avant a laisse) est celle de sa
 * derniere. Sans `relever`, dont le vidage rendrait la generation en avance. */
static int idle_windows_of_current_e2e(void) {
    start_e2e(g_e2e);
    int start = -1;
    size_t second = 0;
    int windows = 0;
    for (int i = 0; i < g_input->frame_count; i++) {
        int idle = idle_frame(g_input, i);
        play_until(i);
        size_t grab_arena = game_zone_used();
        if (idle && start < 0) start = i;
        if (idle && i == start + 1) second = grab_arena;
        int end = idle && (i + 1 == g_input->frame_count || !idle_frame(g_input, i + 1));
        if (end && i > start) {
            if (grab_arena != second) {
                fprintf(stderr, "  %s f%d-%d : partie %lu -> %lu\n", g_e2e, start, i, (unsigned long)second,
                        (unsigned long)grab_arena);
                assert(0 && "la generation monte pendant une fenetre sans action");
            }
            windows++;
        }
        if (!idle || end) start = -1;
    }
    return windows;
}

/* Rend ce que rend `test`, joue dans un enfant (moins de 256). */
static int count_in_child(int (*test)(void)) {
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        int n = test();
        fflush(stdout);
        _exit(n < 255 ? n : 255);
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) < 255 && "un releve de fenetres sans action a echoue");
    return WEXITSTATUS(status);
}

/* Rend le nombre de fenetres sans action des e2e de tests/e2e/. */
static int idle_windows_on_all_e2e(void) {
    DIR *dir = opendir("tests/e2e");
    struct dirent *ent;
    int total = 0;
    assert(dir != NULL);
    while ((ent = readdir(dir)) != NULL) {
        size_t len = strlen(ent->d_name);
        if (len <= 5 || strcmp(ent->d_name + len - 5, ".json") != 0) continue;
        assert(len - 5 < sizeof g_e2e);
        memcpy(g_e2e, ent->d_name, len - 5);
        g_e2e[len - 5] = '\0';
        total += count_in_child(idle_windows_of_current_e2e);
    }
    closedir(dir);
    return total;
}

/* f25-94 : « p40 » s'efface ; sa rangee rendue peut faire baisser `rangee`. */
static void empty_planet_fade_does_not_grow(void) {
    const int frames[] = {25, 94};
    reading_t r[2];
    replay_e2e("empty_planet_fade", frames, 2, r);
    assert(none_rises(&r[0], &r[1]) && "une sorte monte pendant l'effacement de p40");
}

/* f56 : « harvest », p27 devient un eclat. A f139, sa troisieme particule
 * vit encore : on prolonge le rejeu sans action (la souris reste en
 * (100,62), sans clic) jusqu'a son extinction. */
static void tech_geoharvesting_burst_releases_burst(void) {
    const int frames[] = {55, 56, 139};
    reading_t r[3];
    reading_t end;
    int frame;
    replay_e2e("tech_geoharvesting_burst", frames, 3, r);
    assert(r[0].burst == 0 && r[1].burst == 1 && "le « harvest » fait un eclat");
    assert(r[1].particle - r[0].particle >= 140 && "un eclat nait avec 140 particules");
    end = r[2];
    for (frame = 140; end.burst != 0 && frame < 160; frame++) {
        pico8__update60();
        pico8__draw();
        take_reading(&end, frame);
    }
    print_reading("tech_geoharvesting_burst", &end);
    assert(end.burst == 0 && "un eclat eteint doit rendre sa place");
    assert(end.particles == r[0].particles && "un eclat eteint doit rendre sa liste");
}

/* Sans entree, l'aide emet 5 particules par draw et chacune vit 115
 * frames : au-dela, le compte plafonne. */
static void idle_title_screen_settles(void) {
    int ceiling[4] = {0, 0, 0, 0};
    load_e2e(NULL);
    for (int i = 0; i < 400; i++) {
        pico8__update60();
        pico8__draw();
        check_invariant("titre", i);
        assert(particle_zone_live(PARTICLE_SLOT_HELP_LIST) == 1 && "l'aide garde une seule liste");
        int p = particle_zone_live(PARTICLE_SLOT_PARTICLE);
        if (p > ceiling[i / 100]) ceiling[i / 100] = p;
    }
    printf("  titre sans entree, particule max par fenetre de 100 frames : %d %d %d %d\n", ceiling[0], ceiling[1],
           ceiling[2], ceiling[3]);
    assert(ceiling[3] <= ceiling[2] && "le compte de particules de l'aide monte encore apres sa duree de vie");
}

/* Tampon alloue, termine par un nul, que l'appelant libere. */
static char *read_file(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    assert(f != NULL);
    assert(fseek(f, 0, SEEK_END) == 0);
    long size = ftell(f);
    assert(size >= 0);
    rewind(f);
    char *buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, f) == (size_t)size);
    buf[size] = '\0';
    fclose(f);
    *len = (size_t)size;
    return buf;
}

static int same_bytes(const char *a_path, const char *b_path) {
    size_t a_len, b_len;
    char *a = read_file(a_path, &a_len);
    char *b = read_file(b_path, &b_len);
    int same = a_len == b_len && memcmp(a, b, a_len) == 0;
    free(a);
    free(b);
    return same;
}

#define DRIVER "./tmp/pico8_replay"
#define FILL "tmp/test_places_vivantes_remplissage.txt"
#define CAPTURE_A "tmp/test_places_vivantes_a.bin"
#define CAPTURE_B "tmp/test_places_vivantes_b.bin"
#define DUMP_A "tmp/test_places_vivantes_a.txt"
#define DUMP_B "tmp/test_places_vivantes_b.txt"

/* PICO8_REMPLISSAGE=<fichier> : le rejeu y ecrit une ligne « <sorte>
 * <vivantes> » par sorte d'interface puis de particules, les maxima de
 * la session par sorte de particules, puis les octets pris de la zone
 * partie et leur maximum, puis ceux de la sous-zone chargement, en fin de
 * session, sans toucher capture, dump ni
 * code de sortie. */
static void replay_writes_fill_at_session_end(void) {
    const int frames[] = {17};
    reading_t reading[1];
    replay_e2e("pause_restart", frames, 1, reading);

    remove(FILL);
    assert(system(DRIVER " tests/e2e/pause_restart.json " CAPTURE_A " " DUMP_A " > /dev/null") == 0);
    assert(system("PICO8_REMPLISSAGE=" FILL " " DRIVER " tests/e2e/pause_restart.json " CAPTURE_B " " DUMP_B
                  " > /dev/null") == 0);
    assert(same_bytes(CAPTURE_A, CAPTURE_B) && "le remplissage ne change pas la capture");
    assert(same_bytes(DUMP_A, DUMP_B) && "le remplissage ne change pas le dump");

    char expected[1024] = "";
    char line[48];
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        snprintf(line, sizeof line, "%s %d\n", NAMES[s], reading[0].live[s]);
        strcat(expected, line);
    }
    static const char *const PARTICLE[PARTICLE_SLOT_COUNT] = {"particule", "qy", "qy_fv", "burst", "ts"};
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++) {
        snprintf(line, sizeof line, "%s %d\n", PARTICLE[s],
                 particle_zone_live((particle_slot_kind_t)s));
        strcat(expected, line);
    }
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++) {
        snprintf(line, sizeof line, "max_vivantes %s %d\n", PARTICLE[s],
                 particle_zone_max_live((particle_slot_kind_t)s));
        strcat(expected, line);
    }
    for (int s = PARTICLE_SLOT_LIST; s < PARTICLE_SLOT_COUNT; s++) {
        snprintf(line, sizeof line, "max_octets %s %lu\n", PARTICLE[s],
                 (unsigned long)particle_zone_max_bytes((particle_slot_kind_t)s));
        strcat(expected, line);
    }
    for (int s = PARTICLE_SLOT_LIST; s < PARTICLE_SLOT_COUNT; s++) {
        if (s == PARTICLE_SLOT_BURST) continue;
        snprintf(line, sizeof line, "max_elements %s %d\n", PARTICLE[s],
                 particle_zone_max_elements((particle_slot_kind_t)s));
        strcat(expected, line);
    }
    snprintf(line, sizeof line, "partie %lu\nmax_octets partie %lu\n", (unsigned long)game_zone_used(),
             (unsigned long)game_zone_max());
    strcat(expected, line);
    snprintf(line, sizeof line, "chargement %lu\n", (unsigned long)load_zone_used());
    strcat(expected, line);
    snprintf(line, sizeof line, "max_octets chargement %lu\n", (unsigned long)game_zone_load_zone_max());
    strcat(expected, line);
    size_t len;
    char *writes = read_file(FILL, &len);
    printf("  remplissage ecrit par le rejeu :\n%s", writes);
    assert(strcmp(writes, expected) == 0 && "le rejeu ecrit les comptes de la derniere frame, apres vidage");
    free(writes);
    remove(FILL);
    remove(CAPTURE_A);
    remove(CAPTURE_B);
    remove(DUMP_A);
    remove(DUMP_B);
}

/* La valeur de la ligne « <cle> <n> » du remplissage. */
static long fill_value(const char *fill, const char *key) {
    char motif[64];
    long n;
    snprintf(motif, sizeof motif, "\n%s ", key);
    const char *l = strstr(fill, motif);
    assert(l != NULL && "cle absente du remplissage");
    assert(sscanf(l + strlen(motif), "%ld", &n) == 1);
    return n;
}

/* L'eclat de f56 meurt a f141, apres la fin du rejeu (f139), mais ses 140
 * particules naissent d'un coup et la plupart sont mortes a f139 : un
 * maximum lu en fin de session seulement resterait sous 140. */
static void replay_writes_session_peak(void) {
    remove(FILL);
    assert(system("PICO8_REMPLISSAGE=" FILL " " DRIVER " tests/e2e/tech_geoharvesting_burst.json " CAPTURE_A
                  " " DUMP_A " > /dev/null") == 0);
    size_t len;
    char *writes = read_file(FILL, &len);
    long particle = fill_value(writes, "particule");
    long max_particle = fill_value(writes, "max_vivantes particule");
    printf("  tech_geoharvesting_burst : particule %ld en fin de session, max %ld ; burst %ld, max %ld\n", particle,
           max_particle, fill_value(writes, "burst"), fill_value(writes, "max_vivantes burst"));
    assert(fill_value(writes, "max_vivantes burst") >= 1 && "le « harvest » fait un eclat");
    assert(max_particle >= 140 && particle < 140 && "le maximum se releve a chaque prise, pas en fin de session");
    free(writes);
    remove(FILL);
    remove(CAPTURE_A);
    remove(DUMP_A);
}

int main(void) {
    int e2e = invariant_on_all_e2e();
    printf("  invariant particule = somme des #particles inscrits, transit_dots = somme des #transit_dots de entities_by_tag(slipway) : %d e2e, chaque frame\n", e2e);
    assert(e2e == 26 && "tests/e2e/ compte 26 rejeux");
    in_child(pause_restart_returns_to_title);
    in_child(pause_restart_keeps_quit_game_during_click);
    in_child_dying_by_asan(pause_restart_releases_quit_game_next_frame);
    in_child(pause_restart_keeps_game_across_restart);
    in_child(pause_restart_keeps_quit_planets_during_click);
    in_child_dying_by_asan(pause_restart_releases_quit_planets);
    in_child(pause_restart_keeps_quit_economy_during_click);
    in_child_dying_by_asan(pause_restart_releases_quit_economy);
    in_child(pause_restart_keeps_quit_icons_during_click);
    in_child_dying_by_asan(pause_restart_releases_quit_icons);
    in_child(pause_restart_keeps_quit_singletons_during_click);
    for (g_singleton = 0; g_singleton < SINGLETON_COUNT; g_singleton++) {
        printf("  singleton %s :", NAMES_OF_SINGLETONS[g_singleton]);
        in_child_dying_by_asan(pause_restart_releases_quit_singleton);
    }
    in_child(pause_restart_counts_populate_galaxy_states);
    in_child(pause_restart_leaves_load_zone_alone);
    in_child(pause_restart_bounds_dead_registry_blocks);
    in_child(pause_restart_reloads_after_title);
    in_child(pause_restart_reveals_on_new_game_states);
    in_child(game_end_final_score_rebuild_does_not_grow_game);
    in_child(trade_rich_mine_route_rebuild_does_not_grow_game);
    in_child(trade_rich_mine_kept_route_survives_frame);
    in_child(trade_rich_mine_releases_trails_on_reload);
    in_child(refused_grab_arenas_do_not_grow_game);
    in_child(placed_grab_arena_reuses_dead_wormhole_arena);
    in_child(refused_grab_arena_crosses_new_game);
    in_child(offscreen_planet_forgets_refused_grab_arena);
    in_child(held_slipway_does_not_grow_game);
    in_child(refused_slipways_do_not_grow_game);
    in_child(placed_slipway_reuses_dead_wormhole_arena);
    in_child(refused_slipway_crosses_new_game);
    in_child(offscreen_planet_forgets_refused_slipway);
    in_child(held_wormhole_does_not_grow_game);
    in_child(refused_wormholes_do_not_grow_game);
    in_child(open_wormhole_releases_arena_on_death);
    in_child(refused_wormhole_crosses_new_game);
    in_child(refused_probes_do_not_grow_game);
    in_child(each_refused_probe_reuses_previous_arena);
    in_child(offscreen_planet_forgets_refused_probe);
    in_child(wormhole_targets_only_placed_probe);
    in_child(refused_probe_crosses_new_game);
    in_child(abandoned_wormhole_rereads_during_click);
    in_child_dying_by_asan(abandoned_wormhole_dies_with_its_game);
    in_child(message_box_hold_idle_does_not_grow);
    int windows = idle_windows_on_all_e2e();
    printf("  generation constante sur %d fenetres sans action des 26 e2e\n", windows);
    assert(windows > 0);
    in_child(message_box_hold_rewrites_boxes_in_place);
    in_child(empty_planet_fade_does_not_grow);
    in_child(tech_geoharvesting_burst_releases_burst);
    in_child(idle_title_screen_settles);
    in_child(replay_writes_fill_at_session_end);
    in_child(replay_writes_session_peak);
    printf("test_live_slots: all tests passed\n");
    return 0;
}
