/* tools/psx_pad_protocol_check.sh: the pad script's scratchpad words and magics agree between the
 * C header the probe and the game share and tools/psx_pad_play.lua, and a drift is named. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_psx_pad_protocol_work"
#define LOG DIR "/log.txt"
#define HEADER "src/platform/psx_pad_protocol.h"
#define LUA "tools/psx_pad_play.lua"

static const char *read_file(const char *path) {
    static char buf[1 << 14];
    buf[0] = '\0';
    FILE *file = fopen(path, "rb");
    if (file == NULL) return buf;
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static void test_the_committed_files_agree(void) {
    assert(system("sh tools/psx_pad_protocol_check.sh " HEADER " " LUA " > " LOG " 2>&1") == 0);
    assert(strstr(read_file(LOG), "psx-pad-protocol: ok") != NULL);
}

static void test_a_drifted_or_missing_value_is_named(void) {
    assert(system("sed 's/^local END_MAGIC = .*/local END_MAGIC = 0x1234/' " LUA " > " DIR "/drift.lua") == 0);
    assert(system("grep -v '^#define PSX_PAD_HANDSHAKE_OFFSET' " HEADER " > " DIR "/missing.h") == 0);
    assert(system("sh tools/psx_pad_protocol_check.sh " DIR "/missing.h " DIR "/drift.lua > " LOG " 2>&1") != 0);
    assert(strstr(read_file(LOG), "END_MAGIC differs") != NULL);
    assert(strstr(read_file(LOG), "HANDSHAKE_OFFSET missing") != NULL);
}

int main(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    test_the_committed_files_agree();
    test_a_drifted_or_missing_value_is_named();
    printf("test_psx_pad_protocol: ok\n");
    return 0;
}
