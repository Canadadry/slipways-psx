# Make targets

`make help` lists every target in one line each. This page holds the detail:
arguments, outputs, failure conditions. Every target runs in parallel (`-j` set to
the number of cores, output grouped per target).

## `make test`

unit + constants-check + float-grep + float-guard + alloc-guard + pointer-guard + random-guard + button-guard + data-guard + p8-guard + pad-author-self-check, the fast loop.

## `make e2e`

replays each e2e of tests/e2e/ from the title screen, with the `-O0` binary of make test, and compares the sha256 of its capture to <name>.json.sha256; also fails if an e2e lacks its .txt or its .json.sha256, or runs past 180 frames (600 if listed in tools/e2e_long_exceptions.txt, whose "N frames:" must match and exceed 180), or if an e2e does not declare "input": "pad" or carries a mouse field; an e2e whose header names a fixture ("bindings": "<file>.cfg", read from its directory) reruns when that fixture changes, through the .d file replay writes next to the capture.

## `make test-all`

test + e2e, the full suite.

## `make unit`

the binaries of tests/, built and run.

## `make replay EVENTS=<events>`

replays that file in the headless engine, framebuffer dump in tmp/replay_dump.txt, outside the suite; with SAVE_HEX=<file>, also writes the stored record at the end as one line of hex (the initial_save format) and prints its decoded "save:" summary; fails when there is no record or it does not decode; with DIGEST=<file>, also writes the FNV-1a lines of src/devtools/vram_digest.h ("sprite-sheet", "screen <frame>" every 32nd frame of the recording and the last, "map"); the binary also reads CAPTURE_LIST=<list> with CAPTURE_DIR=<dir> (psx-check): draws each listed frame with its draw filter mask and writes pc_frames.bin, counts.txt and capture_script.txt there, exiting 1 on a bad list; refuses a recording whose header says "bindings": "personal"; a file whose header says "start": "resume" (pad_author's from-record) starts from its initial_save at the first frame, not on the title; with REPLAY_STARTS=2, plays it, returns to the title, idles 30 frames and plays it again, and takes no capture but /dev/null, so a dump path can follow it; with ATTRACT_STATE=<file> on a title-start file, writes the state on the last frame of each complete demo playback to <file>.1, <file>.2, ..., then, on the frame of its return to the title, the state to <file>.<n>.return and the stored record (SAVE_HEX's format and failures) to <file>.<n>.return.hex.

## `make state EVENTS=<events>`

make replay with STATE set, which also writes the state of the last frame (clock, difficulty, planets, slipways, structures, counts), zone and cap use, then the cheat activation count, to tmp/state.txt, with the draw meter's lines (zone gpu_frame: GPU bytes a frame against GPU_FRAME_BUFFER_BYTES; cap gpu_cluts, cap gpu_patterns: distinct draw palettes and fill patterns a frame), outside the suite; takes SAVE_HEX=<file>, DIGEST=<file>, REPLAY_STARTS and ATTRACT_STATE as make replay does; with REPLAY_STARTS=2, the first playback's state goes to tmp/state.txt.start1.

## `make state`

replays the worst-case/ games (FULL_GAME_EVENTS) and every e2e of tests/e2e/, each state in tmp/state/<name>.txt, then writes to tmp/state.txt the peak of each zone and cap over them all, its limit and the replay holding it, and prints the bytes left under ZONE_BUDGET_BYTES, then the two GPU primitive buffers' total, outside it; fails if a replay stops or a peak eats its 1/8 margin (9 x peak > 8 x limit), outside the suite.

## `make end-state`

replays every file make state replays, normalizes each last-frame state with tools/end_state.sh (game fields and cheat activations kept; camera, screen, visible, zone, budget and cap dropped; sorted) and compares it with the committed <events>.end-state; fails on any difference, a stopped replay, a missing reference or a state line the script does not know, after examining every replay, outside the suite.

## `make end-state EVENTS=<events> [REFERENCE=<reference>]`

the same check for that file alone, against <events>.end-state or REFERENCE, state in tmp/end-state/, outside the suite.

## `make end-state-baseline`

same replays, writes each missing <events>.end-state and never overwrites one (delete a reference to re-baseline it); refuses a stopped replay or an unknown line, outside the suite.

## `make e2e-baseline`

sets or resets the .json.sha256 of tests/e2e/ from the current code, never run by make test.

## `make e2e-ref`

for each diverging e2e, replays the commit that set its reference (git blame), leaves that capture in tmp/frames/<events>.ref.bin and gives the first diverging frame, outside the suite.

## `make data-guard`

none of the patterns of the pre-composition model (DATA_GUARD_PATTERN) in src/ or tests/, in make test.

## `make p8-guard`

none of the PICO-8-only APIs (P8_GUARD_PATTERN: audio, p8_stat, peek/poke, PICO-8 addresses, string-keyed tags/messages/states, map.json) in src/ or tests/, in make test; entity_state_name's table is the accepted exception (text for make state and frozen hashes).

## `make sanitize-net`

replays the e2e of tests/e2e/ under ASan+UBSan (`-O2`); passes when every replay exits cleanly; also runs test_live_slots (slots returned after each replay, ~17 s), outside the suite.

## `make coverage`

builds tmp/replay_coverage (`--coverage -O0`), replays COVERAGE_EVENTS (default: every e2e of tests/e2e/) one after the other, writes tmp/coverage/lines.txt (`<path>:<line> <0|1>` per executable line of src/game/ and src/engine/, headers included) and prints the totals, outside the suite.

