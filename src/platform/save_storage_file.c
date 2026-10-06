#define _POSIX_C_SOURCE 200809L

#include "game/save_storage.h"

#include "game/save_state.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#define SAVE_DIRECTORY "save"
#define SAVE_FILE SAVE_DIRECTORY "/slipways.sav"
#define SAVE_TEMPORARY SAVE_FILE ".tmp"

static int write_temporary(const uint8_t *bytes, int count) {
    FILE *file = fopen(SAVE_TEMPORARY, "wb");
    if (file == NULL) return 0;
    size_t written = fwrite(bytes, 1, (size_t)count, file);
    int closed = fclose(file) == 0;
    return written == (size_t)count && closed;
}

int save_storage_write(const uint8_t *bytes, int count) {
    assert(count >= 1 && count <= SAVE_STATE_MAX_BYTES && "to clear the save, call save_storage_erase");
    if (mkdir(SAVE_DIRECTORY, 0777) != 0 && errno != EEXIST) return 0;
    /* The rename comes only after a complete, closed write: a crash before it
     * leaves the previous save in place, never a torn one. */
    if (!write_temporary(bytes, count) || rename(SAVE_TEMPORARY, SAVE_FILE) != 0) {
        remove(SAVE_TEMPORARY);
        return 0;
    }
    return 1;
}

int save_storage_read(uint8_t *bytes, int cap) {
    assert(cap >= SAVE_STATE_MAX_BYTES && "a smaller buffer would lose the end of a record");
    FILE *file = fopen(SAVE_FILE, "rb");
    if (file == NULL) return 0;
    size_t count = fread(bytes, 1, SAVE_STATE_MAX_BYTES, file);
    /* The byte past the cap is probed with fgetc, not read into bytes, which
     * may hold exactly SAVE_STATE_MAX_BYTES. */
    int oversize = fgetc(file) != EOF;
    int failed = ferror(file);
    fclose(file);
    if (failed || oversize) return 0;
    return (int)count;
}

int save_storage_erase(void) {
    return remove(SAVE_FILE) == 0 || errno == ENOENT;
}
