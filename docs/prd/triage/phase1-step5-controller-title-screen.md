---
title: "The PSX game EXE driven by the controller, its title screen passed through scripted input"
description: "No PSX executable reads the live pad into the game's input yet. So nothing proves that the game, as it will ship, can be played with the controller, nor that it leaves the title screen."
status: needs-triage
---

## Problem Statement

Plan 10, phase 1, step 5, verbatim:

> 5. **Controller** wired, then the title screen passed in the emulator through scripted input. Proof independent of emulator timing: the PSX game logs over TTY the `pad_buttons` of each input frame; the check turns that log into an `events.json`, replays it on PC with `STATE=` and compares the game lines with the PSX state (`psx_state_compare.sh`), and checks the title screen was left. This step also decides how the game build ends under check (e.g. the Lua quits on a TTY or scratchpad marker), so the EXE checked is the one shipped.

After step 4, the PSX build has three PS-EXEs, and none of them is a game a player can drive:

- `psx-replay` and `psx-gpu-replay` play a recording compiled into the EXE (`embedded_replay_frames`). They never read the pad.
- `psx-pad-probe` reads the pad through the BIOS driver (`psx_pad_read_pressed`, hardware bit order, 1 = pressed) and prints it. It runs no game.
- `pad_buttons` in `p8_input_frame_t` is filled on PC only, by `keymap_pad_buttons` from `keymap.cfg`. `docs/psx-game.md` says so: "The pad driver does not feed `pad_buttons` yet: the game itself is not driven by the pad."

The plan's goal is a `.bin/.cue` that "boots in the emulator, shows the title screen and is driven by the controller". Step 6 puts the step-5 EXE into the CD image. Step 5 must therefore produce that EXE: the game, reading the live pad, with no embedded recording. It must also prove, in the emulator, that the controller drives the game past the title screen.

Two constraints shape the proof:

- **The emulator's pace is not the game's.** How many vsyncs a game frame takes depends on the emulator and the build. A check that expects a given state at a given vsync would be flaky. The plan therefore asks for a proof that does not depend on timing: the PSX game logs what it read on each input frame, and the PC replays exactly that.
- **The game never ends on its own.** A real game loops forever. A check needs it to stop, print its state and exit. The plan asks this step to decide how, so that the EXE under check is the EXE that ships, not a variant built for the check.

## Solution