## `make coverage-compare`

make coverage, then maps each line hit at COVERAGE_BASELINE_COMMIT (default 6346485) to the current tree through `git diff -U0 -M --histogram` of src/game/ and src/engine/ (the default algorithm pairs a closing brace with another function's) and tools/coverage_compare.sh: prints the unhit and deleted ones (a rewritten line is deleted) and the totals, full report in tmp/coverage-compare/report.txt; fails if a line is unhit; the baseline's lines are built once from a worktree of that commit and kept in tmp/coverage-baseline-<commit>/, outside the suite.

## `make full-game`

replays each worst-case/ game (FULL_GAME_EVENTS: full_game.json, then the same game followed by a new game from the title, cut short by pause -> restart -> new game, and cut short by a pause menu left open 300 frames; plus title_idle.json, the title screen left alone 300 frames) to its last frame with the `-O0` binary of make test; passes when every replay exits cleanly (fails on any p8_fatal, a full zone or cap), outside the suite.

## `make pad-author`

builds tmp/pad_author, plays each worst-case/<game>.pad headless and rewrites worst-case/<game>.json from the frames it pressed, then replays each JSON with tmp/replay and fails unless the two last-frame states are byte-identical; also fails on a refused input (a button 5 + 6 chord, a jump-mode D-pad press into an empty cone, a back action (button 6 in the default bindings) that changes nothing unless `back --required`), a failed command check, more than 7200 frames, or 25% or more of the frames in aim mode; a `caption` (`/` breaks the line, `caption -` clears; written as a "captions" header member) is refused with a line over 32 characters, more than 2 lines, the letter v, a character with no font glyph, fewer than 150 frames shown, or a button down fewer than 90 frames after it appears; the per-frame mode log and the totals go to tmp/pad-author/<game>.modes.txt, outside the suite; then the same for pico8/demo.pad -> pico8/demo.json, without the aim budget.

## `make pad-author SCRIPT=<script> [OUT=<events>] [STOP_AT=<label>]`

the same for one script, written to OUT (default tmp/pad-author/<name>.json), ending at its first `label <STOP_AT>` (no aim budget then); a failed command writes the frames so far to tmp/pad-author/<name>.failed.json and the state to tmp/pad-author/<name>.state.txt.

## `make pad-author-proof`

runs tools/pad_author_proofs/: each <e2e>.frames.pad must be byte-identical to tests/e2e/<e2e>.json, each <e2e>.end-state.pad must pass tools/end_state.sh against tests/e2e/<e2e>.json.end-state (both without the aim budget), and each <name>.refused.pad must fail with the error its `# expect:` first line names; each <name>.record.pad (a script that starts with from-record) must pass pad_author's self-check, then replay with REPLAY_STARTS=2 to pad_author's normalized end state at both ends, outside the suite.

## `make demo-check`

tools/demo_check_run.sh, files and dumps in tmp/demo-check/, every check always run, outside the suite: tests/test_demo_check; tests/test_attract_demo (the whole demo through the attract mode twice, one interruption per command range, the player's save untouched) also writes the full cycle at the default threshold over tests/e2e/pad_tech_costs.json's initial_save, which must equal worst-case/attract_cycle.json byte for byte; that file is replayed to its end by the `-O0` replay (with ATTRACT_STATE, capture hashed) and by replay_sanitize (clean exit): each playback passes tools/demo_check.sh and ends at the resumed replay's normalized end state, no third playback, each return and the file's end equal worst-case/title_idle.json.end-state with the stored record equal to the initial_save; pico8/demo.json, replayed with REPLAY_STARTS=2, passes tools/demo_check.sh at both ends (at most 3600 frames, the seven captions pinned in tools/demo_check.sh, in order, tiling the demo, a 180-frame closing card, exactly one more year, colony, slipway and tech than the state at the `probe` label, no cheat); pico8/demo.pad regenerates pico8/demo.json byte for byte (no aim budget), a failure that fails the target but does not hold back the hash; last, and only when the content checks above pass, the capture hash is compared with worst-case/attract_cycle.json.sha256, a missing one failing; to rebaseline after reading the checks: cp tmp/demo-check/attract_cycle.json worst-case/ and cp tmp/demo-check/attract_cycle.sha256 worst-case/attract_cycle.json.sha256; never alongside make test-all (both rebuild binaries in tmp/) or make pad-author (both write tmp/pad-author/).

## `make pad-metrics EVENTS=<events.json> [CHECK=1]`

replays the file with PAD_METRICS and prints its report (time per mode, dead presses by group, hold repeats, section 4 verdict), kept in tmp/pad-metrics/<name>.txt; with CHECK=1, fails when the verdict reads fail; outside the suite.

## `make pad-metrics-worst-case`

the same with CHECK=1 for the four re-authored worst-case/ games, printing each section 4 block, outside the suite.

## `make pad-metrics-check`

regenerates tests/e2e/pad_colonise.json from its proof script, then fails unless the report's `total ` lines equal the generator's and its required group is exactly f18's button 7, f19's button 6 (aim mode, nothing known) and f295's button 6, outside the suite.

## `make worst-case-peaks`

replays the worst-case/ games (FULL_GAME_EVENTS) as make state does and prints, per line of worst-case/peaks-8c4bbf4.txt, the table's peak, the highest peak over those games and its holder, and ok or short; fails on any short line or a stopped game, outside the suite.

## `make rng-sync-probe`

builds and runs tests/test_rng_sync (pad_colonise replayed to a save frame, forked, rnd_state compared with and without the state a save never holds); red while saving rnd_state is not enough, outside the suite.

## `make save-worst-case`

builds and runs tests/test_save_worst_case: replays each worst-case/ game (FULL_GAME_EVENTS), and at every frame of a running game with no wormhole captures, encodes, decodes and re-encodes the save; fails if a record exceeds 8 KiB or does not decode back to the same bytes, prints the largest record with its game and frame, outside the suite.

## `make save-fuzz`

builds and runs tests/test_save_fuzz: replays worst-case/full_game.json, and at every frame that writes a save resumes the stored record into a fresh game after the title and compares it with the original at that frame (bytes, state dump, screen with particles hidden when no box is open); fails on any mismatch, outside the suite.

## `make resume-check EVENTS=<events> BEFORE=<frame> AFTER=<frame>`

builds and runs tests/test_resume_check: replays that file headless and compares the game after frame BEFORE (the stored record, state dump, screen with particles hidden) with the game the recording resumed by frame AFTER; fails on any difference, on a storage write between BEFORE and the resume, with no game at either frame or no title between them, outside the suite.

## `make constants-check`

compares each P8_FIXED_LIT of the core with its recorded hex value (tools/pico8_reference_probe_out.txt); fails unless the mismatches are exactly tools/constants_expected_gaps.txt, in make test.

## `make float-guard`

rewrites tools/float_guard.txt; fails if a file of the core needs floating point (it does not compile under `-mgeneral-regs-only`), in make test.

## `make alloc-guard`

counts the malloc/calloc/realloc/free calls of each object of the core; fails on any difference, higher or lower, from tools/alloc_guard_exceptions.txt (a ratchet, updated by hand), in make test.

## `make pointer-guard`

counts the data pointers and entity_list_t declarations in the struct fields and file-scope variables of src/game/ and entity_class.h, and the local entity_t *name[...] arrays; fails on any difference, higher or lower, from tools/pointer_guard_exceptions.txt (a ratchet, updated by hand), in make test.

## `make float-grep`

searches the core (CORE_SRCS and the headers that `-MM` finds for them) for any floating-point type or call, the rest of src/ excluded by tools/float_grep_exclusions.txt, in make test.

## `make random-guard`

counts, per file of src/, the names of the game stream (p8_fixed_rnd*, p8_fixed_set_rnd_state), of the generator on a caller's state (p8_fixed_rnd_from_state) and of the visual stream (visual_random_draw*), comments and strings left out; fails on any difference from tools/random_guard_exceptions.txt (strict counts, updated by hand), in make test.

## `make button-guard`

counts, per file of src/game/, the P8_PAD_BUTTON_* names, comments and strings left out; fails when a file other than cheat.c and the bindings module names one, in make test.

## `make float-link`

links the objects of tmp/replay without `-lm` into a throwaway binary, never run; outside the suite: make test already builds tmp/replay, with the same link.

## `make sin-table`

regenerates src/p8/quarter_wave.h (the sine and cosine table of the core) from tools/gen_quarter_wave.c, outside the suite.

## `make sprite-sheet`

regenerates src/p8/sprite_sheet_packed.h (the sprite sheet embedded in the core, 4 bits per pixel) from pico8/gfx.json with tools/gen_sprite_sheet.c, outside the suite.

## `make raylib`

compiles and LINKS the interactive binary, never runs it (needs make raylib-vendor-setup first); prints the bindings and keymap files it compiled: <name>.cfg when present, else <name>.default.cfg; every other target compiles bindings.default.cfg, and only test_keymap compiles keymap.default.cfg; a game built with bindings.cfg writes "bindings": "personal" in every recording, which replay refuses; also links pico8/demo.json, compiled into tmp/demo/demo.c; every binary, game included, also links bindings.default.cfg as the default table the demo plays on.

## `make psn00bsdk-toolchain-setup`

runs psx-net-check, then installs the pinned PSn00bSDK release of PSX_SOURCES (prebuilt GCC, elf2x, mkpsxiso, libpsn00b) into PSN00BSDK_DIR (default third_party/psn00bsdk); on macOS, a prebuilt arm64 GCC 14.2.0 and PSn00bSDK built with it (no sudo); with PSN00BSDK_FROM_SOURCE, GCC 12.3.0 built from source too (30-60 min); apt via sudo on Linux unless PSN00BSDK_SKIP_APT is set; exits 0 at once on a complete install, fails on a non-empty directory without its stamp, outside the suite.

## `make psx-build [EVENTS=<events>] [PSX_BUILD_DRAW_FILTER=ON|OFF] [PSX_BUILD_DRAW_METER=ON|OFF] [PSX_BUILD_GAME_ASSERTS=ON|OFF] [PSX_BUILD_OPT=O2|O3|Os]`

builds, in tmp/psx/build, the game's PS-EXEs replaying that recording (default tests/e2e/pad_slipway.json, embedded through tmp/embed_replay), psx-replay drawing in RAM, psx-gpu-replay through the GPU backend and psx-gpu-replay-breakdown (the same, timing each backend call per draw family), both with the draw filter (PSX_GPU_REPLAY_DRAW_FILTER) unless PSX_BUILD_DRAW_FILTER=OFF and with the draw meter (PSX_GPU_REPLAY_DRAW_METER) unless PSX_BUILD_DRAW_METER=OFF, the pad probe, psx-game (the game read from the pad, no draw meter, built without asserts unless PSX_BUILD_GAME_ASSERTS=ON), psx-largest-slots (the largest-case scenarios of src/devtools/largest_slots.c on MIPS), and the game's CD image (PSX_GAME_BIN, PSX_GAME_CUE: slipways.bin/.cue from psx/iso.xml and psx/system.cnf, booting SLIPWAYS.EXE, a copy of psx-game.exe), against PSN00BSDK_DIR, psx-game and both GPU replays at PSX_BUILD_OPT (default O3, refused unless O2, O3 or Os), the other targets at the SDK's O2; then reads those three levels with tools/psx_opt_level.sh, prints "psx-build: opt psx-game <l> psx-gpu-replay <l> psx-gpu-replay-breakdown <l>" and fails naming a target whose level is not PSX_BUILD_OPT, then prints their ELF sizes and the image paths; never boots them; fails naming a missing image file, or pointing at psn00bsdk-toolchain-setup when the SDK is missing, outside the suite.

## `make psx-game-cd`

runs psx-build, then fails unless PSX_GAME_BIN and PSX_GAME_CUE exist and prints them; the .bin is never hashed (mkpsxiso stamps the build time in it), outside the suite.

## `make psx-replay-check`

runs psx-build, writes the PC state and VRAM digests of PSX_EVENTS with tmp/replay (STATE=, DIGEST=), boots the replay PS-EXE until it exits (ceiling PSX_REPLAY_CEILING vsyncs, timeout PSX_REPLAY_TIMEOUT s, both set for pad_slipway), then compares its game lines with the PC's through tools/psx_state_compare.sh and its digest lines through tools/psx_digest_compare.sh (outputs and measurements.txt, with the ELF size, in tmp/psx/replay, ~45 s); never runs a setup, outside the suite.

## `make psx-tools-test`

host tests of the PSX check scripts, outside make test-all; a prerequisite of psx-check and psx-game-check.

## `make psx-game-check`

runs psx-build, boots the game PS-EXE until it exits while tools/psx_pad_play.lua plays PSX_GAME_SCRIPT (default psx/game_title_script.txt; ceiling PSX_GAME_CEILING vsyncs, timeout PSX_GAME_TIMEOUT s), then tools/psx_game_check.sh: game-done, game-pad-log (the TTY pad log turned into game_events.json by tmp/psx_pad_log), game-pad-states, game-state-game-lines (that recording replayed by tmp/replay with STATE=), game-title-left (difficulty PSX_GAME_DIFFICULTY, default reasonable), game-asserts (the TTY's asserts mode is PSX_BUILD_GAME_ASSERTS's, off by default), game-gpu-cost (the TTY's gpu-peak bytes, CLUTs, patterns and build bytes within the PC replay's metered peaks); outputs in tmp/psx/game, ~10 s; never runs a setup, outside the suite.

## `make psx-game-cd-check`

runs psx-game-cd, boots PSX_GAME_CUE until it exits with PSX_GAME_SCRIPT as psx-game-check does (ceiling PSX_GAME_CD_CEILING vsyncs, timeout PSX_GAME_CD_TIMEOUT s), then the seven game-* conditions of tools/psx_game_check.sh and the cue and cd-exe conditions of tools/psx_cd_check.sh (SLIPWAYS.EXE extracted by PSX_DUMPSXISO equals psx-game.exe); outputs in tmp/psx/game-cd, never runs a setup, outside the suite.

## `make psx-check`

the PSX check: runs psx-build (draw filter on, for the masked captures, and draw meter on, for gpu-cost), writes the PC state and VRAM digests of PSX_EVENTS in one tmp/replay run (STATE=, DIGEST=, and CAPTURE_LIST=psx/gpu_captures.txt for the PC frames, family call counts and capture script), boots the replay PS-EXE, the GPU replay PS-EXE twice with that capture script (ceiling PSX_GPU_CEILING vsyncs, timeout PSX_GPU_TIMEOUT s), the pad probe and the game's CD image PSX_GAME_CUE with PSX_GAME_SCRIPT (as psx-game-cd-check does) and psx-largest-slots (ceiling PSX_LARGEST_SLOTS_CEILING vsyncs, timeout PSX_LARGEST_SLOTS_TIMEOUT s) until they exit, six boots, then, for the second capture replay tests/e2e/pad_message_box.json (its PC run with CAPTURE_LIST=psx/gpu_captures_message_box.txt, its GPU replay alone built in tmp/psx/game-check/message-box/build, booted twice, ceiling PSX_MESSAGE_BOX_GPU_CEILING vsyncs, timeout PSX_MESSAGE_BOX_GPU_TIMEOUT s), two more, then prints one line per condition (magic, asserts-on, format against tmp/format_self_test, replay-done, state-game-lines, sprite-sheet, screen, map, gpu-replay-done, gpu-state-game-lines, gpu-cost: its meter lines equal the PC's, ram-room: 64 KiB kernel + GPU ELF (PSX_SIZE) + PSX_STACK_RESERVE <= 2 MiB, the seven gpu-capture-* conditions of tools/psx_capture_check.sh against psx/gpu_capture_ceilings.txt and psx/gpu_capture_references.txt, the same seven as message-box-gpu-capture-* on the second replay against psx/gpu_capture_ceilings_message_box.txt and psx/gpu_capture_references_message_box.txt, no-json through PSX_NM on both replay ELFs and the game ELF, pad-script, the seven game-* conditions of tools/psx_game_check.sh on the CD boot (game-gpu-cost: the CD game's gpu-peak figures within the PC replay's metered peaks for its pad log), game-cue and game-cd-exe of tools/psx_cd_check.sh, game-ram-room: ram-room on the game ELF, largest-slots: psx-largest-slots exits with 0, asserts on, and each of its bytes_button and bytes_row peaks fits its size) through tools/psx_check.sh; fails if any fails; measurements.txt (MIPS zone/budget/cap, vsyncs, size of the ELF, then the GPU replay's vsyncs, meter lines, RAM room, size and capture reports behind "gpu ", then the game's vsyncs, frames, RAM room and size behind "game ", the .bin's bytes and sectors and the CD boot's vsyncs from power-on behind "game cd ", then the bytes_<slot> lines behind "largest ") and conditions.txt in tmp/psx/game-check/run, the capture reports, diff maps and images in its gpu-captures/, the game's outputs in its game/ and the extracted disc in its game-cd/, ~125 s; never runs a setup, outside the suite.

## `make psx-gpu-capture-ceilings LABEL=<label> [CAPTURES=message_box]`

writes the differ counts of that label's captures from the last psx-check run into psx/gpu_capture_ceilings.txt (with CAPTURES=message_box, the second replay's into psx/gpu_capture_ceilings_message_box.txt), other labels untouched; refused unless every other condition passed.

