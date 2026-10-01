/* Outil d'hôte du minimiseur, lu et écrit par pico8_input :
 *   pico8_minimize mechanical <entrée> <sortie>   l'étage mécanique
 *   pico8_minimize drop <entrée> <sortie> <première> <nombre>
 *                                                 retire ces frames
 *   pico8_minimize rewrite <entrée> <sortie>      les mêmes frames, écrites
 *                                                 par pico8_input_save
 *   pico8_minimize count <entrée>                 le nombre de frames
 * Sur échec : code non nul, message sur stderr, <sortie> non écrite. drop
 * échoue sur une plage hors du fichier ou qui le viderait. */

#include "p8/input.h"
#include "devtools/minimize.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_int(const char *text, int *out) {
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value < 0 || value > INT_MAX) {
        return 0;
    }
    *out = (int)value;
    return 1;
}

static int load(pico8_input_t *input, const char *path) {
    pico8_input_init(input);
    if (!pico8_input_load(input, path)) {
        fprintf(stderr, "pico8_minimize: %s absent ou malformé\n", path);
        return 0;
    }
    return 1;
}

static int save(const pico8_input_frame_t *frames, int n, const char *path) {
    if (!pico8_input_save(frames, n, path)) {
        fprintf(stderr, "pico8_minimize: écriture de %s impossible\n", path);
        return 0;
    }
    return 1;
}

static int mechanical(const char *in_path, const char *out_path) {
    pico8_input_t input;
    if (!load(&input, in_path)) {
        return 1;
    }
    pico8_input_frame_t *out = malloc(sizeof *out * (size_t)input.frame_count);
    int ok = out != NULL && save(out, pico8_minimize_mechanical(input.frames, input.frame_count, out), out_path);
    free(out);
    pico8_input_free(&input);
    return !ok;
}

static int drop(const char *in_path, const char *out_path, const char *first_text, const char *count_text) {
    int first;
    int count;
    if (!parse_int(first_text, &first) || !parse_int(count_text, &count)) {
        fprintf(stderr, "pico8_minimize: plage invalide: %s %s\n", first_text, count_text);
        return 1;
    }
    pico8_input_t input;
    if (!load(&input, in_path)) {
        return 1;
    }
    int n = input.frame_count;
    if (count == 0 || first >= n || count > n - first || count == n) {
        fprintf(stderr, "pico8_minimize: plage %d+%d hors des %d frames de %s ou qui le viderait\n", first, count,
                n, in_path);
        pico8_input_free(&input);
        return 1;
    }
    memmove(&input.frames[first], &input.frames[first + count],
            sizeof *input.frames * (size_t)(n - first - count));
    int ok = save(input.frames, n - count, out_path);
    pico8_input_free(&input);
    return !ok;
}

static int rewrite(const char *in_path, const char *out_path) {
    pico8_input_t input;
    if (!load(&input, in_path)) {
        return 1;
    }
    int ok = save(input.frames, input.frame_count, out_path);
    pico8_input_free(&input);
    return !ok;
}

static int count_frames(const char *in_path) {
    pico8_input_t input;
    if (!load(&input, in_path)) {
        return 1;
    }
    printf("%d\n", input.frame_count);
    pico8_input_free(&input);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 4 && strcmp(argv[1], "mechanical") == 0) {
        return mechanical(argv[2], argv[3]);
    }
    if (argc == 6 && strcmp(argv[1], "drop") == 0) {
        return drop(argv[2], argv[3], argv[4], argv[5]);
    }
    if (argc == 4 && strcmp(argv[1], "rewrite") == 0) {
        return rewrite(argv[2], argv[3]);
    }
    if (argc == 3 && strcmp(argv[1], "count") == 0) {
        return count_frames(argv[2]);
    }
    fprintf(stderr, "usage: pico8_minimize mechanical <entrée> <sortie>\n"
                    "       pico8_minimize drop <entrée> <sortie> <première> <nombre>\n"
                    "       pico8_minimize rewrite <entrée> <sortie>\n"
                    "       pico8_minimize count <entrée>\n");
    return 1;
}
