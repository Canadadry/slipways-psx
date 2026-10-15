/* tools/psx_opt_level.sh, the effective optimization level of a CMake per-target flags.make, on hand-written
 * flags files. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/psx_opt_level.sh"
#define DIR "tmp/test_psx_opt_level_work"
#define FLAGS DIR "/flags.make"
#define LOG DIR "/log.txt"

#define HEADER                                                \
    "# CMAKE generated file: DO NOT EDIT!\n"                  \
    "C_DEFINES = -DNDEBUG=1 -DPSN00BSDK=1\n\n"                \
    "C_INCLUDES = -I/sdk/include/libpsn00b -I/repo/src\n\n"

/* The SDK's real shape: CMake's release -O3 first, the psn00bsdk interface's -O2 near the end. */
#define SDK_FLAGS                                                                                         \
    "C_FLAGS = -O3 -DNDEBUG -std=c99 -Wall -Wextra -Werror -g -ffreestanding -fno-builtin -msoft-float " \
    "-march=r3000 -mtune=r3000 -mabi=32 -O2 -G8 -fno-pic -mno-abicalls -mgpopt -mno-extern-sdata"

static void write_file(const char *path, const char *text) {
    FILE *f = fopen(path, "w");
    assert(f != NULL);
    assert(fputs(text, f) >= 0);
    assert(fclose(f) == 0);
}

static int run_on(const char *flags_file_text) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(FLAGS, flags_file_text);
    int status = system(SCRIPT " " FLAGS " > " LOG " 2>&1");
    assert(WIFEXITED(status));
    return WEXITSTATUS(status);
}

static void assert_log(const char *expected) {
    FILE *f = fopen(LOG, "r");
    assert(f != NULL);
    char buf[512];
    size_t len = fread(buf, 1, sizeof buf - 1, f);
    buf[len] = '\0';
    fclose(f);
    if (strcmp(buf, expected) != 0) {
        fprintf(stderr, "expected '%s', got '%s'\n", expected, buf);
        assert(0);
    }
}

static void test_the_last_o_token_wins_over_the_release_o3(void) {
    assert(run_on(HEADER SDK_FLAGS "\n") == 0);
    assert_log("O2\n");
}

static void test_an_os_or_o3_put_after_the_sdk_flags_reads_as_such(void) {
    assert(run_on(HEADER SDK_FLAGS " -Os -Wno-error\n") == 0);
    assert_log("Os\n");
    assert(run_on(HEADER SDK_FLAGS " -O3 -Wno-error\n") == 0);
    assert_log("O3\n");
}

static void test_no_o_token_exits_2(void) {
    assert(run_on(HEADER "C_FLAGS = -std=c99 -Wall -G8\n") == 2);
}

static void test_no_c_flags_line_exits_2(void) {
    assert(run_on(HEADER "C_FLAGZ = -O2\n") == 2);
}

static void test_a_missing_argument_or_file_exits_1(void) {
    int status = system(SCRIPT " > " LOG " 2>&1");
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 1);
    status = system(SCRIPT " " DIR "/no-such-flags.make > " LOG " 2>&1");
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 1);
}

int main(void) {
    test_the_last_o_token_wins_over_the_release_o3();
    test_an_os_or_o3_put_after_the_sdk_flags_reads_as_such();
    test_no_o_token_exits_2();
    test_no_c_flags_line_exits_2();
    test_a_missing_argument_or_file_exits_1();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_opt_level: all tests passed\n");
    return 0;
}