## `make psx-gpu-capture-accept LABEL=<label> [CAPTURES=message_box]`

accepts a label (CAPTURES as above, into psx/gpu_capture_references_message_box.txt), run by an agent only when all its captures differ 0 from the PC, otherwise by the human: after a green psx-check, writes the sha256 of that label's captures (every frame of a label such as all or pattern) into psx/gpu_capture_references.txt, other labels untouched.

## `make psx-gpu-capture-report`

one PC run and one GPU capture boot of psx/gpu_captures.txt, then the comparator, without the other conditions (outputs in tmp/psx/gpu-capture, ~20 s); outside the suite.

## `make psx-full-game-check [PSX_FULL_GAME_ASSERTS=on|off]`

refuses any PSX_FULL_GAME_ASSERTS but off (default) or on; builds the PS-EXEs of PSX_FULL_GAME_EVENTS (worst-case/full_game.json) in tmp/psx/full-game/build, the GPU replays always without the draw filter and the draw meter and always at O3 (PSX_BUILD_OPT=O3), off building the measured GPU replays without asserts while the CPU-path replay keeps them, writes its PC state and digest, boots the replay (ceiling PSX_FULL_GAME_CEILING vsyncs, timeout PSX_FULL_GAME_TIMEOUT s) and the GPU replay with an empty capture script (PSX_FULL_GAME_GPU_CEILING, PSX_FULL_GAME_GPU_TIMEOUT) until they exit, then checks its conditions (replay-done, state-game-lines, sprite-sheet, screen, map, gpu-replay-done, gpu-state-game-lines, gpu-draw-filter and gpu-draw-meter: the GPU boot printed draw-filter off and draw-meter off, then on the GPU boot's psx-frame: lines and MIPS state, its gpu_* lines taken from the PC state, through tools/psx_frame_budget.sh: timer, frame-budget: no frame above PSX_FRAME_BUDGET_HBLANKS, zone-sizes, cap-sizes, then real-game-budget: frames.txt joined with PSX_FULL_GAME_SITUATIONS (default psx/perf/full-run-situations.txt), no steady frame above PSX_FRAME_BUDGET_HBLANKS and the one-off frames' vsyncs over it at most PSX_REAL_GAME_ONE_OFF_VSYNCS, then gpu-ram-room: ram-room on the GPU ELF) through tools/psx_full_game_check.sh, printing every frame line, the frame summary and the real-game figures; measurements.txt gains the PC's gpu_* lines and the GPU replay's gpu-peak line behind "gpu ", the summary behind "gpu frame ", the real-game figures behind "gpu " (outputs in tmp/psx/full-game/run, ~20 min); not run by psx-check, outside the suite.