A fourth PS-EXE, **`psx-game`**, is the game as it ships. It has the GPU backend of step 4 and the BIOS pad driver of step 1, and no embedded recording. On each input frame, it reads port 1 and maps the hardware word to `pad_buttons` with the mapping `keymap.cfg` gives on PC (the PSX button named `cross` in `keymap.cfg` is the controller's ✕, and so on). Then it runs the same frame as the replays: `input_step_run`, `cart_update60`, `cart_draw`, then the GPU frame end.

It writes one TTY line each time `pad_buttons` changes (and always on frame 0): `psx-game: pad <frame> <hex>`. This is a run-length log of the `pad_buttons` of every input frame.

**How it ends under check.** It ends on the scratchpad end word that step 1's pad player (`tools/psx_pad_play.lua`) already writes ten vsyncs after a script's last step. The game clears that word at start, then raises the pad handshake word. At the top of each frame it checks the end word. Once the word holds the end magic, the game:

- prints the replays' end protocol: `psx-replay: done game <frames> vsyncs <n>`, then the `psx-state:` block;
- exits through the PCSX exit register with code 0.

On a console nothing writes that word, so the branch never runs. The EXE that is checked is the EXE that ships: there is no build flag and no second variant.

**The proof.** `make psx-check` boots `psx-game` once, with a committed pad script that walks the title screen as `tests/e2e/pad_title.json` does (Down, Down, Right, Up, then ✕ on "reasonable"), each press held long enough to span several frames. Then:

1. A host tool turns the TTY log into an `events.json`, written by the PC's own recording writer.
2. It also checks that the ordered pad states in the log are the script's states, mapped by the same table. This proves that the hardware-to-`pad_buttons` mapping runs on MIPS as on the host.
3. `tmp/replay` replays that `events.json` with `STATE=`.
4. `psx_state_compare.sh` compares the game lines with the PSX state.
5. A last condition reads the PSX state's `difficulty` and `clock` lines: the title screen was left, with the difficulty the script picks.

## Step 4 as committed

Step 4 is done (HEAD `bc449e0`; see the "Step 1.4" note of `docs/plan-10.md` and `docs/psx-game.md`). This PRD builds on the facts below, checked against the committed code. The H-4-k labels are kept so that the references elsewhere in this PRD stay traceable.

- **H-4-1. Families.** The GPU backend draws all nine entries:
  - `rect`, `rectfill` and `pset` as `TILE`/`TILE_1`, and `line` as `LINE_F2`;
  - `circ` and `circfill` on the software midpoint loop;
  - `spr` and `map` as `SPRT`, with a per-frame draw-palette CLUT cache;
  - `print` from a 4-bit font texture and 16 font CLUTs;
  - patterned fills as 8×8 pattern slots under `DR_TWIN`.

  Every family capture differs by 0 pixels from the PC, except `line`, which differs by 2 near1 pixels. Each capture is pending human acceptance. Step 5's proof uses no pixel, so that review does not block it.
- **H-4-2. GPU backend API** (`src/platform/psx_gpu.h`).
  - `psx_gpu_init()` uploads the sheet, the font texture and its CLUTs. `psx_gpu_backend()` returns a static table.
  - `psx_gpu_frame_begin(int frame)` and `psx_gpu_frame_end(unsigned long metered_bytes)`. `frame_end` clears the display buffer and appends the ×2 upscale. It waits for the previous frame, shows it and submits this one, with no `VSync` wait. It stops with `p8_fatal` when the frame sent more bytes than metered.
  - `psx_gpu_frame_show_last()` is used only by the capture handshake. The game does not call it.
- **H-4-3. GPU replay frame loop** (`src/cmd/psx_gpu_replay.c`).
  - Install order: `p8_draw_backend_set(psx_gpu_backend())`, `draw_filter_install()`, `p8_draw_meter_install()`, then `psx_gpu_frame_begin(0)` before `cart_load` and `cart_init`. Then `capture_start()` runs: a scratchpad handshake that waits for the capture Lua.
  - Each frame runs:
    1. `input_step_run`, then `cart_update60`;
    2. `draw_filter_set_mask` (the capture mask on a capture frame, all otherwise);
    3. `cart_draw`, `p8_draw_meter_frame_end()`, `draw_filter_frame_end()`;
    4. `psx_gpu_frame_end(p8_draw_meter_last_frame().bytes)`;
    5. on a capture frame, `capture_frame` (which calls `psx_gpu_frame_show_last` and waits for the Lua);
    6. `psx_gpu_frame_begin(frame + 1)`.
  - The game EXE runs this loop without `capture_start`, the filter calls and `capture_frame`. Its chain is meter → GPU backend. It must never call `capture_start`: with no capture Lua, that call would wait forever.
- **H-4-4. Scratchpad protocol.**
  - The pad words are unchanged, and are still defined separately in `src/cmd/psx_pad_probe.c` and `tools/psx_pad_play.lua`:
    - the handshake at 0x3fc, magic `0x50414421`;
    - the end word at 0x3f8, magic `0x454e4421`;
    - `END_DELAY` 10 vsyncs.
  - `src/platform/psx_capture_protocol.h` holds only the three capture words (0x3f4, 0x3f0, 0x3ec) and their state values. `tools/psx_capture_protocol_check.sh` compares it with `tools/psx_capture_frames.lua`. That check is driven by `test_psx_capture_protocol` in **`make psx-tools-test`**, not in `make test`.
  - Step 5 therefore adds a separate pad-protocol header and its own check, in the same shape (below). The game links no capture code, so nothing else in it touches these words.
- **H-4-5. Boot tooling.**
  - `tools/psx_until_exit.lua` loads each player only when its variable pair is non-empty: `PSX_PAD_PLAYER`/`PSX_PAD_SCRIPT` for the pad, `PSX_CAPTURE_PLAYER`/`PSX_CAPTURE_SCRIPT` for captures.
  - `psx_boot.sh --until-exit [--pad-script <file>] [--capture-script <file>]` works for any EXE. The game boot passes only `--pad-script`.
  - `psx_boot.sh`'s last-progress pattern is still `^psx-(replay|pad): `.
  - `psx_tty_write` masks interrupts around the BIOS `putchar`, a fix for doubled TTY characters. The game's pad log relies on it.
- **H-4-6. Source lists.** `PSX_GPU_REPLAY_SRCS` is:
  - the driver, `CORE_SRCS`, `state_format.c` and `format_self_test.c`;
  - `src/devtools/draw_filter.c` (check-only);
  - the GPU backend: `psx_gpu.c`, `psx_tiles.c`, `psx_sprite.c`, `psx_text.c`, `psx_clut_cache.c`, `psx_pattern.c`, `psx_pattern_cache.c`, `psx_gpu_textures.c`, `psx_colour.c`, `psx_vram_layout.c` (all in `src/platform/`) and `src/p8/palette.c`;
  - `PSX_PLATFORM_SRCS` and the embedded replay.

  The capture handshake lives in the GPU replay driver itself. None of these files is in `CORE_SRCS`.
- **H-4-7. `make psx-check`.**
  - It has 21 conditions: step 3's fourteen plus the seven `gpu-capture-*` ones. It ends with `psx-check: all twenty-one conditions hold`, and writes `conditions.txt` with `<condition> ok|failed`.
  - `tools/psx_check.sh` takes 24 positional arguments.
  - The run takes about 88 s with four boots: replay, GPU capture, GPU determinism and pad probe. One GPU boot takes 1,103 vsyncs and 19 s; the limits are `PSX_GPU_CEILING` 1655 and `PSX_GPU_TIMEOUT` 29.
  - `make psx-tools-test` (the shell-driven tests of the PSX check scripts) is a prerequisite of `psx-check` and outside `make test-all`. That target is where every new shell-driven test goes.

  Step 5 adds six conditions, giving 27, and one boot, giving five.
- **H-4-8. Start gate.** `make psx-check` was green at the end of step 4, with every family pending human acceptance. Step 5 still re-runs it first. A capture pending acceptance does not block it.
- **H-4-9. RAM.** `ram-room` left 483,064 bytes (GPU ELF text 366,564, bss 1,113,580). Compared with the GPU replay, the game ELF drops the embedded recording (16 bytes a frame), the filter, the capture handshake and the format self-test. It adds the pad driver, the pad map and the pad log. It is expected to fit with a similar margin. A failure of `game-ram-room` stops the step with a report: it is not worked around.

## User Stories

1. As the plan-pilot run, I want step 5 to start by checking that `make psx-check` is green (H-4-8), so that the controller is not built on a broken step-4 pipeline.
2. As the step-5 implementer, I want every "Step 4 as committed" fact re-read against the code first, and any difference recorded in the step note, so that the PRD's assumptions never silently drift.
3. As a player, I want a PSX executable that boots straight to the title screen and reads the controller, so that I can play the game rather than watch a replay.
4. As a player, I want ✕, ○, □, △, the D-pad, L1, R1 and START to do on PSX what the buttons of the same names do on PC through `keymap.cfg`, so that the game feels the same on both.
5. As a player, I want SELECT, L2, R2, L3 and R3 to do nothing, as on PC where `keymap.cfg` binds nothing to them, so that a stray press never reaches the game.
6. As a player, I want the game to run at most one frame a vsync (60 Hz NTSC), so that the PICO-8 cart's 60-frame timing is never exceeded on a fast frame.
7. As a developer, I want the mapping from the hardware pad word to `pad_buttons` in one pure, host-compiled module, so that it is unit-tested on PC and linked unchanged into the PSX EXE.
8. As a developer, I want the mapping's unit test to derive the expected bit of each button from `keymap_button_name`, so that the PSX mapping cannot drift from the PC's button set.
9. As a developer, I want the pure module's hardware bit constants checked at compile time against the SDK's `PAD_*` constants in the PSX-only pad driver, so that a wrong bit cannot hide behind a host test.
10. As a developer, I want the game EXE to read the pad once per input frame, before `input_step_run`, so that one frame sees one consistent pad word, as on PC.
11. As a developer, I want the game EXE's frame to be the GPU replay's frame without the check-only family filter and capture handshake (H-4-3), so that the game draws and plays exactly as the checked replays do.
12. As a developer, I want the game EXE to install the draw meter around the GPU backend, so that step 3's real-bytes-under-metered-bytes guard protects the shipped game too.
13. As a developer, I want the game EXE to start with an erased save storage, as a PC replay without `initial_save` does, so that its first frames match the replay of its own log.
14. As a developer, I want the game EXE to write `psx-game: pad <frame> <hex>` on frame 0 and on every frame where `pad_buttons` changes, so that the full per-frame input is in the TTY at little cost.
15. As a developer, I want each pad line to carry its frame number, so that a lost or reordered TTY line is detected rather than silently shifting the input.
16. As the check, I want the game EXE to end when the pad player's end word appears in the scratchpad, so that the game stops at a point the script chooses, not at a vsync count.
17. As the check, I want the game EXE to test the end word at the top of a frame, before reading the pad, so that every frame it logged was fully run and is replayed on PC.
18. As the check, I want the game EXE to end with the replays' protocol (`psx-replay: done game <frames> vsyncs <n>`, then `psx-state:` lines, exit code 0), so that `psx_state_compare.sh` and `psx_check.sh`'s done check read it unchanged.
19. As the check, I want a `p8_fatal` or a failed assert in the game EXE to print the stopped state and exit with code 3 or 4, as the replays do, so that a crash is never mistaken for a pass.
20. As a player, I want the end-word branch to stay inert on a console: the word is cleared at start and nothing else writes it, so that the shipped game never quits by itself.
21. As a release owner, I want the EXE under check to be byte-for-byte the EXE step 6 packs, with no check-only build flag, so that the proof covers what ships.
22. As a developer, I want the pad scratchpad words and magics named once on the C side, in one PSX header that the probe and the game share, and checked equal to `psx_pad_play.lua`'s constants by a static check in `make psx-tools-test`, so that the game, the probe and the Lua never drift apart.
23. As a developer, I want a committed game pad script that walks the title screen the way `pad_title.json` does, with each press and each release held 20 vsyncs, so that every state spans several input frames whatever the emulator's pace.
24. As a developer, I want the script to wait long enough before its first press for the title rows to finish sliding in, and to idle after ✕ until the game has started, so that the game is in its first month when it ends.
25. As a developer, I want a pure parser for the game's pad log that refuses a malformed line, a missing frame 0, a frame that does not increase, a bit outside the eleven `P8_PAD_*` bits, a missing or repeated done line, and a pad line at or after the done frame count, each named with its line, so that a broken log never becomes a replay.
26. As a developer, I want a host tool that writes the log as an `events.json` through `p8_input_save`, so that the converted recording has the exact format of the committed e2e recordings and loads through the PC's own loader.
27. As a developer, I want the converted file to have no `initial_save`, so that the PC replay erases its storage, as the game EXE did.
28. As a developer, I want the same tool to read the pad script, map each step through the same pure module, and compare the ordered distinct states with the log's, naming the first difference, so that the mapping is proven on MIPS and not only on the host.
29. As the check, I want `game-done`, which passes only when the game boot exited with 0, printed its done line, and printed no stopped state, so that a game that never saw the end word or crashed fails.
30. As the check, I want `game-pad-log`, which passes only when the log converts into an `events.json` whose frame count equals the done line's, so that every frame the game ran is accounted for.
31. As the check, I want `game-pad-states`, which passes only when the log's ordered pad states equal the script's mapped states, so that a missed press, a wrong bit or a wrong mapping fails, naming the first differing state.
32. As the check, I want `game-state-game-lines`, which replays the converted file on PC with `STATE=` and compares every game line with the PSX state through `psx_state_compare.sh`, so that the PSX game played exactly what the PC plays from the same input.
33. As the check, I want `game-title-left`, which passes only when the PSX state's `difficulty` line names the script's difficulty (`reasonable`) and its `clock` line is not `clock year - month -`, so that "the title screen was left" is a fact read from the game, not inferred from a press.
34. As the check, I want `game-ram-room`, the `ram-room` arithmetic applied to the game ELF, so that the EXE that ships is proven to fit the 2 MiB of RAM.
35. As the check, I want `magic` and `no-json` extended to the game EXE, so that it is a valid PS-EXE and carries no JSON loader, like the replays.
36. As a reviewer, I want the converted `events.json`, the PC state, the PSX state and the game's vsyncs kept in the check's output directory, so that a failing run can be replayed and dumped on PC with `make replay`.
37. As a reviewer, I want `measurements.txt` to gain the game's vsyncs, frame count, `ram-room` line and ELF size, each behind `game `, so that the cost of the shipped EXE is visible.
38. As a developer, I want a standalone `make psx-game-check` that builds, boots the game with the script and checks the game conditions alone, so that I can iterate on the controller without paying for the whole `psx-check`.
39. As a reviewer, I want `make psx-check` to add exactly one boot, the game's, and the step note to record its run time and boot count (plan rule "PSX check cost"), so that the check's cost stays visible and justified.
40. As a developer, I want the game's ceiling and timeout measured as the first run plus half again, like every other boot, so that a stalled game fails quickly and a normal run never hits the limit.
41. As a developer, I want the game's post-boot checks in their own script, driven by a `make psx-tools-test` test on crafted TTY files, so that each condition's pass and fail paths are proven without the emulator.
42. As a developer, I want that test's passing case built from `tests/e2e/pad_title.json` and its PC state, so that the log → `events.json` → replay → compare chain is proven end to end on the host.
43. As a developer, I want the parser test to rebuild `tests/e2e/pad_title.json` byte for byte from a log of its frames, so that the converter is proven to write the e2e format exactly.
44. As a developer, I want every `*.sha256` in `tests/e2e/` and every `make state` game line unchanged, so that the PC is proven untouched.
45. As a developer, I want `make test` and `make e2e` no slower than at the start of the step, beyond the new millisecond-scale pure C unit tests (every shell-driven test goes to `make psx-tools-test`), so that the fast loop stays fast.
46. As a reviewer, I want `docs/psx-game.md` to describe the game EXE, its pad mapping, its pad log, how it ends under check, the new conditions and `make psx-game-check`, and to drop "the game itself is not driven by the pad", so that the doc states what is true.
47. As a human, I want neither the raylib window nor the emulator opened in a window, so that verification stays headless.

## Implementation Decisions

### Start

- Step 5 starts by running `make psx-check` (H-4-8) and re-reading H-4-1 to H-4-9 against the code. If the check is red or a setup is missing, the run stops with a report. Any fact found different is adapted to, and recorded.

### Which EXE reads the live pad

- A new PS-EXE, `psx-game`, built by `make psx-build` next to the three others, from a new driver `src/cmd/psx_game.c`. It is a game EXE, distinct from the embedded-replay EXEs:
  - it links no `embedded_replay` source and no format self-test;
  - it links neither of step 4's check-only devtools: the family filter and the capture handshake.
- The replays and the probe are unchanged, except that the probe takes its pad constants from the shared header (below).
- The Makefile gets one variable for the GPU backend's sources (H-4-6), used by both `PSX_GPU_REPLAY_SRCS` and the new `PSX_GAME_SRCS`, so that the two lists cannot drift. `PSX_GAME_SRCS` is:
  - the driver;
  - `CORE_SRCS`, which holds the memory save storage and the embedded sprite sheet;
  - `src/devtools/state_format.c`, because the state is printed at the end under check;
  - the GPU backend sources;
  - `src/platform/psx_pad.c` and the new `src/platform/psx_pad_map.c`;
  - `PSX_PLATFORM_SRCS`.

  `psx/CMakeLists.txt` gains the `psx-game` target and the required `PSX_GAME_SOURCES` list, built with the same flags as the others. Asserts stay on (`-UNDEBUG`), as in every PSX target. Whether to ship with asserts off is a plan-11 question.
- The game EXE carries the state formatter and the TTY lines into the shipped game. On a console the TTY goes to the BIOS output and is harmless. Their size counts in `game-ram-room`.

### Pad map (`src/platform/psx_pad_map.{h,c}`, new, pure, host-compiled)

- It names the sixteen hardware bits in PSn00bSDK's order (1 = pressed, as `psx_pad_read_pressed` returns them):

  | bit | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
  |---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
  | button | SELECT | L3 | R3 | START | UP | RIGHT | DOWN | LEFT | L2 | R2 | L1 | R1 | TRIANGLE | CIRCLE | CROSS | SQUARE |

- `int psx_pad_map_buttons(uint16_t pressed)` returns the `P8_PAD_*` mask. Each hardware button whose lowercase name is a `keymap.cfg` button name (`up down left right cross circle square triangle l1 r1 start`) sets that button's `P8_PAD_*` bit. SELECT, L3, R3, L2 and R2 set nothing. This is the mapping `keymap.cfg` gives on PC: there, `keymap.cfg` binds PC keys and gamepad buttons to these eleven PSX buttons; on PSX, the buttons are the controller's own.
- `uint16_t psx_pad_map_hardware_bit(const char *name)` returns the hardware bit of an uppercase button name (the names `PCSX.CONSTS.PAD.BUTTON` and the pad scripts use), and 0 for an unknown name. The host tool uses it to read pad scripts.
- `src/platform/psx_pad.c` (PSX only) asserts at compile time that each named bit equals the SDK's `PAD_*` constant of the same button, as step 3 checks the cost model against `sizeof`.
- No `keymap.cfg` is read on PSX: there is no file system, and the controller's buttons need no remapping.

### Game driver (`src/cmd/psx_game.c`, new)

- Start:
  1. `ResetGraph(0)`;
  2. graphics and VRAM structs, the RAM sprite sheet loaded, `psx_gpu_init()`;
  3. `save_storage_erase()`;
  4. a one-frame `p8_input_t` holding a zero frame, set as the dispatch input, as the PC replay sets its recording before `cart_load`. All pad recordings have a zero mouse;
  5. `psx_exit_set_hook` with the stopped-state writer of the replays;
  6. GPU backend set, meter installed;
  7. `psx_gpu_frame_begin(0)`, `cart_load()`, `cart_init()`;
  8. `psx_pad_start()`;
  9. the end word cleared, then the handshake word set to its magic;
  10. `psx-game: ready`.
- Each frame, `frame` counting from 0:
  1. if the end word holds the end magic, leave the loop;
  2. pace: if `VSync(-1)` still equals the vsync count read at the previous frame's start, `VSync(0)`. The game then never runs faster than 60 Hz, and a slow frame is never delayed further;
  3. `pad = psx_pad_map_buttons(psx_pad_read_pressed())`. On frame 0, or when `pad` differs from the previous frame's, print `psx-game: pad <frame> <pad as %03x>`;
  4. `input_step_run` on `{0, 0, 0, pad}`, `cart_update60`, `cart_draw`, `p8_draw_meter_frame_end`, `psx_gpu_frame_end(metered bytes)`, `psx_gpu_frame_begin(frame + 1)` (H-4-2, H-4-3).
- End, reached only under check: `psx-replay: done game <frames> vsyncs <n>` (the vsyncs since the first frame), the `psx-state:` block through `psx_tty_state_sink`, then `psx_exit(PSX_EXIT_OK)`. `<frames>` is the number of frames run, so the log's frames are 0 to `<frames>` − 1.
- The end protocol uses the replays' line on purpose. To the check, a game that ended is a replay of its own log, and `psx_state_compare.sh` and the done check of `psx_check.sh` read it unchanged. The game's own lines use the `psx-game:` prefix, which `psx_boot.sh` adds to its last-progress pattern.

### How the game build ends under check

- Decision: through the **scratchpad end word of step 1's pad protocol**, written by the unchanged `tools/psx_pad_play.lua` ten vsyncs after the script's last step. The Lua already plays any EXE's script from the handshake word. The game raises that word, so no new Lua is needed and the game boot is `psx_boot.sh --until-exit --pad-script <script> --exe psx-game.exe`.
- Rejected: the Lua quitting the emulator itself, on a TTY or scratchpad marker. The game would then never print its state, and the state is the proof.
- Rejected: a `-D` build flag or a second "check" EXE. Plan step 5 requires that the EXE checked is the one shipped. Step 6 packs `psx-game.exe` as built.
- On a console, the scratchpad is cleared by the game before the handshake, and nothing writes it afterwards (H-4-4: the game links no capture code, the only other scratchpad user). The branch is dead code there.
- The pad words and magics move to a new header, `src/platform/psx_pad_protocol.h`, shared by `psx_pad_probe.c` and `psx_game.c` (H-4-4). It defines `PSX_PAD_HANDSHAKE_OFFSET`, `PSX_PAD_END_OFFSET`, `PSX_PAD_HANDSHAKE_MAGIC` and `PSX_PAD_END_MAGIC`. The capture header is left alone.
- A new `tools/psx_pad_protocol_check.sh`, in the shape of `psx_capture_protocol_check.sh`, compares those constants with `psx_pad_play.lua`'s `HANDSHAKE_OFFSET`, `END_OFFSET`, `HANDSHAKE_MAGIC` and `END_MAGIC`, and names the constant that drifted. `test_psx_pad_protocol` drives it in `make psx-tools-test`.

### Game pad script (`psx/game_title_script.txt`, new, committed)

- It uses the pad-script grammar of step 1. It mirrors `pad_title.json`: idle, Down, Down, Right (nothing to the right), Up, then ✕ on "reasonable", then idle:

  ```
  90 DOWN
  110 -
  130 DOWN
  150 -
  170 RIGHT
  190 -
  210 UP
  230 -
  250 CROSS
  270 -
  330 -
  ```

  The first press waits 90 vsyncs: the title rows slide in over 10 frames on PC (`pad_title.txt`). Each press and each release lasts 20 vsyncs: pad navigation acts on edges, so a press held for several frames moves the focus once. The last 60 idle vsyncs let the started game run into its first month before the end word.
- If the first run shows a state shorter than one frame, or the title rows not yet in place at the first press, the implementer lengthens the offsets, never the code, and records it in the step note.
- The expected difficulty, `reasonable`, is a `make` variable (`PSX_GAME_DIFFICULTY`) passed to the check, next to `PSX_GAME_SCRIPT`.

### Pad log module and host tool

- **`src/devtools/pad_log.{h,c}`** (new, pure, PC only). It parses TTY text (CR stripped) and ignores every line that is not a `psx-game: pad` line or the `psx-replay: done` line. It returns the frame count and the `pad_buttons` of every frame: each pad line's value holds until the next pad line. It refuses, naming the line:
  - a malformed pad line;
  - a first pad line not at frame 0;
  - a frame that does not increase;
  - a value with bits outside `(1 << 11) − 1`;
  - no done line, or more than one;
  - a pad line at or after the done line's frame count.

  It also gives the ordered list of distinct consecutive states.
- **Script reader**, in the same module. It reads the pad-script grammar (`<offset> <BUTTON>...` or `<offset> -`; offsets increase; the last step is `-`; blank and `#` lines skipped) and maps each step through `psx_pad_map_hardware_bit` and `psx_pad_map_buttons`. It returns the expected ordered distinct states, starting from 0, the state before the first step.
- **Host tool `src/cmd/psx_pad_log.c` → `tmp/psx_pad_log <tty> <pad-script> <events.json>`.** It prints one line per aspect, in the style of `psx_digest_compare.sh`:
  - `psx-pad-log: log: ok, <frames> frames`: the log parsed, and `<events.json>` was written through `p8_input_save` (no `initial_save`). Otherwise `psx-pad-log: log: <error naming the line>`, and nothing is written;
  - `psx-pad-log: states: ok, <n> states`, or `psx-pad-log: states: first differing state, <i>: log <hex> script <hex>`. This line is skipped when the log failed.

  It exits 0 when both are ok, 1 otherwise, and 2 on usage.

### Post-boot checks (`tools/psx_game_check.sh`, new)

- Usage: `psx_game_check.sh <tty> <boot-status> <boot-output> <pad-script> <pad-log-tool> <replay-tool> <difficulty> <out-dir>`. It runs after the boot, so that it can be driven on crafted files. Every condition is evaluated before concluding, with one `psx-game-check: <condition>: ok[, detail]` line each, or the failure on stderr. Exit 1 if any failed.
  - `game-done`: boot status 0, one `psx-replay: done game <frames> vsyncs <n>` line, no `psx-state: stopped p8_fatal` line.
  - `game-pad-log`: the tool's `log: ok` line, and its frame count equals the done line's.
  - `game-pad-states`: the tool's `states: ok` line.
  - `game-state-game-lines`: run `STATE=<out-dir>/pc_state.txt <replay-tool> <out-dir>/game_events.json /dev/null <out-dir>/pc.dump.txt`. Its `replay: <n> frame(s) run` must equal the frame count. Then `psx_state_compare.sh <tty> <out-dir>/pc_state.txt <out-dir>/game-state <frames>` must print `game lines match`. Skipped, and failed, when `game-pad-log` failed.
  - `game-title-left`: in the PSX state (`psx-state:` lines), the `difficulty` line is exactly `difficulty <difficulty>`, and the `clock` line is not `clock year - month -`. Both lines are quoted on failure.
- `psx_check.sh` boots the game after the pad probe:
  - with `PSX_GAME_CEILING` / `PSX_GAME_TIMEOUT`, measured as the first run plus half again;
  - with `--pad-script $(PSX_GAME_SCRIPT)`.

  It calls `psx_game_check.sh` into `<out-dir>/game` and relays its five lines as its own conditions.
- `psx_check.sh` adds `game-ram-room` itself, with the `ram-room` arithmetic on the game ELF. `magic` and `no-json` cover the game EXE and ELF. `check_replay_done` takes the expected frame count as a parameter instead of reading the PC's global.
- `psx_check.sh` (24 arguments, H-4-7) gains five arguments: the game EXE, its script, its ceiling, its timeout, and the expected difficulty. It reuses `tmp/replay` and gains `tmp/psx_pad_log` as a sixth argument. `make psx-check` depends on `tmp/psx_pad_log`. Six conditions are added (H-4-7), 27 in all. The header comment, `conditions.txt` and the final line (`all twenty-seven conditions hold`) name the new total.
- `measurements.txt` gains, each behind `game `:
  - the vsyncs and frames of the done line;
  - the `ram-room <used> <limit> <left>` line;
  - the ELF `size` lines.
- **`make psx-game-check`** (new, outside `test-all`): `psx-tools-test`, `psx-build`, the game boot with `PSX_GAME_SCRIPT`, then `psx_game_check.sh` into `tmp/psx/game`. The Makefile legend describes it and the game boot of `psx-check`.

### Boots and cost (plan rule "PSX check cost")

- One boot is added: the game EXE. No existing boot can carry this proof:
  - the replays read embedded frames, not the pad;
  - the probe runs no game;
  - and the shipped EXE must be the one booted.
- The pad probe's boot is kept. It proves the raw word for fourteen buttons, including the five the game ignores, and the game boot sees only the eleven mapped bits.
- Expected cost: the emulator start (about 4 s) plus about 340 vsyncs after the handshake, so about 6 to 8 s on top of H-4-7's ~88 s. The PC replay and the tool run in milliseconds. The step note records the measured `make psx-check` time and its boot count (five).

### What stays put

- No file of `src/game/` or `src/engine/` changes. `pad_nav.c` and `pad_panel.c` turn `pad_buttons` into the game's input, unchanged.
- The PC `game`, `replay`, `keymap.cfg` and every e2e are unchanged.
- The replay EXEs and their conditions are unchanged.
- Nothing PSX enters `make test-all`. Step 6's `.bin/.cue` is not started.

## Implementation Chunks

1. **Pad map, pad log and host tool (host only).** Start gate and a re-read of "Step 4 as committed" first. Then:
   - `psx_pad_map` and `test_psx_pad_map`;
   - `pad_log` (log and script readers) and `test_pad_log`, including the byte-for-byte rebuild of `pad_title.json`;
   - `tmp/psx_pad_log`.

   Done when `make test-all`, `make state`, `make full-game` and `make sanitize-net` are green. No PSX file changes yet, apart from the pure module, which the PSX build does not link yet.
2. **Game EXE and its standalone check.**
   - the pad-protocol header, `tools/psx_pad_protocol_check.sh` and `test_psx_pad_protocol` (in `make psx-tools-test`);
   - the compile-time SDK bit check in `psx_pad.c`;
   - `psx_game.c`;
   - the shared GPU-backend source variable, `PSX_GAME_SRCS`, and the CMake target;
   - `psx_boot.sh`'s progress pattern;
   - `psx/game_title_script.txt`;
   - `tools/psx_game_check.sh` and `test_psx_game_check`;
   - `make psx-game-check`, with its ceiling and timeout measured.

   Done when `make psx-game-check` is green and `make psx-check` is still green.
3. **Integration into `make psx-check` and hand-off.**
   - The game boot and the six conditions in `psx_check.sh`;
   - `magic` and `no-json` extended;
   - the measurement lines;
   - the Makefile legend.

   Then `docs/psx-game.md`:
   - the game EXE, the pad map, the pad log, the end word, the conditions, the targets;
   - "Still to do": step 6, with "The pad driver does not feed `pad_buttons` yet" removed.

   Then the step-note draft: any "Step 4 as committed" difference, the script's final offsets, the game's vsyncs, frames, ceiling and timeout, the `game-ram-room` line, ELF size, and `make psx-check`'s run time and boot count.

## Testing Decisions

- A good test drives a public interface and checks what comes out of it: the bits a mapping returns, the frames and errors a parser returns, the bytes a writer writes, and the lines and exit code of a script on crafted files. It never checks internal tables or how the log is stored.
- **`test_psx_pad_map`** (new, host, `make test`). It links `keymap.c` on the host.
  - For each `i` below `KEYMAP_BUTTON_COUNT`, the hardware bit named by the uppercase of `keymap_button_name(i)` maps to `1 << i`.
  - SELECT, L3, R3, L2 and R2 map to 0.
  - A combination maps to the OR of its parts, and `0xFFFF` maps to `(1 << 11) − 1`.
  - The name lookup returns 0 for an unknown or lowercase name.
- **`test_pad_log`** (new, `make test`).
  - A log built from `tests/e2e/pad_title.json`'s frames (frame 0 plus every change, then a done line) writes, through the tool's writer, a file byte-identical to `tests/e2e/pad_title.json`.
  - Values hold between lines, and foreign TTY lines are ignored.
  - Each refusal names its line: missing frame 0, frame not increasing, bad hex, out-of-range bits, no done line, two done lines, a pad line at or after the done frame count.
  - The script reader on `psx/game_title_script.txt` gives `000 002 000 002 000 008 000 001 000 010 000`, and refuses a bad offset, an unknown button, or a last step that is not `-`.
  - A step holding only SELECT maps to 0 and adds no state.
- **`test_psx_game_check`** (new, `make psx-tools-test`, script on crafted files, with the real `tmp/replay` and `tmp/psx_pad_log`). The passing TTY is built from `pad_title.json`: its pad lines, a `psx-replay: done game 30 vsyncs 1` line, and the PC state of `pad_title.json` behind `psx-state:`. All five conditions pass with difficulty `reasonable`. Then one change per case:
  - boot status 1;
  - no done line;
  - a `stopped p8_fatal` line;
  - a pad line dropped, so the states differ from the script;
  - one state line altered (`game-state-game-lines` names it);
  - the log cut before ✕ (`difficulty -` and `game-title-left` fail);
  - the expected difficulty `tough`.

  The test uses a script matching `pad_title`'s states. `pad_title` is 30 frames, so the replay runs in milliseconds.
- **Static check.** `test_psx_pad_protocol` (new, `make psx-tools-test`): the committed header and Lua agree, and a drifted or missing value is named. Prior art: `test_psx_capture_protocol`.
- **PSX side.** The six new conditions of `make psx-check`, and `make psx-game-check`, are the integration tests of the driver, the pad driver on MIPS, the end word and the Lua. No unit test runs the toolchain or the emulator.
- **PC net.** Every e2e `*.sha256`, `make state`, `make full-game` and `make sanitize-net` stay unchanged and green: no PC behaviour changes.
- Prior art:
  - `test_keymap` (button names and bits);
  - `test_psx_vram_layout` (host-compiled PSX pure code);
  - `test_psx_state_compare`, `test_psx_pad_compare` and `test_psx_digest_compare` (scripts on crafted files);
  - `test_input` (the `p8_input_save` format);
  - `test_psx_capture_protocol` and `test_psx_capture_check` (step 4, `make psx-tools-test`).

## Out of Scope

- **The game's `.bin/.cue`** and repeating the proof on the `.cue` (step 6).
- **Saves on the memory card.** The game EXE uses the memory save storage, erased at boot, as the replays do.
- Analog sticks, DualShock modes, port 2, multitap, rumble, and remapping buttons on PSX.
- Audio (out of scope for the port).
- Judging or fixing step 4's GPU divergences, or freezing their references (the human's job).
- Frame-rate tuning beyond the 60 Hz cap, shipping with asserts off, and removing the RAM framebuffer or RAM sheet from the game EXE (plan 11).
- `make psx-full-game-check` (pending for plan 11).
- Retiring the pad probe.
- Any change to `src/game/`, `src/engine/`, the PC binaries or the e2e.

