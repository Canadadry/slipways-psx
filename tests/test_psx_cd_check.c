/* tools/psx_cd_check.sh, the cue and cd-exe conditions of the game's disc, on
 * crafted files with a stub dumpsxiso: no emulator, no toolchain. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/psx_cd_check.sh"
#define DIR "tmp/test_psx_cd_check_work"
#define CUE DIR "/build/slipways.cue"
#define BIN DIR "/build/slipways.bin"
#define EXE DIR "/build/psx-game.exe"
#define STUB DIR "/dumpsxiso"
#define DISC_EXE DIR "/disc-copy.exe"
#define OUT DIR "/out"
#define LOG DIR "/log.txt"
#define SECTOR 2352
#define EXE_BYTES 4096

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

static void write_bytes(const char *path, size_t bytes, size_t flipped_offset) {
    FILE *file = fopen(path, "wb");
    assert(file != NULL);
    for (size_t i = 0; i < bytes; i++) {
        int byte = (int)((i * 7) & 0xff);
        fputc(i == flipped_offset ? byte ^ 0xff : byte, file);
    }
    fclose(file);
}

/* Static buffer, overwritten by each call; empty when the file is missing. */
static const char *read_file(const char *path) {
    static char buf[1 << 14];
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

/* The stub takes dumpsxiso's `-x <dir> <bin>` and copies DISC_EXE into <dir>
 * as `name`, or prints `failure` on stderr and exits 1 when it is not NULL. */
static void write_stub(const char *name, const char *failure) {
    char text[1024];
    if (failure != NULL) {
        snprintf(text, sizeof(text), "#!/bin/sh\necho '%s' >&2\nexit 1\n", failure);
    } else {
        snprintf(text, sizeof(text),
                 "#!/bin/sh\n"
                 "[ \"$1\" = -x ] && [ -f \"$3\" ] || { echo \"stub: bad arguments: $*\" >&2; exit 1; }\n"
                 "mkdir -p \"$2\" && cp " DISC_EXE " \"$2/%s\" && echo SYSTEM > \"$2/SYSTEM.CNF\"\n",
                 name);
    }
    write_file(STUB, text);
    assert(system("chmod +x " STUB) == 0);
}

/* A disc of two sectors, a game EXE, and a stub extracting an exact copy. */
static void setup_passing(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR "/build") == 0);
    write_file(CUE, "FILE \"slipways.bin\" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n");
    write_bytes(BIN, 2 * SECTOR, (size_t)-1);
    write_bytes(EXE, EXE_BYTES, (size_t)-1);
    write_bytes(DISC_EXE, EXE_BYTES, (size_t)-1);
    write_stub("SLIPWAYS.EXE", NULL);
}

static int run_with(const char *dumpsxiso) {
    char command[512];
    snprintf(command, sizeof(command), SCRIPT " " CUE " " EXE " SLIPWAYS.EXE %s " OUT " > " LOG " 2>&1",
             dumpsxiso);
    return system(command);
}

static int run(void) {
    return run_with(STUB);
}

static void test_exact_copy_on_disc_passes_both(void) {
    setup_passing();
    assert(run() == 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx-cd-check: cue: ok, " BIN ", 2 sectors\n") != NULL);
    assert(strstr(log, "psx-cd-check: cd-exe: ok, SLIPWAYS.EXE, 4096 bytes\n") != NULL);
}

static void test_changed_byte_on_disc_names_its_offset(void) {
    setup_passing();
    write_bytes(DISC_EXE, EXE_BYTES, 1000);
    assert(run() != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx-cd-check: cue: ok") != NULL);
    assert(strstr(log, "psx-cd-check: cd-exe: " OUT "/disc/SLIPWAYS.EXE differs from " EXE " at byte 1001") != NULL);
    assert(strstr(log, "cd-exe: ok") == NULL);
}

static void test_other_name_on_disc_lists_the_extracted_names(void) {
    setup_passing();
    write_stub("PSXGAME.EXE", NULL);
    assert(run() != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx-cd-check: cd-exe: no SLIPWAYS.EXE on the disc; extracted: PSXGAME.EXE SYSTEM.CNF\n") !=
           NULL);
}

/* A shorter copy has no differing byte: cmp reports the end of file instead. */
static void test_truncated_copy_on_disc_fails(void) {
    setup_passing();
    write_bytes(DISC_EXE, EXE_BYTES - 1, (size_t)-1);
    assert(run() != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx-cd-check: cd-exe: " OUT "/disc/SLIPWAYS.EXE differs from " EXE ": cmp: EOF on " OUT
                       "/disc/SLIPWAYS.EXE") != NULL);
}

static void test_failing_dumpsxiso_is_quoted(void) {
    setup_passing();
    write_stub(NULL, "ERROR: File does not contain a valid ISO9660 file system.");
    assert(run() != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx-cd-check: cd-exe: " STUB " exited with 1: ERROR: File does not contain a valid ISO9660 "
                       "file system.\n") != NULL);
}

/* dumpsxiso 2.03 prints its ERROR line on stdout, not stderr. */
static void test_dumpsxiso_error_on_stdout_is_quoted(void) {
    setup_passing();
    write_file(STUB, "#!/bin/sh\necho 'DUMPSXISO 2.03'\necho\necho 'ERROR: Cannot open file x.bin...'\nexit 1\n");
    assert(run() != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx-cd-check: cd-exe: " STUB " exited with 1: ERROR: Cannot open file x.bin...\n") != NULL);
}

static void test_missing_dumpsxiso_names_the_setup(void) {
    setup_passing();
    assert(run_with(DIR "/no-such-dumpsxiso") != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx-cd-check: cd-exe: " DIR "/no-such-dumpsxiso is missing: run make psn00bsdk-toolchain-setup\n") !=
           NULL);
}

static void test_failed_cue_skips_and_fails_cd_exe(void) {
    setup_passing();
    write_bytes(BIN, SECTOR + 1, (size_t)-1);
    assert(WEXITSTATUS(run()) == 1);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx-cd-check: cue: " BIN " is 2353 bytes, not a whole number of 2352-byte sectors\n") != NULL);
    assert(strstr(log, "psx-cd-check: cd-exe: skipped: cue failed\n") != NULL);
    assert(strstr(log, ": ok") == NULL);
}

static void test_wrong_argument_count_exits_2(void) {
    setup_passing();
    assert(WEXITSTATUS(system(SCRIPT " " CUE " " EXE " SLIPWAYS.EXE " STUB " > " LOG " 2>&1")) == 2);
    assert(strstr(read_file(LOG), "usage: psx_cd_check.sh <cue> <exe> <disc-name> <dumpsxiso> <out-dir>") != NULL);
}

int main(void) {
    test_exact_copy_on_disc_passes_both();
    test_changed_byte_on_disc_names_its_offset();
    test_other_name_on_disc_lists_the_extracted_names();
    test_truncated_copy_on_disc_fails();
    test_failing_dumpsxiso_is_quoted();
    test_dumpsxiso_error_on_stdout_is_quoted();
    test_missing_dumpsxiso_names_the_setup();
    test_failed_cue_skips_and_fails_cd_exe();
    test_wrong_argument_count_exits_2();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_cd_check: all tests passed\n");
    return 0;
}