## `make psx-real-game-budget [FRAMES=<frame table>] [SITUATIONS=<situation table>]`

tools/psx_real_game_budget.sh on the two tables (defaults psx/perf/full-run-frames.txt and PSX_FULL_GAME_SITUATIONS): steady frames over PSX_FRAME_BUDGET_HBLANKS with the worst one, one-off frames over it and their vsyncs against PSX_REAL_GAME_ONE_OFF_VSYNCS, the verdict; fails when it is failed; never boots or builds, outside the suite.

## `make psx-short-runs-cut [PSX_SHORT_RUNS=<names>]`

cuts each run of psx/short_runs.txt (or those named) out of PSX_FULL_GAME_EVENTS with tmp/short_run_cut into tmp/psx/short-runs/<name>/: run.json (a prefix, or the title resume of the last save at least 9 frames before the window whose next frame is idle, then the full game's frames up to the window's end), reference.json (the full game's frames up to the matching frame) and map.txt; then, per run, both must pass tmp/embed_replay and replay with STATE=; prints per run whether the two tools/end_state.sh normalized states are the same, with the differing lines (reported, not gating: a save holds no camera, jump selection or crosshair); fails on a refused cut, an embed failure, a stopped replay or a missing file, ~25 s; never boots, outside the suite.

## `make psx-short-runs [PSX_SHORT_RUNS=<names>] [PSX_SHORT_RUN_ASSERTS=on|off] [PSX_SHORT_RUN_OPT=O2|O3|Os]`

refuses any PSX_SHORT_RUN_ASSERTS but off (default) or on and any PSX_SHORT_RUN_OPT but O3 (default), O2 (the SDK's level) or Os, runs psx-short-runs-cut, then per run builds psx-gpu-replay and psx-gpu-replay-breakdown with its run.json embedded in tmp/psx/short-runs/<name>/build, both without the draw filter and the draw meter (off: without asserts, unused-* warnings not errors; O3/Os: at that level; every other target keeps asserts and O2), fails the run unless tools/psx_opt_level.sh reads PSX_SHORT_RUN_OPT as the last -O of both targets' flags, prints "opt <level>" and both ELFs' text/data/bss, boots the plain one twice and the breakdown once with an empty capture script (ceiling PSX_SHORT_RUN_CEILING vsyncs, default 1080, timeout PSX_SHORT_RUN_TIMEOUT s, default 20; exit ok|failed, wall time and vsyncs in boots.txt; a failed boot fails its run), then tools/psx_short_run.sh into <name>/analysis: done (each boot's state dump complete), deterministic, representative (against PSX_SHORT_RUN_REFERENCE, default psx-full-game-check's frames.txt; empty skips it, a missing file fails), breakdown, asserts (every boot reports PSX_SHORT_RUN_ASSERTS's mode), then the window's plain update, draw and submit sums, draw split, overhead and best cases; fails after every run when a boot or a condition failed, ~4 min; never runs a setup, outside the suite and psx-check.

## `make psx-probe-baseline [PSX_SHORT_RUNS=<names>]`

runs psx-short-runs-cut, then per run builds the probe build (psx-gpu-replay with -DPSX_PROBE=ON, asserts off, O3, no draw filter, no draw meter) of unpatched HEAD from a temporary git worktree, boots it once with an empty capture script (ceiling PSX_PROBE_CEILING vsyncs, timeout PSX_PROBE_TIMEOUT s) unless tools/psx_opt_level.sh reads another level than O3 (invalid "opt <level>, not O3"), checks it with tools/psx_probe.sh against itself and, when valid, stamps tmp/psx/probe/baseline/<name>/fingerprint.txt with the tree ids of src/ and psx/, the blob id of PSX_FULL_GAME_EVENTS and "opt O3"; fails if a run is invalid; removes the worktree on every exit, outside the suite.

## `make psx-probe PATCH=<file> [PSX_SHORT_RUNS=<names>] [PSX_PROBE_LABEL=<label>]`

the same pipeline on HEAD with the patch git apply'd in the worktree, outputs in tmp/psx/probe/<label>/ (default label: the patch's base name; baseline is reserved), then per run tools/psx_probe.sh against the baseline boot plus the ELF text sizes: window draw, update, submit and total deltas, game-state same or differs, delay / calls when the patch used platform/psx_probe.h; a run is invalid when the patch does not apply, the build fails or is not at O3, its baseline is missing or stale, or tools/psx_probe.sh says so; fails after every run if one is invalid, outside the suite and psx-check.