## Further Notes

- **Why timing does not matter.** The PC replays exactly the frames the PSX logged, whatever their vsync count. Only two things depend on the emulator's pace: the difficulty the script lands on, and the ordered states. Both hold as long as each script state spans at least one frame, which 20-vsync holds give with a wide margin. A failure there names the missing state, and the fix is in the script's offsets.
- **Why a change-only log.** Per-frame lines would multiply the TTY output by the frame count for no extra information. Frame numbers on every line, a mandatory frame 0 and the done line's count make any loss detectable.
- **Why reuse the replays' end protocol.** It keeps `psx_state_compare.sh`, the done check and the stopped-state path unchanged. The `game` name in the done line tells the game's run apart in every report.
- **A recording for humans.** `game_events.json`, left in the check's output directory, is an ordinary pad recording. `make replay EVENTS=<it>` dumps the PSX session's last frame on PC.
- **Step note contents.**
  - The start gate's result.
  - Any H-4-k fact found to differ from the code, and how the step adapted.
  - The pad map table.
  - The script's final offsets.
  - The game's frames and vsyncs, `PSX_GAME_CEILING` and `PSX_GAME_TIMEOUT`.
  - `game-ram-room` and the ELF size.
  - `make psx-check`'s conditions count, run time and boot count.
  - Anything the game EXE showed on screen that step 4's captures did not cover, recorded, not judged.
