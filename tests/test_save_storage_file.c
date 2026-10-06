#define _POSIX_C_SOURCE 200809L

#include "game/save_state.h"
#include "game/save_storage.h"

#include "save_storage_contract.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char g_work_dir[] = "tmp/test_save_storage_file_XXXXXX";

static void enter_scratch_directory(void) {
    assert(mkdtemp(g_work_dir) != NULL);
    assert(chdir(g_work_dir) == 0);
}

/* remove() takes a file or an empty directory: every case below leaves at
 * most these three paths. */
static void clear_save_directory(void) {
    remove("save/slipways.sav.tmp");
    remove("save/slipways.sav");
    remove("save");
}

static void leave_scratch_directory(void) {
    clear_save_directory();
    assert(chdir("../..") == 0);
    assert(remove(g_work_dir) == 0 && "the scratch directory must be empty at the end");
}

static uint8_t g_bytes[SAVE_STATE_MAX_BYTES + 2];
static uint8_t g_read[SAVE_STATE_MAX_BYTES];

static int path_exists(const char *path) {
    struct stat info;
    return stat(path, &info) == 0;
}

static void write_raw_file(const char *path, const uint8_t *bytes, int count) {
    FILE *file = fopen(path, "wb");
    assert(file != NULL);
    assert(fwrite(bytes, 1, (size_t)count, file) == (size_t)count);
    assert(fclose(file) == 0);
}

/* Static buffer, overwritten by each call; *count receives the file size. */
static const uint8_t *read_raw_file(const char *path, int *count) {
    static uint8_t buf[SAVE_STATE_MAX_BYTES + 2];
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    *count = (int)fread(buf, 1, sizeof buf, file);
    assert(fclose(file) == 0);
    return buf;
}

static void test_first_write_creates_the_save_directory_and_file(void) {
    clear_save_directory();
    assert(!path_exists("save"));
    memcpy(g_bytes, "SLWY\x01\x00\xff", 7);
    assert(save_storage_write(g_bytes, 7) == 1);
    int count;
    const uint8_t *file_bytes = read_raw_file("save/slipways.sav", &count);
    assert(count == 7);
    assert(memcmp(file_bytes, g_bytes, 7) == 0);
}

static void test_a_successful_write_leaves_no_temporary_file(void) {
    clear_save_directory();
    g_bytes[0] = 0x5A;
    assert(save_storage_write(g_bytes, 1) == 1);
    assert(!path_exists("save/slipways.sav.tmp"));
}

static void test_a_stale_temporary_file_is_ignored(void) {
    clear_save_directory();
    memset(g_bytes, 0x33, 10);
    assert(save_storage_write(g_bytes, 10) == 1);
    memset(g_bytes, 0x77, 20);
    write_raw_file("save/slipways.sav.tmp", g_bytes, 20);
    assert(save_storage_read(g_read, SAVE_STATE_MAX_BYTES) == 10);
    assert(g_read[0] == 0x33 && g_read[9] == 0x33);
}

static void test_an_empty_or_oversize_file_reads_as_no_save(void) {
    clear_save_directory();
    assert(mkdir("save", 0777) == 0);
    write_raw_file("save/slipways.sav", g_bytes, 0);
    assert(save_storage_read(g_read, SAVE_STATE_MAX_BYTES) == 0);
    memset(g_bytes, 0x44, SAVE_STATE_MAX_BYTES + 1);
    write_raw_file("save/slipways.sav", g_bytes, SAVE_STATE_MAX_BYTES + 1);
    assert(save_storage_read(g_read, SAVE_STATE_MAX_BYTES) == 0);
}

/* A directory in place of the temporary file makes fopen fail even as root,
 * where a permission trick would not. */
static void test_a_failed_write_keeps_the_previous_save(void) {
    clear_save_directory();
    memset(g_bytes, 0x61, 5);
    assert(save_storage_write(g_bytes, 5) == 1);
    assert(mkdir("save/slipways.sav.tmp", 0777) == 0);
    memset(g_bytes, 0x62, 9);
    assert(save_storage_write(g_bytes, 9) == 0);
    int count;
    const uint8_t *file_bytes = read_raw_file("save/slipways.sav", &count);
    assert(count == 5);
    assert(file_bytes[0] == 0x61 && file_bytes[4] == 0x61);
}

static void test_a_regular_file_named_save_fails_the_write(void) {
    clear_save_directory();
    write_raw_file("save", g_bytes, 3);
    assert(save_storage_write(g_bytes, 3) == 0);
}

static void test_erase_without_a_file_succeeds(void) {
    clear_save_directory();
    assert(save_storage_erase() == 1);
    assert(mkdir("save", 0777) == 0);
    assert(save_storage_erase() == 1);
}

int main(void) {
    enter_scratch_directory();
    save_storage_contract_run();
    test_first_write_creates_the_save_directory_and_file();
    test_a_successful_write_leaves_no_temporary_file();
    test_a_stale_temporary_file_is_ignored();
    test_an_empty_or_oversize_file_reads_as_no_save();
    test_a_failed_write_keeps_the_previous_save();
    test_a_regular_file_named_save_fails_the_write();
    test_erase_without_a_file_succeeds();
    leave_scratch_directory();
    printf("test_save_storage_file: ok\n");
    return 0;
}