## `make frame-profile [EVENTS=<events>] [REFERENCE=<end-state>]`

builds tmp/frame_profile (`-O2`, asserts off, game, engine and PICO-8 runtime built with -finstrument-functions) and replays that recording (default worst-case/full_game.json) as tmp/replay does; writes in tmp/frame-profile/<name>/ frames.txt (per frame: update and draw PC time, calls, time, net time and calls below of each direct child of cart_update60 and cart_draw, self time, calls, time, cost-model units and texture-window switches per draw family, in-call) and summary.txt (the run's totals and shares), then checks the last state against <events>.end-state or REFERENCE with tools/end_state.sh: prints same-game yes, no (fails) or unchecked (no reference), outside the suite.

## `make frame-profile-breakdown-check [PSX_SHORT_RUNS=<names>]`

runs psx-short-runs-cut, then per run (default draw-5564) profiles its run.json into tmp/frame-profile/short-runs/<name>/ (same-game against its run.end-state) and, through tools/frame_profile_check.sh breakdown, compares the PC per-family calls with the psx-draw: lines of the run's last breakdown boot frame by frame over the window: calls-match ok with the window totals side by side, or the first mismatch; a missing breakdown TTY fails naming the make psx-short-runs to run; never boots, outside the suite.

## `make frame-profile-coverage [FRAME_PROFILE_THRESHOLD=<pct>] [FRAME_PROFILE_REPEATS=<k>]`

runs psx-short-runs-cut, profiles full_game.json and the start and every draw-* run of psx/short_runs.txt k times each (default 4, ~1 min) into tmp/frame-profile/coverage/<name>/<k>/, then tools/frame_profile_check.sh coverage (also in tmp/frame-profile/coverage.txt): per cart_draw child and draw family, frames above the threshold (default 2 %) of cart_draw's PC time in all k full profiles (per-frame minimum), mean and max share, share in each draw window (median of its k profiles), verdict covered, uncovered (with a proposed 15-frame resume window), sporadic (under 15 frames, listed) or below, then the windows to add; fails while anything is uncovered, outside the suite.

## `make frame-profile-subtree ROOT=<symbol> [PART=draw|update] [PSX_SHORT_RUNS=<names>] [FRAME_PROFILE_REPEATS=<k>]`

runs psx-short-runs-cut, profiles each run's run.json (default start and every draw-* run of psx/short_runs.txt) k times (default 4) with that function as the draw root (default) into tmp/frame-profile/subtree/<symbol>/<name>/<k>/, or with PART=update as the update root (draw root cart_draw) into tmp/frame-profile/subtree-update/ (same-game against its run.end-state), then tools/frame_profile_check.sh subtree (also in summary.txt of that <symbol> directory): per run, the root's PC time per window frame, then per child its calls per window frame and median share of the root's PC time, and the root's self share; fails on an unknown or ambiguous symbol; never boots, outside the suite.

## `make frame-profile-unit-cost [PSX_SHORT_RUNS=<names>]`

runs psx-short-runs-cut, then per run (default every run of psx/short_runs.txt) profiles its run.json into tmp/frame-profile/unit-cost/<name>/ (same-game against its run.end-state) and, through tools/frame_profile_check.sh unit-cost, joins the PC units with the run's last breakdown boot over the window into <name>/unit-cost.txt: per family calls, units, switches, hblanks, hblanks per call and per unit, or the first calls mismatch; then the runs summed in tmp/frame-profile/unit-cost/pooled.txt; a missing breakdown TTY fails naming the make psx-short-runs to run; never boots, outside the suite.

## `make frame-profile-spikes [FRAME_PROFILE_REPEATS=<k>] [FRAME_PROFILE_SPIKE_THRESHOLD=<hblanks>]`

profiles full_game.json k times (default 4) with cart_update60 as the update root into tmp/frame-profile/spikes/cart_update60/<k>/, then tools/frame_profile_check.sh spikes on the latest judge table psx/perf/full-run-frames.txt: every frame whose update exceeds the threshold (default 263) walked down to the child holding half its PC time; while a frame is open on a symbol, profiles that symbol k times as update root into spikes/<symbol>/<k>/ (one that does not resolve goes to spikes/unresolvable.txt and closes the walk) and runs it again, 16 levels at most; writes tmp/frame-profile/spikes/spikes.txt, fails while a frame is open; never boots, outside the suite.

## `make frame-profile-spread DRAW_GAINS=<gains> [UPDATE_RATIOS=<ratios>] [SPREAD_LABEL=<label>]`

tools/frame_profile_check.sh spread on the judge table and tmp/frame-profile/spikes/spikes.txt: per spike frame, its class's work (PC share times the ratio of its cart_update60 child, <child>=<ratio>,...) and the frames it takes under 263 hblanks with draw cut by the gains (default=<pct>[,<first>-<last>=<pct>]...), or does-not-fit past 600 frames; writes spikes/spread-<label>.txt (label default "default"); a missing spikes.txt fails naming make frame-profile-spikes; never boots, outside the suite.

## `make frame-profile-ordinary [FRAME_PROFILE_REPEATS=<k>] [FRAME_PROFILE_THRESHOLD=<pct>] [FRAME_PROFILE_SPIKE_THRESHOLD=<hblanks>] [UPDATE_RATIOS=<ratios>]`

profiles full_game.json k times (default 4) with cart_update60, then input_step_run as the update root into tmp/frame-profile/ordinary/<symbol>/<k>/, then tools/frame_profile_check.sh ordinary on the judge table's frames with update at most the spike threshold (default 263) and the windows draw-5564@27.0 and draw-6000@19.6: per node of both roots, PSX cost from PC shares times the ratios (<piece>=<r>[:<a>-<b>],..., default 1), calls, runs-on, above or under the threshold (default 2 % of the mean frame); while a node above is open, profiles that symbol k times as update root (one that does not resolve goes to ordinary/unresolvable.txt) and runs it again, 16 levels at most; writes tmp/frame-profile/ordinary/ordinary.txt, fails while a node is open; never boots, outside the suite.

## `make lever-estimate LEVERS=<document> [FRAMES=<frame table>] [SITUATIONS=<situation table>]`

tools/lever_estimate.sh: applies the lever-table block of LEVERS (required, no default) to the frame table joined with the situation table (defaults psx/perf/full-run-frames.txt and psx/perf/full-run-situations.txt), budget 263, one-off allowance 10800; prints the steady mean cost and 2% threshold, the steady frames over after each lever in the greedy ranking and at the end, nominal and pessimistic (every gain x 0.8), whether the document's order is the ranking, and real-game-budget on both projected tables; writes tmp/lever-estimate/estimate.txt and the projected tables; fails on a stale order or a malformed input; never boots, outside the suite.

## `make situations [EVENTS=<events>] [SITUATIONS_OUT=<file>]`

builds tmp/situations and replays the file (default worst-case/full_game.json) headless, writing one "<frame> <situation> <family>" line per frame under a "frame situation family" header to SITUATIONS_OUT (default tmp/situations-table/<name>.txt), outside the suite.

## `make situations-check`

regenerates worst-case/full_game.json's situation table into tmp/situations-table/full_game.txt and fails on any difference with psx/perf/full-run-situations.txt, naming the first differing frame (~2 s), outside the suite.

## `make situation-report [FRAMES=<frame table>] [SITUATIONS=<situation table>]`

tools/situation_report.sh: per situation its frames, frames over 263, mean update and draw; per one-off its occurrences and longest run (long past 31 frames); verdicts frame-count, families, one-off-runs (defaults psx/perf/full-run-frames.txt and psx/perf/full-run-situations.txt), outside the suite.

## `make steady-windows [FRAMES=<frame table>] [SITUATIONS=<situation table>]`

tools/steady_windows.sh on the two tables (same defaults), budget 263, windows of 15, 3 candidates: per steady situation over, its candidate windows, outside the suite.

## `make short-run-situations [PSX_SHORT_RUNS=<names>]`

runs psx-short-runs-cut and builds tmp/situations, then per run labels its run.json into <run>/situations.txt and checks with tools/short_run_situations.sh that every window frame carries the full game's steady situation of psx/perf/full-run-situations.txt; fails when a run is not "same"; never boots, outside the suite.

## `make steady-profile [PSX_SHORT_RUNS=<names>] [FRAME_PROFILE_REPEATS=<k>]`

pools the breakdown-vs-plain overhead per backend call over every manifest run named after a steady situation (tmp/frame-profile/steady/overhead.txt), then per run (default those runs) profiles run.json K times rooted at draw_layers/cart_update60 and at cart_draw/input_step_run and runs tools/steady_profile.sh profile with psx/perf/draw-functions.txt into tmp/frame-profile/steady/<run>/summary.txt: per function group and function, hblanks inside the primitives and in the game's code, the update pieces, submit and the adds-up line; flags a run the manifest marks "no representative run"; with PSX_SHORT_RUNS unset, a steady situation with no run, or one the manifest marks "no representative run", is shared by PC time over the full game's frames over 263 (no-run, tmp/frame-profile/steady/<situation>/summary.txt); needs make psx-short-runs, fails on a missing boot or an adds-up failed; never boots, outside the suite.

## `make frame-profile-test`

host tests of the PC frame profile tool, outside make test, make test-all, make e2e and psx-tools-test.

## `make psx-pad-check`

runs psx-build, boots the pad probe until it exits while tools/psx_pad_play.lua plays PSX_PAD_SCRIPT (default psx/pad_probe_script.txt) from the probe's handshake (ceiling PSX_PAD_CEILING vsyncs, timeout PSX_PAD_TIMEOUT s), then compares the probe's pad states with the script's through tools/psx_pad_compare.sh (outputs in tmp/psx/pad, ~5 s); never runs a setup, outside the suite.

## `make psx-hello`

builds psx-hello/build/psx-hello.exe against PSN00BSDK_DIR, reconfigured from scratch each time; fails pointing at psn00bsdk-toolchain-setup when the SDK is missing, outside the suite.

## `make psx-hello-cd`

builds psx-hello, then its CD image psx-hello/build/psx-hello.bin and PSX_HELLO_CUE (default psx-hello/build/psx-hello.cue), outside the suite.

## `make psx-net-check`

probes every host of PSX_SOURCES (default tools/psx_sources.txt) without downloading; fails, naming each unreachable host, outside the suite.

## `make psx-emulator-setup`

runs psx-net-check, then builds the pinned PCSX-Redux and its OpenBIOS from PSX_SOURCES into PSX_EMULATOR_DIR (default third_party/pcsx-redux), with every tools/pcsx-redux-*.patch applied, ~10 min; apt via sudo on Linux unless PSX_EMULATOR_SKIP_APT is set, the user's Homebrew on macOS (after psn00bsdk-toolchain-setup); exits 0 at once on a complete install, fails on a non-empty directory without its stamp, outside the suite.

## `make psx-boot [PSX_EXE=<exe> | PSX_ISO=<cue>]`

boots that input (or nothing) headless in PSX_EMULATOR_DIR for PSX_FRAMES vsyncs (default 600), writes PSX_OUT.vram (capture: "psx-vram <w> <h> <depth>" line, then raw bytes), PSX_OUT.tty.txt and PSX_OUT.log (default PSX_OUT tmp/psx/boot); fails naming the input after PSX_BOOT_TIMEOUT s (default 120), outside the suite.

## `make psx-emulator-check`

fetches the test media of PSX_SOURCES into tmp/psx/media (sha256-checked), boots the test .exe twice, the test .cue twice and nothing once in PSX_EMULATOR_DIR (outputs in tmp/psx/check, ~1 min); fails, one line per failed condition, unless both pairs of captures match, PSX_KNOWN_LINE is in the exe TTY and the .cue capture differs from the empty one; never runs the setup, outside the suite.

## `make psx-hello-check`

builds psx-hello, boots it twice and nothing once in PSX_EMULATOR_DIR (outputs in tmp/psx/hello-check, ~35 s); fails, one line per failed condition, unless the .exe has the PS-EXE magic, its captures match, differ from the empty one and are not one color, and PSX_HELLO_TTY_LINE is in its TTY; never runs a setup, outside the suite.

## `make psx-hello-cd-check`

builds psx-hello-cd, then the same check on PSX_HELLO_CUE (outputs in tmp/psx/hello-cd-check, ~35 s), with a cue condition (the .cue names a .bin of whole 2352-byte sectors) in place of the magic; never runs a setup, outside the suite.

## `make psx-hello-e2e`

runs psx-hello-cd-check, then compares the sha256 of its first capture with PSX_HELLO_REFERENCE (default psx-hello/psx-hello-cd.vram.sha256); fails on a failed proof, a mismatch, a missing reference or a PSX_FRAMES other than 600; never runs a setup, ~35 s, outside the suite.

## `make psx-hello-e2e-baseline`

same proof, then sets PSX_HELLO_REFERENCE; never run by any other target.
