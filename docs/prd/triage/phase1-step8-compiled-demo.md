---
title: "Compiled demo"
description: "The core cannot read pico8/demo.json at run time (the PSX has no file system), so the demo must be compiled in as C data the core can use, refused at build time when the JSON breaks the caption rules."
status: needs-triage
---

## Problem Statement

Plan 18, phase 1, step 8: **Compiled demo.**

> 8. **Compiled demo.** `src/cmd/demo_generate.c` turns `pico8/demo.json` into `tmp/demo/demo.c` (the record's bytes, one button mask per frame, the captions), following `bindings_generate`'s pattern, with a Makefile rule that rebuilds it when the JSON changes. It refuses what step 6 refuses (a hand-edited JSON) with `file: message`. Verified by a unit test that the compiled data equals the JSON read by the replay reader, a no-change `make raylib` recompiling nothing, and `make test` not slower.

After step 7 the demo exists as `pico8/demo.json`: the record as `initial_save`, the frames, and the captions as `{from, to, text}`. The attract mode (steps 10 and 11) has to play it from the core. Only a file reader can use the JSON:

- **The PSX cannot open it** (section 2, "One mechanism on every platform"; "Rejected: loading `demo.json` at runtime"). The core has no file reader and must not gain one.
- **No C shape exists** for the demo. Step 10 needs the record's bytes for `cart_resume_game`, one pad mask per frame to feed in place of the player's frames, and step 11 needs the captions with their frame ranges. Nothing declares them, and the core cannot include `src/devtools/`.
- **A hand-edited JSON would reach the screen unchecked.** Step 6's rules (32 characters per line, 2 lines, no `v`, a glyph for every character, 150 frames shown, no press in the first 90 frames) are checked only when `pad_author` writes the file. A caption fixed by hand in `demo.json` would be compiled in as typed and drawn by the core.

## Solution

1. **A core header** declares the compiled demo as C data: the record's bytes, one 16-bit button mask per frame, and the captions as `{from, to, text}`, with the frame limit. It sits in `src/game/` so step 10's attract module can include it; it holds no function and no pointer.
2. **A host generator, `demo_generate`**, reads `pico8/demo.json` with the replay reader and step 6's caption module, refuses anything `pad_author` would not have written (`<file>: <message>`, exit 1), and writes `tmp/demo/demo.c`, a C unit that defines that data. It writes through a temporary file and a rename, like `bindings_generate`.
3. **A Makefile rule** rebuilds `tmp/demo/demo.c` when `pico8/demo.json` or the generator changes, and nothing else. The generated unit is never committed (plan 17's pattern: no sync check needed). The interactive `game` links it from this step, so the no-change rebuild property is tested on the real binary; the core reads it from step 10.
4. **Tests in `make test`:** the generator's refusals on small hand-written JSON files, the captions reader's round trip, and the compiled data compared with `pico8/demo.json` as the replay reader reads it.

## User Stories

1. As the step-10 implementer, I want the record's bytes compiled in, so that the attract start calls `cart_resume_game` on them with no file read.
2. As the step-10 implementer, I want one pad mask per frame in a plain array, with the frame count, so that I feed `demo[frame]` in place of the player's frame without decoding anything.
3. As the step-10 implementer, I want frame 0 of the compiled frames to be the first frame after the resume, so that the contract of step 5 holds in the core as in `replay`.
4. As the step-11 implementer, I want each caption as `{from, to, text}` with `from` inclusive, `to` exclusive and lines joined by `\n`, so that I find the caption of a frame and split its lines with no parsing.
5. As the step-11 implementer, I want the caption text size fixed in the core header, so that I size nothing myself.
6. As a PSX porter, I want the demo as `const` C arrays with fixed-width types and no pointer to the heap, so that it lands in read-only data on the console with no loader.
7. As a maintainer, I want the generated unit rebuilt when `pico8/demo.json` changes, so that a regenerated demo is what the binaries play.
8. As a maintainer, I want nothing rebuilt when nothing changed, so that `make test`, `make e2e` and `make raylib` stay as fast as before.
9. As a maintainer, I want the generator to refuse a caption line over 32 characters, so that a hand edit cannot overflow the band.
10. As a maintainer, I want the generator to refuse more than 2 caption lines, so that the band's height holds.
11. As a maintainer, I want the generator to refuse the letter `v`, so that the demo never reads "discouer".
12. As a maintainer, I want the generator to refuse a character the font has no glyph for, so that no caption draws a blank.
13. As a maintainer, I want the generator to refuse a caption shown fewer than 150 frames, so that every caption stays readable.
14. As a maintainer, I want the generator to refuse a button press in a caption's first 90 frames, so that a frame edited by hand cannot rush a caption.
15. As a maintainer, I want the generator to refuse a captions member that is not byte for byte what `pad_author` writes (spacing, order, escapes), so that any hand edit of the member is caught even when its numbers look valid.
16. As a maintainer, I want the generator to refuse captions out of order, overlapping, empty or past the last frame, so that the core never meets a range it cannot handle.
17. As a maintainer, I want the generator to refuse a file without `"start": "resume"` and an `initial_save`, so that the demo always starts from a record.
18. As a maintainer, I want the generator to refuse a file with a `"bindings"` member, so that the demo always plays on the default bindings (section 2).
19. As a maintainer, I want the generator to refuse a demo with no caption or more than 3600 frames, so that section 2's teaching demo and length cap hold in what is compiled.
20. As a maintainer, I want each refusal as `<file>: <message>` with the frame or caption it concerns, so that I find the bad spot at once.
21. As a maintainer, I want the refusal messages of the caption rules to be step 6's own, so that `pad_author` and the generator say the same thing for the same fault.
22. As a maintainer, I want a refused JSON to leave no generated unit behind and the previous one untouched, so that a failed build never links half a file.
23. As a maintainer, I want a unit test that the compiled data equals `pico8/demo.json` as the replay reader reads it, so that the generator's C output is proven, not trusted.
24. As a maintainer, I want that test to also prove that the compiled record decodes, so that a bad record is caught at `make test`, not at the attract start.
25. As a maintainer, I want the generator to link only host modules, never the engine, so that step 10 can link the demo into the engine without a cycle.
26. As a maintainer, I want the core header to declare no pointer, so that `make pointer-guard` keeps its counts.
27. As a maintainer, I want the new files placed in `make float-grep`'s partition, so that the guard keeps passing.
28. As a reviewer, I want the generated unit readable (record 16 bytes per line, masks 16 per line, captions one per line), so that I can read a compiled demo when I need to.
29. As plan-pilot, I want `make demo-check` to keep passing, so that the rule "run it in every step that touches the demo" passes at this step.

## Implementation Decisions

### The core header

- **Place and name: `src/game/demo_data.h`, module `demo_data`.** In `src/game/` because the core includes it from step 10 and the core cannot include `src/devtools/`. Not `demo.h`: `tests/test_demo.c` already exists (the old headless demo of the runtime), and a module `demo` would have no free test name.
- **No `.c` in `src/game/`.** The only definition is the generated unit, as `bindings_compiled` is defined only by `bindings_generate`'s output.
- **Shape** (decided here; step 10 and 11 consume it):

  ```c
  #define DEMO_DATA_FRAME_LIMIT 3600
  #define DEMO_DATA_CAPTION_TEXT_SIZE 66   /* 2 lines of 32 characters, '\n' between, NUL */

  typedef struct {
      int from;                                  /* shown on from <= frame < to */
      int to;
      char text[DEMO_DATA_CAPTION_TEXT_SIZE];    /* lines joined by '\n' */
  } demo_data_caption_t;

  extern const uint8_t demo_data_record[];       /* a save record, as save_state_decode reads it */
  extern const int demo_data_record_count;
  extern const uint16_t demo_data_pad_buttons[]; /* P8_PAD_BUTTON_* masks; [0] is the first frame after the resume */
  extern const int demo_data_frame_count;
  extern const demo_data_caption_t demo_data_captions[];
  extern const int demo_data_caption_count;
  ```

- **Separate `extern const` arrays and an inline text array, not a struct of pointers.** `make pointer-guard` counts data pointers in struct fields and file-scope variables of `src/game/` with strict equality, and its list says no step may raise a count; `extern` declarations are not counted. Trap to record next to the declarations if anyone folds them into a struct.
- **`uint16_t` masks**: 12 buttons fit. The compile-time check that `P8_PAD_BUTTON_COUNT <= 16` sits in the generator, not in the header: `make button-guard` counts every `P8_PAD_BUTTON_*` word in `src/game/`, `P8_PAD_BUTTON_COUNT` included, outside `cheat.c` and the bindings module. One mask per frame, no run-length coding: about 5.5 KB for 2736 frames, and step 10 reads `demo_data_pad_buttons[i]` directly (the plan says "one button mask per frame").
- **`DEMO_DATA_CAPTION_TEXT_SIZE`** must equal `PAD_CAPTION_TEXT_SIZE`; the generator, which includes both headers, checks it at compile time. The core header cannot include the devtools one.
- **`DEMO_DATA_FRAME_LIMIT` = 3600**, section 2's cap, so step 10 can size or assert against it.

### Reading the captions back: `pad_caption` gains a reader

- Step 6's module writes the `"captions"` member; it now also reads it: **`pad_caption_read_member(json, captions, capacity, error, error_size)`** returns the caption count found in the header (before `"frames"`), 0 when the member is absent, or -1 and the reason. It decodes the only escape the writer emits (`\n`); any other backslash is a refusal.
- The format knowledge stays in one module, beside its writer. The replay reader keeps ignoring the member (step 6).
- **Canonical form check:** after reading, the generator rewrites the captions with `pad_caption_write_member` and requires the bytes to equal the member as it stands in the file. Any spacing, order or escape `pad_author` would not have written is a refusal (`"captions" is not as pad_author writes it`), whatever the reader tolerated.

### The loader: `src/devtools/demo_source.{c,h}`

A host module, so every refusal is tested in-process, without forking the generator:

- **`demo_source_load(source, path, error, error_size)`**: 1 and the demo in `source`, or 0 and `"<path>: <reason>"` in `error`. **`demo_source_close(source)`** frees what the replay reader allocated.
- It reads the file with **`replay_source_open`** (frames, `initial_save`, `start`): the same reader as `replay`, so the generator and `replay` cannot read different demos. It refuses, in this order:
  1. the file does not load (missing, not a pad file, a mouse field: the reader's own failures): `cannot load as a replay`;
  2. no `"start": "resume"` (which implies an `initial_save`): `not a from-record demo`;
  3. a `"bindings"` member, through `replay_source_read_bindings` returning anything but the default status: `has a "bindings" member: the demo plays on the default bindings`;
  4. more than `DEMO_DATA_FRAME_LIMIT` frames;
  5. the captions member unreadable, absent (`no caption`), or not canonical (above);
  6. a caption range with `from < 0`, `to <= from`, `to` past the frame count, or `from` before the previous caption's `to`: `caption <n>: frames <from>-<to> ...`;
  7. a caption text that `pad_caption_parse` refuses or changes (the stored text, its `\n` written back as `/`, must parse to itself): step 6's message, prefixed `caption <n>: `;
  8. step 6's frame rules, by **replaying the caption track over the frames**: frame by frame, at each frame first the shown caption is closed (`pad_caption_track_clear` when no caption starts there, the next `pad_caption_track_show` when one does) and a caption starting there is shown, then `pad_caption_track_frame` gets that frame's mask; at the end, `pad_caption_track_end` with checks on. Messages are step 6's, prefixed `frame <f>: `. This is the order in which `pad_author` runs a `caption` line before the frames it emits, so the generator accepts exactly what `pad_author` writes, gaps (`caption -`) included.
- Ranges (6) are checked before the track (8) because the track asserts frames only move forward: an assert is a trap, not a validator.
- The record is **not decoded** by the loader: `save_state_decode` needs the engine, and the generator must not link it (below). The unit test decodes it instead (Testing Decisions), and `make demo-check` plays it.
- Errors read `<file>: <message>`, the plan's format (no line number: a JSON has none that means anything to a reader of the demo).

### The generator: `src/cmd/demo_generate.c`

- `usage: demo_generate <demo.json> <output.c>`, exit 2 on bad usage, 1 on a refusal or an I/O failure, 0 on success. Structure as `bindings_generate`: load, write `<output>.tmp`, check `ferror` and `fclose`, rename; on any failure remove the temporary file, leaving the previous unit untouched.
- **The generated unit**, header line `/* Generated by demo_generate from <input>. Do not edit. */`, then `#include "game/demo_data.h"` and the six definitions: the record as hex bytes 16 per line, the masks as decimal numbers 16 per line, one caption per line as `{ from, to, "text" }`, then the three counts. Arrays are sized by their initialisers.
- **Escapes in C strings:** `\n` for the line break, and `\\`, `\"` and `\?` for those characters. `\?` matters: `-std=c99` enables trigraphs, so a caption containing `??/` or `??)` would otherwise compile to another character. One escape per character type keeps the rule mechanical.
- **Link closure: host modules only** — the generator, `demo_source`, `pad_caption`, the replay reader and what it needs (`input_json`, `p8/input`, `bindings`, `save_storage_memory`), and the font for `p8_font_has_glyph` (`p8/gfx` with what `test_pad_caption` links beside it). **Never `ENGINE_SRCS`, never `$(BINDINGS_DEFAULT_UNIT)`:** once step 10 links the demo unit into the engine closure, a generator that links the engine would need its own output to link. This is the reason the record is not decoded at generation.

### Makefile

- Next to `BINDINGS_DEFAULT_UNIT`: `DEMO_UNIT := $(TMP_DIR)/demo/demo.c`, a rule building `$(TMP_DIR)/demo_generate` from its objects, and `$(DEMO_UNIT): pico8/demo.json $(TMP_DIR)/demo_generate` running `mkdir -p $(@D)` then the generator. No `FORCE`, no `.source` file: the JSON is the only input, so a no-change build sees the unit up to date.
- **`$(DEMO_UNIT)` joins `GAME_SRCS`** in this step. Nothing in `game` references the symbols yet; they are linked as data. This makes the plan's "no-change `make raylib` recompiling nothing" a real check from this step, and step 10 has nothing to add to `game`.
- **Not added to `ENGINE_SRCS`, `REPLAY_SRCS` or `PAD_AUTHOR_SRCS`** in this step: nothing reads the data yet. Step 10 links it wherever its attract module goes, as `$(BINDINGS_DEFAULT_UNIT)` is appended per target.
- **The `-include` line of the default object tree gains `$(OBJ_DIR)/tmp/demo/*.d`**, as for `tmp/bindings/` and `tmp/keymap/`: without it, a change to `src/game/demo_data.h` would not recompile the generated object. Step 10 adds the other trees' lines if it compiles the unit with other flags.
- Two unit binaries join `UNIT_TESTS` (Testing Decisions). `test_demo_data`'s rule lists `pico8/demo.json` as a run-time prerequisite, as fixtures are listed.
- The `make raylib` legend gains a few words: it also links `pico8/demo.json` compiled into `tmp/demo/demo.c`. The rest of the legend is step 14's.

### Guards

- **`make float-grep`**: every `.c`/`.h` of `src/` is in the core or in `tools/float_grep_exclusions.txt`, never both. Add `src/cmd/demo_generate.c`, `src/devtools/demo_source.c` and `.h` (host tools), and **`src/game/demo_data.h`**, which no core file includes until step 10. Trap for step 10: once its attract module includes the header, the header is in the core and its exclusion line must go, or `float-grep` fails on "both". The justification text says so.
- **`make pointer-guard`**: no change expected (no pointer in `src/game/demo_data.h`, see above).
- **`make button-guard`**: it counts any word starting `P8_PAD_BUTTON_` in `src/game/` (`P8_PAD_BUTTON_COUNT` too), so the header names none; the generated unit lives in `tmp/` (not scanned) and writes masks as numbers.
- `p8-guard`, `data-guard`, `random-guard`, `alloc-guard`: unaffected (no matching name).

### Verifying the no-rebuild property

- **`make test`:** run it twice; the second run compiles nothing (no `-c -o` line, no `demo_generate` line in its output), and `make -q tmp/demo/demo.c tmp/obj/tmp/demo/demo.o tmp/demo_generate tmp/test_demo_data` exits 0.
- **`make raylib`:** try `make raylib-vendor-setup` once. If raylib is vendored: `make raylib` twice, the second prints only its two `bindings:`/`keymap:` lines, and `make -q tmp/game` exits 0. If it cannot be vendored (no network), `make -q tmp/game` cannot answer (the missing `libraylib.a` has no rule), so check every other prerequisite of `tmp/game`: after a build, `make -q tmp/demo/demo.c tmp/bindings/game.c tmp/keymap/game.c` exits 0, and `make -p -q tmp/game` (database print) shows `tmp/demo/demo.c` among `tmp/game`'s prerequisites with no `FORCE` in its chain. The step note says which of the two was done.
- **Timing:** `make test` with nothing to rebuild, three runs, step 1's method, compared with step 1's minimum (8.2 s) and step 6's range. `make e2e` is untouched (no replay binary changes) and all `*.sha256` stay unchanged.

### Guardrails for this step

- No change to `src/game/*.c`, `input_step`, the draw or the save path: the core is not touched, so `make end-state` and `make full-game` are not required. The core gains one header, read by nothing yet.
- Runs: `make test-all`; `make demo-check` (plan-pilot's rule: the step touches the demo's chain); `make pad-author-proof` (the caption module changes) and `make pad-author` with no diff in `worst-case/` or `pico8/demo.json`.
- **The step note records:** the generated unit's size and its three counts (record bytes, frames, captions); the no-rebuild evidence and which variant was used for `make raylib`; the `make test` timings; one by-hand refusal of a hand-edited copy of `pico8/demo.json` in `tmp/` (a caption given a `v`), with the printed message, not committed.

## Implementation Chunks

1. **Captions reader.** `pad_caption_read_member` and its tests in `tests/test_pad_caption.c` (round trip with the writer, absent member, bad escape, malformed entry). Done when `make test` passes and `make pad-author-proof` is unchanged.
2. **Loader.** `src/devtools/demo_source.{c,h}`, `tests/test_demo_source.c` with every refusal, their `float-grep` lines. Done when `make test` passes.
3. **Header, generator, Makefile, compiled-data test.** `src/game/demo_data.h`, `src/cmd/demo_generate.c`, the `DEMO_UNIT` rules, `GAME_SRCS`, the `.d` include, `tests/test_demo_data.c`, the legend words and `float-grep` lines. Done when `make test-all`, `make demo-check` and the no-rebuild checks pass.

## Testing Decisions

- A good test drives a module through its public interface and reads only its results: return values, the error text, the compiled arrays. No test reads the generator's C text.
- **`tests/test_pad_caption.c`** (extended): `pad_caption_read_member` on a member written by `pad_caption_write_member` gives back the same captions (one caption, several, a two-line text); absent member gives 0; a `\t` escape, an unterminated text and a missing `"to"` give -1 with a reason.
- **`tests/test_demo_source.c`** (new): the test writes its fixtures to `tmp/test_demo_source_work/` with `p8_input_save` and a header built from `pad_caption_write_member` (so the accepted fixture has `pad_author`'s exact layout), then makes each bad variant by one string edit or one changed frame. Cases:
  - accepted: one caption over 150 frames; two captions tiling; two captions with a gap (`caption -`); a press at frame 90 of a caption; the loaded frames, record bytes and captions are what the fixture holds;
  - refused, each checking the message's `<path>: ` prefix and step 6's wording: a line of 33 characters, 3 lines, a `v`, a character with no glyph, a caption shown 149 frames, the last caption cut short, a press at frame 89 of a caption, overlapping captions, a caption past the last frame, an empty range, a non-canonical member (an extra space), a text with a `/`, no `"start"`, a `"bindings"` member, 3601 frames, no caption, a missing file, a mouse field.
  - Fixtures stay small (a few hundred frames), except the 3601-frame case.
- **`tests/test_demo_data.c`** (new): links `$(DEMO_UNIT)` (built from the committed `pico8/demo.json`), the replay reader and the engine closure with the default bindings unit (as `tests/test_record_start.c` links them). It opens `pico8/demo.json` with `replay_source_open` and checks: frame count and every mask equal; `initial_save` count and every byte equal; `save_state_decode` accepts the compiled record; the compiled captions, put in a track and written with `pad_caption_write_member`, give a member found byte for byte in the file (the replay reader ignores captions, so the writer is the independent witness); every caption text fits `DEMO_DATA_CAPTION_TEXT_SIZE`.
- Both new tests go in `UNIT_TESTS` (`make test`): they run in-process in milliseconds, and `make test`'s no-change timing is measured.
- **Prior art:** `tests/test_bindings.c` (a generated unit linked into its test), `tests/test_pad_caption.c` (caption rules and messages), `tests/test_replay_source.c` (the reader on fixtures), `tests/test_record_start.c` (engine link with `pico8/demo_record.hex`).

## Out of Scope

- Linking the demo unit into the engine closure, `replay`, `pad_author` or any core module; the attract state machine (step 10); drawing captions and the label (step 11); the default table beside the compiled one (step 9); the attract e2e (step 12); extending `make demo-check` (step 13); docs beyond the legend's few words (step 14).
- Any change to `pad_author`'s output, step 6's rules or messages, the replay reader's handling of captions.
- Compressing the frames (run-length or other), a PSX build of the unit.
- Decoding the record inside the generator (it would need the engine: see the link-closure decision).
- Changing `pico8/demo.json` or `pico8/demo.pad` (step 7).

## Further Notes

- Every number of the compiled demo comes from step 7's file. If step 7's file differs from the hypotheses below, the generator and tests need no change unless a hypothesis marked "if not" says otherwise: they read the file, they do not pin its content.
- `pad_author` writes the record as lowercase hex and `replay_source` reads it; the generator writes `0x..` bytes, so letter case never matters downstream.
- The caption track in step 6 sizes its storage from the frame budget (`PAD_AUTHOR_CAPTIONS_MAX`); the loader sizes its own from `DEMO_DATA_FRAME_LIMIT / PAD_CAPTION_SHOWN_MIN + 1` (25), which also bounds what `pad_caption_read_member` may return.

### Hypotheses

Step 7 is being implemented while this PRD is written; its PRD is `docs/prd/triage/phase1-step7-demo-script.md`. What this PRD assumes about its outcome. If step 7's note says otherwise, the note wins and only the named part of this PRD follows it.

- **H-7-1 (file and layout):** `pico8/demo.json` exists, is committed, and is `pad_author`'s output unchanged: `{ "input": "pad", "start": "resume", "initial_save": "<hex>", "captions": [ … ], "frames": [ … ] }` in that order, as `p8_input_save` and `pad_author`'s header writer produce it; one caption per line, one frame per line. If the captions member sits elsewhere (after `"frames"`), `pad_caption_read_member` reads it where `pad_author` writes it, and the canonical check compares there.
- **H-7-2 (members absent):** the file has no `"bindings"` and no `"attract_after"` member. If it has `"attract_after"`, the loader ignores it (it means nothing for the demo); a `"bindings"` member stays a refusal, and that would be a step-7 defect.
- **H-7-3 (frames):** about 2736 frames (step 7's decided idles), at most 3600, pad masks only. If the count is different but at most 3600, nothing changes; above 3600, the generator refuses and that is step 7's stop rule, not this step's to relax.
- **H-7-4 (captions):** seven captions in section 2's order and text, written by `pad_caption_write_member`, tiling the demo from frame 0 (or 1, under step 7's H-6-5 fallback) to the last frame, the closing one 180 frames. The generator does not depend on the count, the tiling or the start frame: it accepts any captions `pad_author` can write, gaps included.
- **H-7-5 (record):** `initial_save` holds the bytes of `pico8/demo_record.hex` (3852 bytes, step 2's note). The compiled-data test compares with the JSON, not with the hex file, so a re-cut record changes nothing here.
- **H-7-6 (regeneration):** `make pad-author` regenerates `pico8/demo.json` (with `--no-aim-budget`), so an edit of `pico8/demo.pad` followed by `make pad-author` then `make` rebuilds the unit through the JSON's date alone.
- **H-7-7 (`make demo-check`):** the target exists, outside `make test-all`, builds `tmp/pad_author`, `tmp/replay` and its own checker test, regenerates `pico8/demo.json` into `tmp/demo-check/` and compares it with `cmp`, and plays the committed file with `REPLAY_STARTS=2`. This step neither extends nor depends on its internals; it only runs it. If it is missing, the step note says so and the plan-pilot rule is reported, not worked around.
- **H-7-8 (names):** step 7 adds `tests/test_demo_check.c` and `tools/demo_check.sh`, and no `src/game/demo*`, `src/devtools/demo_source*`, `tests/test_demo_source.c` or `tests/test_demo_data.c`; this step's names do not collide with it. If one does, this step renames its own file.

### Decisions taken without a human (guideline rules)

No human answers questions in this run. Each decision below was taken under `.claude/skills/guideline/SKILL.md` ("make the plan work", "keep it safe", "the smaller change"):

- **Header `src/game/demo_data.h`, data as six `extern const` symbols, caption text inline:** the core can include it; `make pointer-guard` keeps its counts (no step may raise one); the name avoids the existing `test_demo.c`.
- **A loader module `demo_source` in `src/devtools/`, the generator a thin `main`:** every refusal tested in-process within `make test`'s budget, as `bindings_parse` lets `bindings_generate` stay thin.
- **Reuse `pad_caption`'s parse and track to check the JSON, and add a reader beside its writer:** the plan says the generator refuses what step 6 refuses; reusing the same functions gives the same rules and the same messages, and the format stays in one module.
- **Canonical member check by rewriting with `pad_caption_write_member`:** catches any hand edit of the member for free, with no second notion of what is valid.
- **Also refuse no `"start": "resume"`, a `"bindings"` member, more than 3600 frames and no caption:** each is a section 2 rule the core relies on (record start, default bindings, length cap, a teaching demo), each one line; and no zero-length C array is ever written.
- **Generator links no engine; the record's decoding is checked by the unit test:** avoids a link cycle at step 10.
- **`game` links the unit from this step; the engine, `replay` and `pad_author` do not:** makes the plan's no-rebuild check real now; the rest is step 10's.
- **Masks as `uint16_t`, one per frame, no compression:** the plan's wording, a few KB, direct indexing.
- **Escape `\?` in generated strings:** trigraphs are on under `-std=c99`.
- **No-rebuild check without raylib:** `make -q` on every other prerequisite of `tmp/game` plus the database print, after one attempt to vendor raylib.
- **Both new tests in `make test`:** in-process, milliseconds; the no-change timing is measured against step 1's baseline.
