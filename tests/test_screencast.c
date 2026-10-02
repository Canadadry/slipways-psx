/* Le format de capture vu du dehors : des pixels a des positions
 * donnees, un nombre de frames, des refus sur des fichiers invalides --
 * jamais la presence d'un champ, d'une fonction ou d'une structure
 * interne. Les fichiers de travail sont ecrits sous tmp/ et effaces. */

#include "devtools/screencast.h"

#include "p8/gfx.h"
#include "p8/mem.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Le pixel d'indice i vaut (i + seed) & 0x0f : deux frames de seeds
 * distincts different en presque tout point, ce qui donne aux
 * assertions ci-dessous le pouvoir de distinguer la frame N de la
 * frame N+1, donc d'echouer sur un offset decale ou deux frames
 * interverties. Un motif constant les rendrait toutes aveugles. */
static void paint_frame(p8_memory_t *mem, int seed) {
    for (int i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        p8_poke(mem, P8_ADDR_SCREEN + i, (unsigned char)((i + seed) & 0x0f));
    }
}

static unsigned char expected_pixel(int seed, int x, int y) {
    return (unsigned char)(((y * P8_SCREEN_WIDTH + x) + seed) & 0x0f);
}

static unsigned char pixel_at(const unsigned char *frame, int x, int y) {
    return frame[y * P8_SCREEN_WIDTH + x];
}

/* Ecrit `count` frames de seeds 0..count-1 a `path`. */
static void write_seeded_frames(const char *path, int count) {
    pico8_screencast_writer_t writer;
    p8_memory_t mem;

    p8_mem_init(&mem);
    assert(pico8_screencast_writer_open(&writer, path) == 1);
    for (int seed = 0; seed < count; seed++) {
        paint_frame(&mem, seed);
        assert(pico8_screencast_write_frame(&writer, &mem) == 1);
    }
    assert(pico8_screencast_writer_close(&writer) == 1);
}

/* L'aller-retour : ce qui a ete ecrit depuis la memoire du jeu se relit
 * pixel pour pixel. Les positions asserties couvrent les quatre coins et
 * le centre, et les deux quartets d'un meme octet (x pair et x impair),
 * seul endroit ou une inversion de quartets se verrait. */
static void test_written_frame_reads_back_pixel_for_pixel(void) {
    const char *path = "tmp/test_screencast_roundtrip.bin";
    pico8_screencast_reader_t reader;
    unsigned char frame[PICO8_SCREENCAST_FRAME_PIXELS];

    write_seeded_frames(path, 3);

    assert(pico8_screencast_reader_open(&reader, path) == PICO8_SCREENCAST_OK);
    assert(pico8_screencast_read_frame(&reader, 0, frame) == 1);

    assert(pixel_at(frame, 0, 0) == expected_pixel(0, 0, 0));
    assert(pixel_at(frame, 1, 0) == expected_pixel(0, 1, 0));
    assert(pixel_at(frame, 127, 0) == expected_pixel(0, 127, 0));
    assert(pixel_at(frame, 64, 64) == expected_pixel(0, 64, 64));
    assert(pixel_at(frame, 0, 127) == expected_pixel(0, 0, 127));
    assert(pixel_at(frame, 127, 127) == expected_pixel(0, 127, 127));

    pico8_screencast_reader_close(&reader);
    remove(path);
}

/* Le nombre de frames est la taille du fichier divisee par la taille
 * d'une frame, et rien d'autre : le format n'a ni en-tete ni index ou
 * un compteur pourrait se cacher. L'assertion sur la taille du fichier
 * est l'autre moitie de la meme affirmation, et c'est elle qui fixe les
 * 8 192 octets par frame. */
static void test_frame_count_is_file_size_divided_by_frame_size(void) {
    const char *path = "tmp/test_screencast_count.bin";
    pico8_screencast_reader_t reader;

    write_seeded_frames(path, 5);

    FILE *raw = fopen(path, "rb");
    assert(raw != NULL);
    assert(fseek(raw, 0, SEEK_END) == 0);
    assert(ftell(raw) == 5L * 8192L);
    fclose(raw);

    assert(pico8_screencast_reader_open(&reader, path) == PICO8_SCREENCAST_OK);
    assert(pico8_screencast_frame_count(&reader) == 5);

    pico8_screencast_reader_close(&reader);
    remove(path);
}

