/* tools/install_psx_emulator.sh on a local git fixture standing in for
 * PCSX-Redux: a Makefile that writes a stub pcsx-redux and openbios.bin, no
 * apt, no network, no emulator build. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DIR "tmp/test_psx_emulator_setup_work"
#define UPSTREAM DIR "/upstream"
#define SOURCES_DIR DIR "/tools"
#define SOURCES SOURCES_DIR "/psx_sources.txt"
#define INSTALL DIR "/install"
#define LOG DIR "/log.txt"
#define GIT "git -c user.name=test -c user.email=test@example.com -c init.defaultBranch=main -C " UPSTREAM " "

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
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

/* Static buffer: the first line of the command's stdout, newline stripped. */
static const char *output_of(const char *command) {
    static char line[256];
    FILE *pipe = popen(command, "r");
    assert(pipe != NULL);
    line[0] = '\0';
    if (fgets(line, sizeof(line), pipe) == NULL) {
        line[0] = '\0';
    }
    pclose(pipe);
    line[strcspn(line, "\n")] = '\0';
    return line;
}

static const char *PATCH = "--- a/src/core/listener.txt\n"
                           "+++ b/src/core/listener.txt\n"
                           "@@ -1 +1 @@\n"
                           "-leaks\n"
                           "+restores the top\n";

static const char *BAD_PATCH = "--- a/src/core/listener.txt\n"
                               "+++ b/src/core/listener.txt\n"
                               "@@ -1 +1 @@\n"
                               "-not what the file holds\n"
                               "+anything\n";

/* An upstream with a base commit and a headless-fix commit on top, and a
 * sources list naming both, with no patch next to it. */
static void setup(void) {
    char cwd[1024];
    char sources[4096];
    char base[64];
    char fix[64];
    assert(system("rm -rf " DIR " && mkdir -p " UPSTREAM "/src/core " UPSTREAM "/src/mips/openbios " SOURCES_DIR) == 0);
    write_file(UPSTREAM "/Makefile", "pcsx-redux:\n\tprintf '#!/bin/sh\\n' > pcsx-redux && chmod +x pcsx-redux\n");
    write_file(UPSTREAM "/src/mips/openbios/Makefile", "all:\n\ttouch openbios.bin\n");
    write_file(UPSTREAM "/src/core/listener.txt", "leaks\n");
    write_file(UPSTREAM "/src/core/gpu.txt", "segfaults headless\n");
    assert(system(GIT "init -q && " GIT "config uploadpack.allowAnySHA1InWant true && " GIT "add -A && " GIT
                      "commit -q -m base") == 0);
    snprintf(base, sizeof(base), "%s", output_of(GIT "rev-parse HEAD"));
    write_file(UPSTREAM "/src/core/gpu.txt", "boots headless\n");
    assert(system(GIT "commit -q -a -m fix") == 0);
    snprintf(fix, sizeof(fix), "%s", output_of(GIT "rev-parse HEAD"));
    assert(getcwd(cwd, sizeof(cwd)) != NULL);
    snprintf(sources, sizeof(sources),
             "emulator git file://%s/" UPSTREAM " %s\nemulator-headless-fix git file://%s/" UPSTREAM " %s\n", cwd,
             base, cwd, fix);
    write_file(SOURCES, sources);
}

static int install(void) {
    return system("PSX_EMULATOR_SKIP_APT=1 sh tools/install_psx_emulator.sh " INSTALL " " SOURCES " > " LOG " 2>&1");
}

static void test_a_patch_next_to_the_sources_is_applied_after_the_fix(void) {
    setup();
    write_file(SOURCES_DIR "/pcsx-redux-listener.patch", PATCH);
    assert(install() == 0);
    assert(strcmp(read_file(INSTALL "/src/core/listener.txt"), "restores the top\n") == 0);
    assert(strcmp(read_file(INSTALL "/src/core/gpu.txt"), "boots headless\n") == 0);
}

static void test_the_patch_is_part_of_the_stamp(void) {
    char expected[256];
    setup();
    write_file(SOURCES_DIR "/pcsx-redux-listener.patch", PATCH);
    assert(install() == 0);
    snprintf(expected, sizeof(expected), "\npatch pcsx-redux-listener.patch %s\n",
             output_of(". tools/psx_portable.sh && sha256_of " SOURCES_DIR "/pcsx-redux-listener.patch"));
    assert(strstr(read_file(INSTALL "/psx-emulator.stamp"), expected) != NULL);
    assert(install() == 0);
    assert(strstr(read_file(LOG), "psx-emulator-setup: already installed in " INSTALL "\n") != NULL);
}

static void test_an_install_without_the_patch_is_not_taken_as_current(void) {
    setup();
    assert(install() == 0);
    write_file(SOURCES_DIR "/pcsx-redux-listener.patch", PATCH);
    assert(install() != 0);
    assert(strstr(read_file(LOG), "holds no stamp for these pins") != NULL);
}

static void test_a_patch_that_does_not_apply_fails_naming_it(void) {
    setup();
    write_file(SOURCES_DIR "/pcsx-redux-bad.patch", BAD_PATCH);
    assert(install() != 0);
    assert(strstr(read_file(LOG), "psx-emulator-setup: pcsx-redux-bad.patch does not apply cleanly") != NULL);
    assert(read_file(INSTALL "/psx-emulator.stamp")[0] == '\0');
}

int main(void) {
    test_a_patch_next_to_the_sources_is_applied_after_the_fix();
    test_the_patch_is_part_of_the_stamp();
    test_an_install_without_the_patch_is_not_taken_as_current();
    test_a_patch_that_does_not_apply_fails_naming_it();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_emulator_setup: all tests passed\n");
    return 0;
}
