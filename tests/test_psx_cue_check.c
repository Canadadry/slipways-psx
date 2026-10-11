/* tools/psx_cue_check.sh, the cue condition of psx-hello and of the game's
 * disc, on crafted .cue/.bin pairs: no emulator, no toolchain. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/psx_cue_check.sh"
#define DIR "tmp/test_psx_cue_check_work"
#define CUE DIR "/disc/game.cue"
#define BIN DIR "/disc/game.bin"
#define LOG DIR "/log.txt"
#define SECTOR 2352

#define CUE_TEXT \
    "FILE \"game.bin\" BINARY\n" \
    "  TRACK 01 MODE2/2352\n" \
    "    INDEX 01 00:00:00\n"

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

static void write_bin(const char *path, size_t bytes) {
    FILE *file = fopen(path, "wb");
    assert(file != NULL);
    for (size_t i = 0; i < bytes; i++) {
        fputc((int)(i & 0xff), file);
    }
    fclose(file);
}

/* Static buffer, overwritten by each call; empty when the file is missing. */
static const char *read_file(const char *path) {
    static char buf[1 << 12];
    buf[0] = '\0';
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return buf;
    }
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static void fresh_dir(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR "/disc") == 0);
}

static int run(const char *cue) {
    char command[512];
    snprintf(command, sizeof(command), SCRIPT " %s > " LOG " 2>&1", cue);
    return system(command);
}

static void test_whole_sectors_pass_and_print_the_bin(void) {
    fresh_dir();
    write_file(CUE, CUE_TEXT);
    write_bin(BIN, 2 * SECTOR);
    assert(run(CUE) == 0);
    assert(strcmp(read_file(LOG), BIN "\n") == 0);
}

static void test_cue_without_file_line_fails(void) {
    fresh_dir();
    write_file(CUE, "  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n");
    write_bin(BIN, SECTOR);
    assert(run(CUE) != 0);
    assert(strcmp(read_file(LOG), CUE " has no FILE \"<bin>\" line\n") == 0);
}

static void test_missing_bin_fails_naming_it(void) {
    fresh_dir();
    write_file(CUE, CUE_TEXT);
    assert(run(CUE) != 0);
    assert(strcmp(read_file(LOG), BIN ", named by " CUE ", is missing or empty\n") == 0);
}

static void test_empty_bin_fails_naming_it(void) {
    fresh_dir();
    write_file(CUE, CUE_TEXT);
    write_bin(BIN, 0);
    assert(run(CUE) != 0);
    assert(strcmp(read_file(LOG), BIN ", named by " CUE ", is missing or empty\n") == 0);
}

static void test_partial_sector_fails_naming_the_size(void) {
    fresh_dir();
    write_file(CUE, CUE_TEXT);
    write_bin(BIN, SECTOR + 1);
    assert(run(CUE) != 0);
    assert(strcmp(read_file(LOG), BIN " is 2353 bytes, not a whole number of 2352-byte sectors\n") == 0);
}

/* A game.bin in the current directory must not satisfy a .cue elsewhere. */
static void test_relative_bin_resolves_next_to_the_cue(void) {
    fresh_dir();
    write_file(CUE, CUE_TEXT);
    write_bin(DIR "/game.bin", SECTOR);
    char command[512];
    snprintf(command, sizeof(command), "cd " DIR " && sh ../../tools/psx_cue_check.sh disc/game.cue > log.txt 2>&1");
    assert(system(command) != 0);
    assert(strcmp(read_file(LOG), "disc/game.bin, named by disc/game.cue, is missing or empty\n") == 0);
    write_bin(BIN, SECTOR);
    assert(system(command) == 0);
    assert(strcmp(read_file(LOG), "disc/game.bin\n") == 0);
}

static void test_wrong_argument_count_exits_2(void) {
    fresh_dir();
    int status = system(SCRIPT " > " LOG " 2>&1");
    assert(WEXITSTATUS(status) == 2);
    assert(strstr(read_file(LOG), "usage: psx_cue_check.sh <cue>") != NULL);
}

int main(void) {
    test_whole_sectors_pass_and_print_the_bin();
    test_cue_without_file_line_fails();
    test_missing_bin_fails_naming_it();
    test_empty_bin_fails_naming_it();
    test_partial_sector_fails_naming_the_size();
    test_relative_bin_resolves_next_to_the_cue();
    test_wrong_argument_count_exits_2();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_cue_check: all tests passed\n");
    return 0;
}