/* Chaque frame du fichier est la sienne, dans l'ordre ou elle a ete
 * ecrite. Les assertions portent sur les valeurs des pixels et pas sur
 * le nombre de frames, parce qu'une source qui les servirait a l'envers
 * passerait un test qui les compterait. */
static void test_each_frame_holds_its_own_pixels_in_order(void) {
    const char *path = "tmp/test_screencast_in_order.bin";
    pico8_screencast_reader_t reader;
    unsigned char frame[PICO8_SCREENCAST_FRAME_PIXELS];

    write_seeded_frames(path, 6);

    assert(pico8_screencast_reader_open(&reader, path) == PICO8_SCREENCAST_OK);
    assert(pico8_screencast_frame_count(&reader) == 6);

    for (int seed = 0; seed < 6; seed++) {
        assert(pico8_screencast_read_frame(&reader, seed, frame) == 1);
        assert(pixel_at(frame, 0, 0) == expected_pixel(seed, 0, 0));
        assert(pixel_at(frame, 3, 0) == expected_pixel(seed, 3, 0));
        assert(pixel_at(frame, 64, 64) == expected_pixel(seed, 64, 64));
        assert(pixel_at(frame, 127, 127) == expected_pixel(seed, 127, 127));
    }

    pico8_screencast_reader_close(&reader);
    remove(path);
}

/* Rien dans le format n'oblige a lire les frames dans l'ordre, et une
 * lecture qui dependrait de l'ordre ferait mentir "la frame N est a
 * l'offset N x 8192". L'ordre teste lit a rebours, saute, et relit une
 * frame deja lue ; on exige les memes pixels qu'en lecture
 * sequentielle. */
static void test_frames_can_be_read_out_of_sequence(void) {
    const char *path = "tmp/test_screencast_out_of_order.bin";
    pico8_screencast_reader_t reader;
    unsigned char frame[PICO8_SCREENCAST_FRAME_PIXELS];
    static const int order[] = { 4, 0, 3, 1, 4, 2 };

    write_seeded_frames(path, 5);

    assert(pico8_screencast_reader_open(&reader, path) == PICO8_SCREENCAST_OK);

    for (size_t i = 0; i < sizeof(order) / sizeof(order[0]); i++) {
        int seed = order[i];
        assert(pico8_screencast_read_frame(&reader, seed, frame) == 1);
        assert(pixel_at(frame, 0, 0) == expected_pixel(seed, 0, 0));
        assert(pixel_at(frame, 1, 0) == expected_pixel(seed, 1, 0));
        assert(pixel_at(frame, 70, 12) == expected_pixel(seed, 70, 12));
        assert(pixel_at(frame, 127, 127) == expected_pixel(seed, 127, 127));
    }

    pico8_screencast_reader_close(&reader);
    remove(path);
}

/* Un fichier absent est refuse a l'ouverture, avec un resultat qui lui
 * est propre : une fenetre ouverte sur du noir est indiscernable d'une
 * session qui commence par un ecran noir. */
static void test_missing_file_is_refused(void) {
    pico8_screencast_reader_t reader;

    assert(pico8_screencast_reader_open(&reader, "tmp/test_screencast_no_such_file.bin") ==
           PICO8_SCREENCAST_UNREADABLE);
    assert(pico8_screencast_frame_count(&reader) == 0);

    pico8_screencast_reader_close(&reader);
}

/* Ecrit `size` octets arbitraires a `path`, pour fabriquer les
 * fichiers que la lecture doit refuser. */
static void write_raw_bytes(const char *path, long size) {
    FILE *raw = fopen(path, "wb");
    assert(raw != NULL);
    for (long i = 0; i < size; i++) {
        assert(fputc((int)(i & 0xff), raw) != EOF);
    }
    assert(fclose(raw) == 0);
}

/* Un fichier vide est refuse et n'est pas une capture de zero frame :
 * zero est un multiple de 8 192, donc sans ce refus explicite il
 * passerait la validation de taille et le visualiseur ouvrirait une
 * fenetre sur rien. */
static void test_empty_file_is_refused(void) {
    const char *path = "tmp/test_screencast_empty.bin";
    pico8_screencast_reader_t reader;

    write_raw_bytes(path, 0);

    assert(pico8_screencast_reader_open(&reader, path) == PICO8_SCREENCAST_EMPTY);
    assert(pico8_screencast_frame_count(&reader) == 0);

    pico8_screencast_reader_close(&reader);
    remove(path);
}

