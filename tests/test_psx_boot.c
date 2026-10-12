/* tools/psx_boot.sh --until-exit, on a stub install whose pcsx-redux is a
 * shell script running $PSX_STUB: no emulator is ever booted. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DIR "tmp/test_psx_boot_work"
#define INSTALL DIR "/install"
#define EXE DIR "/game.exe"
#define PREFIX DIR "/out/run"
#define LOG DIR "/log.txt"

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

/* True when `log` holds `expected` with each '@' replaced by the working
 * directory: psx_boot.sh names its input and log by absolute path. */
static int has(const char *log, const char *expected) {
    char cwd[1024];
    char text[4096];
    assert(getcwd(cwd, sizeof(cwd)) != NULL);
    size_t len = 0;
    for (const char *c = expected; *c != '\0'; c++) {
        if (*c == '@') {
            assert(len + strlen(cwd) < sizeof(text));
            memcpy(text + len, cwd, strlen(cwd));
            len += strlen(cwd);
        } else {
            assert(len + 1 < sizeof(text));
            text[len++] = *c;
        }
    }
    text[len] = '\0';
    return strstr(log, text) != NULL;
}

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " INSTALL "/src/mips/openbios") == 0);
    write_file(INSTALL "/psx-emulator.stamp", "stub\n");
    write_file(INSTALL "/src/mips/openbios/openbios.bin", "bios\n");
    write_file(INSTALL "/pcsx-redux", "#!/bin/sh\neval \"$PSX_STUB\"\n");
    assert(system("chmod +x " INSTALL "/pcsx-redux") == 0);
    write_file(EXE, "exe\n");
}

/* `stub` is shell code run by the stub emulator; it sees the boot's
 * environment (PSX_RESULT_FILE, PSX_PROGRESS_FILE, ...). */
static int boot(const char *stub, const char *timeout) {
    char command[2048];
    snprintf(command, sizeof(command),
             "PSX_STUB='%s' PSX_BOOT_TIMEOUT=%s sh tools/psx_boot.sh " INSTALL " --until-exit --exe " EXE
             " 100 " PREFIX " > " LOG " 2>&1",
             stub, timeout);
    return system(command);
}

static void test_abort_names_signal_last_progress_line_and_vsync(void) {
    setup();
    assert(boot("echo \"psx-replay: frame 4608\"; echo \"vsync 7000\" > \"$PSX_PROGRESS_FILE\"; kill -ABRT $$",
                "10") != 0);
    assert(has(read_file(LOG), "psx-boot: @/" EXE ": emulator aborted with status 134 (signal SIGABRT); last progress "
                               "line: psx-replay: frame 4608; last vsync: 7000 (see @/" PREFIX ".log)\n"));
}

static void test_abort_ignores_a_stale_progress_file(void) {
    setup();
    assert(system("mkdir -p " DIR "/out") == 0);
    write_file(PREFIX ".progress", "vsync 99999\n");
    assert(boot("kill -ABRT $$", "10") != 0);
    assert(has(read_file(LOG), "emulator aborted with status 134 (signal SIGABRT); last progress line: (none); last "
                               "vsync: (none) (see "));
}

static void test_exit_result_with_status_0_passes(void) {
    setup();
    assert(boot("echo \"exit 120\" > \"$PSX_RESULT_FILE\"", "10") == 0);
    assert(has(read_file(LOG), "psx-boot: @/" EXE ": exited with 0 after 120 vsyncs, @/" PREFIX ".tty.txt\n"));
}

static void test_ceiling_result_keeps_its_message(void) {
    setup();
    assert(boot("echo \"psx-game: frame 3\"; echo \"ceiling 100\" > \"$PSX_RESULT_FILE\"; exit 1", "10") != 0);
    assert(has(read_file(LOG), "psx-boot: @/" EXE ": reached the ceiling of 100 vsyncs before exiting; last "
                               "progress line: psx-game: frame 3 (see @/" PREFIX ".log)\n"));
}

static void test_error_result_keeps_its_message(void) {
    setup();
    assert(boot("echo \"error capture out of step\" > \"$PSX_RESULT_FILE\"; exit 1", "10") != 0);
    assert(has(read_file(LOG), "psx-boot: @/" EXE ": capture out of step\n"));
}

static void test_program_exit_code_keeps_its_message(void) {
    setup();
    assert(boot("echo \"exit 50\" > \"$PSX_RESULT_FILE\"; exit 3", "10") != 0);
    assert(has(read_file(LOG), "psx-boot: @/" EXE ": program exited with code 3; last progress line: (none) (see @/" PREFIX
                               ".log)\n"));
}

static void test_timeout_names_the_last_vsync(void) {
    setup();
    assert(boot("echo \"psx-pad: step 2\"; echo \"vsync 60\" > \"$PSX_PROGRESS_FILE\"; exec sleep 5", "1") != 0);
    assert(has(read_file(LOG), "psx-boot: @/" EXE ": timed out after 1 s before exiting; last progress line: "
                               "psx-pad: step 2; last vsync: 60 (see @/" PREFIX ".log)\n"));
}

static void test_progress_file_is_named_after_the_out_prefix(void) {
    setup();
    assert(boot("echo \"$PSX_PROGRESS_FILE\"; echo \"exit 1\" > \"$PSX_RESULT_FILE\"", "10") == 0);
    assert(has(read_file(PREFIX ".tty.txt"), "@/" PREFIX ".progress\n"));
}

/* GNU stdbuf passes its modes to the command as _STDBUF_O and _STDBUF_E. */
static void test_emulator_output_is_unbuffered_when_the_host_has_stdbuf(void) {
    if (system("command -v stdbuf > /dev/null 2>&1") != 0) {
        return;
    }
    setup();
    assert(boot("echo \"stdout $_STDBUF_O stderr $_STDBUF_E\"; echo \"exit 1\" > \"$PSX_RESULT_FILE\"", "10") == 0);
    assert(has(read_file(PREFIX ".tty.txt"), "stdout 0 stderr 0\n"));
}

int main(void) {
    test_abort_names_signal_last_progress_line_and_vsync();
    test_abort_ignores_a_stale_progress_file();
    test_exit_result_with_status_0_passes();
    test_ceiling_result_keeps_its_message();
    test_error_result_keeps_its_message();
    test_program_exit_code_keeps_its_message();
    test_timeout_names_the_last_vsync();
    test_progress_file_is_named_after_the_out_prefix();
    test_emulator_output_is_unbuffered_when_the_host_has_stdbuf();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_boot: all tests passed\n");
    return 0;
}
