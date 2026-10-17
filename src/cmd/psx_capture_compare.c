/* usage: psx_capture_compare <capture.vram> <pc_frames.bin> <index> <label> <frame> <mask-hex> <calls> <out-dir>
 * Compares one GPU capture with frame <index> of the PC capture file, prints the report line and
 * writes <out-dir>/<label>-f<frame>.diff.txt and .ppm. Exit 0 whatever the pixels say; exit 1 only
 * on unreadable or malformed input, naming it. */

#include "devtools/capture_compare.h"
#include "devtools/capture_list.h"
#include "devtools/screencast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char capture[CAPTURE_COMPARE_PIXEL_BYTES + 64];
static unsigned char pc[SCREENCAST_FRAME_PIXELS];
static capture_compare_image_t psx;
static char map[CAPTURE_COMPARE_DIFF_MAP_BYTES];
static unsigned char ppm[CAPTURE_COMPARE_PPM_BYTES];

static int fail(const char *what, const char *path) {
    fprintf(stderr, "psx_capture_compare: %s: %s\n", path, what);
    return 1;
}

static int parse_number(const char *text, int base, long *out) {
    char *end;
    long value = strtol(text, &end, base);
    if (end == text || *end != '\0' || value < 0) return 0;
    *out = value;
    return 1;
}

static int write_file(const char *path, const void *bytes, size_t size) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) return 0;
    int written = fwrite(bytes, 1, size, file) == size;
    return fclose(file) == 0 && written;
}

int main(int argc, char **argv) {
    if (argc != 9) {
        fprintf(stderr, "usage: psx_capture_compare <capture.vram> <pc_frames.bin> <index> <label> <frame> <mask-hex> "
                        "<calls> <out-dir>\n");
        return 1;
    }
    const char *capture_path = argv[1];
    const char *pc_path = argv[2];
    const char *label = argv[4];
    long index, frame, mask, calls;
    if (!parse_number(argv[3], 10, &index)) return fail("index is not a number", argv[3]);
    if (!parse_number(argv[5], 10, &frame)) return fail("frame is not a number", argv[5]);
    if (!parse_number(argv[6], 16, &mask)
        || ((unsigned long)mask > DRAW_FILTER_ALL && (unsigned long)mask != CAPTURE_LIST_DISPLAY_MASK)) {
        return fail("mask is not a hex mask", argv[6]);
    }
    if (!parse_number(argv[7], 10, &calls)) return fail("calls is not a number", argv[7]);

    FILE *file = fopen(capture_path, "rb");
    if (file == NULL) return fail("unreadable capture", capture_path);
    size_t size = fread(capture, 1, sizeof capture, file);
    fclose(file);
    char error[CAPTURE_COMPARE_ERROR_CAP];
    if (!capture_compare_decode(capture, size, &psx, error)) return fail(error, capture_path);

    screencast_reader_t reader;
    if (screencast_reader_open(&reader, pc_path) != SCREENCAST_OK) return fail("unreadable PC frames", pc_path);
    int read = screencast_read_frame(&reader, (int)index, pc);
    screencast_reader_close(&reader);
    if (!read) return fail("no such PC frame index", pc_path);

    capture_compare_metrics_t metrics = capture_compare_measure(pc, &psx);
    char names[CAPTURE_LIST_NAMES_CAP];
    capture_list_mask_names((unsigned)mask, names);
    char line[CAPTURE_COMPARE_REPORT_CAP];
    capture_compare_report_line(line, label, (int)frame, names, (int)calls, &metrics);

    char path[1024];
    capture_compare_diff_map(pc, &psx, map);
    snprintf(path, sizeof path, "%s/%s-f%ld.diff.txt", argv[8], label, frame);
    if (!write_file(path, map, sizeof map)) return fail("cannot write", path);
    capture_compare_image(pc, &psx, ppm);
    snprintf(path, sizeof path, "%s/%s-f%ld.ppm", argv[8], label, frame);
    if (!write_file(path, ppm, sizeof ppm)) return fail("cannot write", path);
    printf("%s\n", line);
    return 0;
}