/* Une taille qui n'est pas un multiple de la taille d'une frame decrit
 * une frame incomplete : fichier tronque, ou ecriture interrompue. Le
 * refuser a l'ouverture est ce qui empeche de confondre un fichier
 * incomplet avec une session courte. Les deux tailles testees sont les
 * deux formes du cas : 8191 = moins d'une frame, 2 x 8192 + 1 = des
 * frames entieres suivies d'un reste. */
static void test_size_that_is_not_a_whole_number_of_frames_is_refused(void) {
    const char *short_path = "tmp/test_screencast_short.bin";
    const char *ragged_path = "tmp/test_screencast_ragged.bin";
    pico8_screencast_reader_t reader;

    write_raw_bytes(short_path, 8191);
    assert(pico8_screencast_reader_open(&reader, short_path) == PICO8_SCREENCAST_TRUNCATED);
    assert(pico8_screencast_frame_count(&reader) == 0);
    pico8_screencast_reader_close(&reader);
    remove(short_path);

    write_raw_bytes(ragged_path, 2L * 8192L + 1L);
    assert(pico8_screencast_reader_open(&reader, ragged_path) == PICO8_SCREENCAST_TRUNCATED);
    assert(pico8_screencast_frame_count(&reader) == 0);
    pico8_screencast_reader_close(&reader);
    remove(ragged_path);
}

/* Un echec d'ecriture est rendu a l'appelant plutot que subi en
 * silence, et aucun fichier n'est laisse derriere : un fichier partiel
 * produirait une empreinte fausse, indiscernable d'une regression. */
static void test_writer_reports_a_path_it_cannot_open(void) {
    const char *path = "tmp/test_screencast_no_such_dir/capture.bin";
    pico8_screencast_writer_t writer;
    pico8_screencast_reader_t reader;

    assert(pico8_screencast_writer_open(&writer, path) == 0);
    assert(pico8_screencast_reader_open(&reader, path) == PICO8_SCREENCAST_UNREADABLE);

    pico8_screencast_reader_close(&reader);
}

/* Lit tout le fichier et rend sa taille. */
static long read_whole_file(const char *path, unsigned char *out, long capacity) {
    FILE *raw = fopen(path, "rb");
    assert(raw != NULL);
    size_t got = fread(out, 1, (size_t)capacity, raw);
    assert(fclose(raw) == 0);
    return (long)got;
}

/* Un aller-retour par la lecture ne change pas un octet du fichier.
 * C'est la propriete dont depend l'empreinte de `make e2e` : le
 * jour ou une lecture modifierait le fichier qu'elle lit, regarder une
 * capture suffirait a la faire diverger. */
static void test_reading_leaves_the_file_byte_for_byte_unchanged(void) {
    const char *path = "tmp/test_screencast_readonly.bin";
    pico8_screencast_reader_t reader;
    unsigned char frame[PICO8_SCREENCAST_FRAME_PIXELS];
    static unsigned char before[3 * 8192];
    static unsigned char after[sizeof(before)];

    write_seeded_frames(path, 3);
    assert(read_whole_file(path, before, (long)sizeof(before)) == (long)sizeof(before));

    assert(pico8_screencast_reader_open(&reader, path) == PICO8_SCREENCAST_OK);
    assert(pico8_screencast_read_frame(&reader, 2, frame) == 1);
    assert(pico8_screencast_read_frame(&reader, 0, frame) == 1);
    assert(pico8_screencast_read_frame(&reader, 1, frame) == 1);
    pico8_screencast_reader_close(&reader);

    assert(read_whole_file(path, after, (long)sizeof(after)) == (long)sizeof(after));
    assert(memcmp(before, after, sizeof(before)) == 0);

    remove(path);
}

/* Demander une frame que le fichier n'a pas est refuse, et le tampon de
 * l'appelant n'est pas touche : sans ce refus, le visualiseur qui
 * depasse la fin afficherait le contenu precedent de son tampon, c'est
 * a dire la derniere frame lue, et une capture finie ressemblerait a
 * une capture figee. */
