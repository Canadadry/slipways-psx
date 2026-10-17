/* Linked against one source written by the embed_replay tool, whose name
 * argument was the recording's own path: the embedded frames and save are
 * compared with what the PC replay source loads from that path. */

#include "devtools/embedded_replay.h"
#include "devtools/replay_source.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOOL "./tmp/embed_replay"

/* `<argv[0]>_work`: the two binaries built from this file run in parallel. */
static char work_dir[256];

static void test_embedded_frames_equal_the_replay_source_frames(void) {
    replay_source_t source;
    assert(replay_source_open(&source, embedded_replay_name) == REPLAY_SOURCE_LOADED);
    assert(embedded_replay_frame_count > 0);
    assert(embedded_replay_frame_count == replay_source_frame_count(&source));

    for (int i = 0; i < embedded_replay_frame_count; i++) {
        p8_input_frame_t frame;
        assert(replay_source_next_frame(&source, &frame) == 1);
        assert(memcmp(&frame, &embedded_replay_frames[i], sizeof(frame)) == 0);
    }

    replay_source_close(&source);
}

static void test_embedded_save_equals_the_replay_source_save(void) {
    replay_source_t source;
    assert(replay_source_open(&source, embedded_replay_name) == REPLAY_SOURCE_LOADED);

    assert(embedded_replay_initial_save_count == source.initial_save_count);
    assert(memcmp(embedded_replay_initial_save, source.initial_save, (size_t)source.initial_save_count) == 0);
    assert(embedded_replay_attract_after == replay_source_attract_after(&source));

    replay_source_close(&source);
}

static int file_exists(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fclose(file);
    return 1;
}

static void write_text(const char *path, const char *text) {
    FILE *file = fopen(path, "wb");
    assert(file);
    assert(fputs(text, file) >= 0);
    assert(fclose(file) == 0);
}

static void run_shell(const char *command, int expect_success) {
    int status = system(command);
    assert(expect_success ? status == 0 : status != 0);
}

/* The tool must exit non-zero, name the recording on stderr and leave no
 * output behind: a half-written source would link into an empty replay. */
static void assert_refused(const char *recording) {
    char command[1024];
    char output[512];
    char err_path[512];
    snprintf(output, sizeof(output), "%s/out.c", work_dir);
    snprintf(err_path, sizeof(err_path), "%s/err.txt", work_dir);

    snprintf(command, sizeof(command), "rm -rf %s && mkdir -p %s", work_dir, work_dir);
    run_shell(command, 1);
    int used = snprintf(command, sizeof(command), TOOL " %s name %s 2> %s", recording, output, err_path);
    assert(used > 0 && (size_t)used < sizeof(command));
    run_shell(command, 0);

    assert(!file_exists(output));

    static char err[4096];
    FILE *file = fopen(err_path, "rb");
    assert(file);
    size_t length = fread(err, 1, sizeof(err) - 1, file);
    fclose(file);
    err[length] = '\0';
    assert(strstr(err, recording));
}

static void test_mouse_recording_is_refused(void) {
    assert_refused("tests/fixtures/events_mouse_recording_refused.json");
}

static void test_unreadable_recording_is_refused(void) {
    assert_refused("tests/fixtures/no_such_recording.json");
}

static void test_empty_recording_is_refused(void) {
    char path[512];
    snprintf(path, sizeof(path), "%s_empty.json", work_dir);
    write_text(path, "{\n  \"input\": \"pad\",\n  \"frames\": [\n  ]\n}\n");
    assert_refused(path);
    remove(path);
}

static void test_resume_start_is_refused(void) {
    assert_refused("demo/demo.json");
}

static void test_bindings_member_is_refused(void) {
    assert_refused("tests/e2e/pad_swap_aim_menu.json");
}

int main(int argc, char **argv) {
    assert(argc >= 1);
    int used = snprintf(work_dir, sizeof(work_dir), "%s_work", argv[0]);
    assert(used > 0 && (size_t)used < sizeof(work_dir));

    test_embedded_frames_equal_the_replay_source_frames();
    test_embedded_save_equals_the_replay_source_save();
    test_mouse_recording_is_refused();
    test_unreadable_recording_is_refused();
    test_empty_recording_is_refused();
    test_resume_start_is_refused();
    test_bindings_member_is_refused();

    printf("test_embed_replay: all tests passed (%s)\n", embedded_replay_name);
    return 0;
}