static void test_frame_index_past_the_end_is_refused(void) {
    const char *path = "tmp/test_screencast_past_end.bin";
    pico8_screencast_reader_t reader;
    unsigned char frame[PICO8_SCREENCAST_FRAME_PIXELS];

    write_seeded_frames(path, 2);

    assert(pico8_screencast_reader_open(&reader, path) == PICO8_SCREENCAST_OK);
    assert(pico8_screencast_read_frame(&reader, 1, frame) == 1);

    assert(pico8_screencast_read_frame(&reader, 2, frame) == 0);
    assert(pico8_screencast_read_frame(&reader, 4242, frame) == 0);

    /* Le tampon tient toujours la derniere frame reellement lue. */
    assert(pixel_at(frame, 0, 0) == expected_pixel(1, 0, 0));
    assert(pixel_at(frame, 127, 127) == expected_pixel(1, 127, 127));

    pico8_screencast_reader_close(&reader);
    remove(path);
}

/* Ecrire une frame ne touche pas a la memoire du jeu, ni l'ecran ni le
 * reste : une capture qui modifierait ce qu'elle observe invaliderait
 * tout l'instrument. Le `const` de la signature le dit deja au
 * compilateur ; ce test le dit sur toute la memoire, y compris ce qu'un
 * futur contournement du `const` atteindrait. L'octet pose hors ecran
 * (PICO8_ADDR_MAP) est ce qui donne a l'assertion prise sur le reste de
 * la memoire et pas seulement sur une zone laissee a zero. */
static void test_writing_a_frame_leaves_the_game_memory_untouched(void) {
    const char *path = "tmp/test_screencast_readonly_memory.bin";
    pico8_screencast_writer_t writer;
    static p8_memory_t mem;
    static p8_memory_t before;

    p8_mem_init(&mem);
    paint_frame(&mem, 7);
    p8_poke(&mem, P8_ADDR_MAP, 0x42);
    before = mem;

    assert(pico8_screencast_writer_open(&writer, path) == 1);
    assert(pico8_screencast_write_frame(&writer, &mem) == 1);
    assert(pico8_screencast_writer_close(&writer) == 1);

    assert(memcmp(&before, &mem, sizeof(mem)) == 0);

    remove(path);
}

/* Le format vu depuis le dehors du projet : l'octet d'indice k tient le
 * pixel 2k dans son quartet de poids faible et le pixel 2k+1 dans celui
 * de poids fort. D'ou les valeurs asserties : pixels 0x03 et 0x0c
 * donnent l'octet 0xc3, pixels 0x0f et 0x00 donnent 0x0f. Un
 * aller-retour ne peut pas prouver cet ordre -- ecrire et relire avec
 * les quartets intervertis des deux cotes rendrait les memes pixels --
 * et c'est pourtant ce qui est promis a un script jetable qui lirait le
 * fichier sans dependre du code du projet. */
static void test_on_disk_byte_holds_the_even_pixel_in_its_low_nibble(void) {
    const char *path = "tmp/test_screencast_nibbles.bin";
    pico8_screencast_writer_t writer;
    p8_memory_t mem;
    static unsigned char raw[8192];

    p8_mem_init(&mem);
    paint_frame(&mem, 0);
    p8_poke(&mem, P8_ADDR_SCREEN + 0, 0x03);
    p8_poke(&mem, P8_ADDR_SCREEN + 1, 0x0c);
    p8_poke(&mem, P8_ADDR_SCREEN + 2, 0x0f);
    p8_poke(&mem, P8_ADDR_SCREEN + 3, 0x00);

    assert(pico8_screencast_writer_open(&writer, path) == 1);
    assert(pico8_screencast_write_frame(&writer, &mem) == 1);
    assert(pico8_screencast_writer_close(&writer) == 1);

    assert(read_whole_file(path, raw, (long)sizeof(raw)) == (long)sizeof(raw));
    assert(raw[0] == 0xc3);
    assert(raw[1] == 0x0f);

    remove(path);
}

int main(void) {
    test_written_frame_reads_back_pixel_for_pixel();
    test_frame_count_is_file_size_divided_by_frame_size();
    test_each_frame_holds_its_own_pixels_in_order();
    test_frames_can_be_read_out_of_sequence();
    test_missing_file_is_refused();
    test_empty_file_is_refused();
    test_size_that_is_not_a_whole_number_of_frames_is_refused();
    test_writer_reports_a_path_it_cannot_open();
    test_reading_leaves_the_file_byte_for_byte_unchanged();
    test_frame_index_past_the_end_is_refused();
    test_writing_a_frame_leaves_the_game_memory_untouched();
    test_on_disk_byte_holds_the_even_pixel_in_its_low_nibble();
    printf("test_pico8_screencast: all tests passed\n");
    return 0;
}
