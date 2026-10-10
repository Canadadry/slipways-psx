---
title: "`caption` command"
description: "pad_author has no way to say what the demo shows on screen and when, so the captions would have to be timed by hand and would drift whenever the demo is regenerated."
status: needs-triage
---

## Problem Statement

Plan 18, phase 1, step 6: **`caption` command.**

> 6. **`caption` command.** `pad_author` accepts `caption <text>` (`/` breaks the line, `caption -` clears) and writes `"captions": [{"from": F, "to": T, "text": "…"}]` into its JSON; the replay reader accepts and ignores the member. It refuses, with the generator's usual `file:line` message and a `.refused.pad` proof each: a line over 32 characters, more than 2 lines, the letter `v`, a character the font has no glyph for, a caption shown fewer than 150 frames, or a button press fewer than 90 frames after a caption appears. Verified by the proofs, `make pad-author-proof`, and `make pad-author` with no diff in `worst-case/`.

The attract mode teaches the game with seven captions (section 2's table) laid over a recorded pad stream. Today the demo's author, an agent writing `pico8/demo.pad` in step 7, has no way to attach a caption to a moment of the script. The frame numbers could be typed by hand into the JSON, but every regeneration after a gameplay change moves the beats (a `fold` that pans one frame longer shifts everything after it), and hand-typed ranges would silently drift off the action they describe.

Nothing checks the captions either:

- **Text.** The font of `src/p8/gfx.c` draws `v` with the `u` glyph (section 2's trap), draws any character it does not know as a solid placeholder block, and has no line break: `p8_print` draws a `\n` as a placeholder. A 33-character line runs off the 128-pixel screen (4 pixels per character).
- **Pacing.** Section 2 requires each caption to stay at least 150 frames, and to appear at least 90 frames before the next button press, so a viewer reads it before the action it describes. A script that breaks this looks fine to its author and only shows up when a human watches the demo, which is the last step of the plan.

## Solution

1. **A `caption` script command.** `caption <text>` shows `<text>` from the next frame `pad_author` emits. A `/` in the text breaks the line. `caption -` clears it. A new `caption` replaces the one shown. A caption still shown when the script ends lasts until the output's last frame, so the closing card needs no `caption -`.
2. **`pad_author` computes the frame ranges itself** and writes them into its output as a header member, `"captions"`, a list of `{"from": F, "to": T, "text": "…"}` in frame order. `F` and `T` are indexes into the same output's `"frames"` array: the caption is shown on frames `F` to `T - 1`. A regenerated demo keeps its captions on the beats they describe, because the ranges come from where the `caption` lines sit in the script, not from typed numbers.
3. **`pad_author` refuses a caption the attract mode cannot show well**, with its usual `file:line: frame N: reason` message:
   - a line longer than 32 characters;
   - more than 2 lines;
   - the letter `v` (or `V`);
   - a character the font has no glyph for;
   - a caption shown fewer than 150 frames;
   - a button down fewer than 90 frames after a caption appears.

   Each refusal has a `.refused.pad` proof in `tools/pad_author_proofs/`, run by `make pad-author-proof`.
4. **The rules live in one small module**, `pad_caption`, with no cart in it: text parsing and checking, the frame bookkeeping, and the JSON member. `pad_author` drives it. Step 8's generator, which must refuse "what step 6 refuses" in a hand-edited JSON, can link the same module instead of copying the rules.
5. **The replay reader accepts and ignores `"captions"`.** It already looks up only the members it knows. A unit test pins that a file with the member loads the same frames and header values as without it, and `pad_author`'s own self-check replays every captioned output.
6. **No existing output changes.** A script without `caption` writes no `"captions"` member, so `make pad-author` gives no diff in `worst-case/` and `make pad-author-proof` none in `tests/e2e/`.

## User Stories

1. As the step-7 author, I want to write `caption probe the dark/to find new planets` just before the beat it describes, so that the caption follows the beat when the demo is regenerated.
2. As the step-7 author, I want `pad_author` to compute each caption's first and last frame, so that I never type a frame number.
3. As the step-7 author, I want a `/` to break the caption into two lines, so that a two-line caption fits on one script line.
4. As the step-7 author, I want spaces around the `/` to be ignored, so that `build a trading empire / across the stars` and `build a trading empire/across the stars` give the same caption.
5. As the step-7 author, I want `caption -` to clear the caption, so that a stretch of the demo can run without text.
6. As the step-7 author, I want a new `caption` to replace the shown one directly, so that consecutive beats need no `caption -` between them.
7. As the step-7 author, I want a caption still shown when the script ends to last until the last frame, so that the closing card needs no extra command.
8. As the step-7 author, I want a caption of any number of words, so that `colonize planets:/each one needs and makes goods` is not refused by the script's 8-word limit.
9. As the step-7 author, I want a line over 32 characters refused, so that no caption runs off the 128-pixel screen.
10. As the step-7 author, I want more than 2 lines refused, so that the caption band stays the size step 11 will give it.
11. As the step-7 author, I want the letter `v` refused, so that "discover" never reads "discouer" on screen.
12. As the step-7 author, I want a character with no glyph refused, so that no caption shows a solid block where a letter should be.
13. As the step-7 author, I want a caption shown fewer than 150 frames refused, so that every caption stays long enough to read.
14. As the step-7 author, I want a button down within 90 frames of a caption's appearance refused, so that the viewer reads the caption before the action it explains.
15. As the step-7 author, I want each refusal to name the file, line and frame, so that I fix the right line at once.
16. As the step-7 author, I want a too-short caption reported at the `caption` line that opened it, so that I know which caption to lengthen, not which command came after it.
17. As the step-7 author, I want a too-early press reported at the line of the command that pressed, so that I know which beat needs a longer idle before it.
18. As the step-7 author, I want `--try` lines to accept `caption` and apply the same checks, so that I can try a beat with its caption before writing it into the script.
19. As the step-7 author, I want `--stop-at` to work on a captioned script without refusing the caption cut by the stop label, so that I can inspect the state halfway through the demo.
20. As the step-7 author, I want captions to work in a script started by `from-record` (step 5) and in a title-based script alike, so that one command serves the demo and the proofs.
21. As the step-7 author, I want captions to work across `include`, so that a captioned script may be split.
22. As the step-8 implementer, I want the JSON layout of `"captions"` fixed (one caption per line, `\n` the only escape), so that my generator parses it without a JSON library.
23. As the step-8 implementer, I want the text rules and pacing rules in one module I can link, so that the generator refuses a hand-edited JSON with exactly the rules `pad_author` applies.
24. As the step-10 and step-11 implementers, I want the frame range defined against the output's frame indexes, so that the attract module shows caption `k` exactly on the frames `pad_author` meant.
25. As the step-11 implementer, I want the line break stored as `\n` in the text, so that I split the caption into lines and print each with `p8_print`, which draws no line breaks itself.
26. As a maintainer, I want every script without `caption` to produce byte-identical output, so that `worst-case/` and every e2e file stay unchanged.
27. As a maintainer, I want `replay` to load a file with `"captions"` exactly like the same file without it, so that the member can never change a replay.
28. As a maintainer, I want each refusal the plan names to have a `.refused.pad` proof, so that `make pad-author-proof` keeps every rule working.
29. As a maintainer, I want the rules unit-tested in `make test` in a few milliseconds, so that `make test` does not get slower.
30. As a maintainer, I want the font's glyph set read from the font itself, so that a glyph added to `src/p8/gfx.c` later is accepted without touching the caption rules.
31. As a reviewer, I want `pico8/demo.json` (step 7) to carry its captions at the top of the file, with their text readable, so that I review the demo's words without scrolling past thousands of frames.
32. As a player (later), I want every caption on screen long enough to read and before the action it explains, so that the demo teaches instead of flashing text.

## Implementation Decisions

### The command

- **Syntax.** `caption <text>` or `caption -`. The text is the rest of the line after `caption` and its following whitespace, taken raw, not split into words: the script tokenizer stops after the first word when that word is `caption`, and keeps the rest as the single argument. This lifts the 8-word limit (`TOKENS_MAX`) for this command only, and keeps runs of inner spaces as written.
  - `#` still starts a comment and `;` still separates `--try` commands, as on every line. Neither has a glyph, so no caption loses text to them.
  - `caption` with no text fails with `caption: needs a text or -`.
  - `caption -` with no caption shown fails with `caption -: no caption shown`. A stray `caption -` is a script mistake, and refusing it costs one line.
- **Lines.** The text is split at each `/`. Each line is trimmed of leading and trailing spaces. An empty line (`caption a//b`, `caption a/`, `caption /b`) fails with `caption: line <n> is empty`. A literal `/` can therefore never appear in a caption: accepted, none of section 2's captions has one.
- **Text checks, on the `caption` line** (the frame in the message is the current frame count):
  - `caption: line <n> has <k> characters, at most 32`;
  - `caption: <k> lines, at most 2`;
  - `caption: the letter v draws as u`, for `v` and `V`;
  - `caption: no font glyph for '<c>'` for a printable byte, `caption: no font glyph for byte 0x<hh>` for any other (tab, UTF-8, control bytes).

  Checked in that order, the first failure wins. Uppercase letters are accepted: the font draws them with the lowercase glyphs, as section 2's captions are lowercase anyway.
- **The glyph set comes from the font.** `src/p8/gfx.c` gains `int p8_font_has_glyph(char c)`: 1 when `font_glyph_rows` returns a glyph other than the placeholder. Today that is space, `0`–`9`, `a`–`z` and `A`–`Z`, and `! $ % ' ( ) + , - . / : = ?`. The `v` rule is separate, because the font does have a glyph for `v` (the wrong one). Fixing that glyph is out of scope (plan section 2).

### The frame ranges

- **Indexes.** `from` and `to` are 0-based indexes into the output's `"frames"` array. `from` is inclusive, `to` exclusive. A caption is shown on frame `i` exactly when `from <= i < to`, and it is shown for `to - from` frames.
  - Frame 0 is the output's first frame. For a record-based output that is the first frame after the resume (H-5-1). For a title-based output it is the first frame `pad_author` emits after boot.
  - "Shown on frame `i`" means drawn by the `cart_draw` of the loop turn that consumes frame `i`. That is the contract steps 10 and 11 implement.
- **How `pad_author` computes them.** `pad_author` counts the frames it has emitted (`frame_count`).
  - `caption <text>` run when `frame_count` is `N` closes the shown caption, if any, with `to = N`, then opens the new one with `from = N`.
  - `caption -` run at `N` closes the shown caption with `to = N`.
  - The end of a run closes the shown caption with `to = frame_count`, the output's frame count.
  - A caption opened and closed at the same `N` would be shown 0 frames, and fails the 150-frame rule like any other short caption.
- **Order.** Captions are written in the order they were opened. Ranges never overlap and `from < to` always holds. Gaps are allowed (after `caption -`).
- **Ends of a run.**
  - `pad_author_run`, whole run: the shown caption is closed at the last frame and checked against the 150-frame rule.
  - `pad_author_run` stopped by `--stop-at`: the shown caption is closed at the stop frame **without** the 150-frame check. The output is a prefix, and a caption cut by the stop label was not shown short, it was cut. Every caption closed before the stop is checked as usual.
  - `pad_author_run_line` (`--try`): its run starts with the captions of the main run and closes the shown caption at its end, with the check.
  - A run that fails for any reason closes nothing, and its `.failed.json` carries no `"captions"` member. It is a debugging file.

### The pacing rules

- **Shown long enough.** When a caption closes (by `caption`, `caption -` or the end of a whole run), `to - from` must be at least 150. Otherwise the run fails with `caption: shown <n> frames, at least 150`. The location is the file and line of the `caption` command that **opened** it, and the frame is its `from`: that is the line the author must move.
  - `pad_author` keeps a copy of the opening line's file path and line number with each open caption, since an `include`d file's path buffer does not outlive its `include`.
- **Read before the action.** A *button press* here is any output frame whose mask is non-zero. While a caption is shown, `pad_author` refuses to emit a non-zero frame at index `i` with `i - from < 90`. The message is `caption: a button down <i - from> frame(s) after the caption appeared, at least 90`, at the line of the command that emitted it, with the usual frame.
  - So frames `from` to `from + 89` of every caption are all empty.
  - This is slightly stricter than "the first frame whose mask goes from 0 to non-zero". It also refuses a button still held from before the caption, since a held button acts on screen while the viewer reads (a `year` hold, say). Every high-level command ends with its buttons up, so the only scripts this refuses are ones where a `caption` line sits inside a low-level `hold`. Decided under the guideline rule "make the plan work": the stricter reading serves section 2's goal, that the viewer reads before the action.
  - Only the shown caption needs checking. A closed caption was shown at least 150 frames (else the run already failed), so its whole 90-frame window was checked while it was shown.
- **Where the press check runs.** In `emit`, right after the frame-budget check and before the chord and empty-cone checks. Every frame `pad_author` produces goes through `emit`, including the frames of `wait_until` and of every high-level command.
- **Not checked here:** the closing card's 180 frames and the demo's 3600-frame cap (section 2). They are properties of `pico8/demo.pad`, which step 7 writes and checks. The tool checks only the six rules step 6 lists.

### The `pad_caption` module (new, devtools)

A pure module, with no cart and no `pad_author` in it. It links only `p8_font_has_glyph`. It holds all the rules above, and `pad_author` calls it. The shape below is the decision. The exact names may follow the codebase's conventions.

```c
#define PAD_CAPTION_LINE_CHARS 32
#define PAD_CAPTION_LINES_MAX 2
#define PAD_CAPTION_SHOWN_MIN 150
#define PAD_CAPTION_QUIET_MIN 90
#define PAD_CAPTION_TEXT_SIZE (PAD_CAPTION_LINES_MAX * (PAD_CAPTION_LINE_CHARS + 1))

typedef struct {
    int from, to;                     /* frame indexes, from inclusive, to exclusive */
    char text[PAD_CAPTION_TEXT_SIZE]; /* lines joined by '\n', no trailing '\n' */
} pad_caption_t;

/* 1 and the checked text in `out`, or 0 and the reason (no location) in `error`. */
int pad_caption_parse(const char *script_text, char *out, size_t out_size, char *error, size_t error_size);

typedef struct {
    pad_caption_t *captions; int capacity; int count;
    int shown;                        /* 1 while captions[count - 1] has no `to` yet */
} pad_caption_track_t;

void pad_caption_track_init(pad_caption_track_t *track, pad_caption_t *storage, int capacity);
int pad_caption_track_show(pad_caption_track_t *track, int frame, const char *text, char *error, size_t error_size);
int pad_caption_track_clear(pad_caption_track_t *track, int frame, char *error, size_t error_size);
int pad_caption_track_frame(const pad_caption_track_t *track, int frame, int buttons, char *error, size_t error_size);
int pad_caption_track_end(pad_caption_track_t *track, int frame, int checked, char *error, size_t error_size);
int pad_caption_write_member(const pad_caption_track_t *track, char *out, size_t out_size);
```

- `show`, `clear` and `end` close the shown caption and fail on the 150-frame rule. `pad_author` then formats the location from the opening line it kept.
- `frame` is the 90-frame check for one frame about to be emitted.
- The text in `show` is already parsed. `pad_caption_parse` is a separate call, so step 8 can check a JSON caption's text with it.
- A whole output `(captions, frames)` is checked by replaying its frames through `frame` and its closings through `end`. Step 8 can do the same, so the rules exist once.
- **Capacity.** `pad_author_t` holds `PAD_AUTHOR_FRAME_BUDGET / PAD_CAPTION_SHOWN_MIN + 1` captions (49). Every closed caption takes at least 150 of the 7200 budgeted frames, and at most one more is cut by a stop label, so the array cannot overflow. `show` asserts it with a message rather than adding a refusal no script can reach.

### The JSON member

- **Place.** `"captions"` is a header member, written after `"start"` and `"initial_save"` when they exist (H-5-3) and before `"frames"`, through `p8_input_save`'s header-members argument (H-5-2). The plan lists `initial_save`, `frames`, `captions` as the content of `demo.json`, not as an order. In the header, the captions are reviewable at the top of the file, and step 8 reads them where the reader already looks up members.
- **Exact layout,** fixed so step 8 can parse it line by line:

  ```
    "captions": [
      { "from": 0, "to": 150, "text": "build a trading empire\nacross the stars" },
      { "from": 150, "to": 330, "text": "probe the dark\nto find new planets" }
    ],
  ```

  One caption per line, keys in this order, single spaces as shown, and `\n` (backslash, `n`) the only escape. No other escape is ever needed: `"` and `\` have no glyph, so the text checks refuse them.
- **Absent when empty.** A run with no caption writes no `"captions"` member. Its output is byte-identical to today's, or to step 5's for a record-based script.
- `pad_caption_write_member` writes the member as bare JSON (starting with `"captions"`, no indent before it, no trailing comma), as `p8_input_save`'s header members require (H-5-2). `src/cmd/pad_author.c` joins it to step 5's members.

### The replay reader

- **No parsing change.** `replay_source` looks up only `"input"`, `"initial_save"`, `"attract_after"`, `"bindings"` and (H-5-4) `"start"` before `"frames"`, by exact quoted key. A caption's text cannot contain `"`, so no caption can fake a key, and `"from"`, `"to"` and `"text"` match none of them. `p8_input_load` reads only the `"frames"` array.
- "Accepts and ignores" is pinned by tests, not by code. If a test shows the reader rejects the member, the fix goes in the reader, smallest change first.

### Other changes

- **Makefile:**
  - `pad_caption.c` joins `PAD_AUTHOR_SRCS`;
  - a new `test_pad_caption` binary joins `make test`;
  - the `make pad-author` legend gains the caption refusals, in its list of refusals;
  - `make pad-author-proof` needs no new rule: the `.refused.pad` kind already covers the new proofs.
- **Proofs**, in `tools/pad_author_proofs/`, each starting with `# expect: <message>` and `title 12 reasonable` like the existing refusals. None depends on step 5.
  - `caption_line_long.refused.pad`: a 33-character line;
  - `caption_lines.refused.pad`: three lines;
  - `caption_letter_v.refused.pad`: `caption discover new worlds`;
  - `caption_glyph.refused.pad`: a `&`;
  - `caption_short.refused.pad`: `caption x`, `idle 149`, `caption -`;
  - `caption_press.refused.pad`: `caption x`, `idle 89`, then a high-level command whose first frame presses (e.g. `probe 256 250`).
- **The step-5 record proof gets its captions.** `tools/pad_author_proofs/demo_beats.record.pad` (H-5-5) gains section 2's seven captions, each placed just before its beat's leading `idle 90`, and the opening caption at frame 0. Its gaps already follow the pacing sizes. Its existing checks then prove, with captions, that:
  - `pad_author` accepts a real captioned demo from the record;
  - its self-check `replay` reads the member and replays to the same state;
  - the two-start replay is unaffected.

  If the captions break a pacing rule there, the implementer lengthens that proof's idles, never the rules.
- **Guardrail runs.** This step changes `pad_author` and `src/p8/gfx.c`, but not `src/game/`, `input_step`, the draw path or `replay`. It runs `make test-all`, `make pad-author` (no diff in `worst-case/`) and `make pad-author-proof` (no diff in `tests/e2e/`). `make end-state` and `make full-game` are not required by the plan for this step. They run anyway, since `src/p8/gfx.c` is in `replay` (cheap, outside `make test-all`).
- **The step note records:**
  - the frame-range contract;
  - the stricter press reading;
  - the captioned record proof's `"captions"` member as written (seven `from`/`to` pairs);
  - the `make test` timing against step 1's minimum (8.2 s).

## Implementation Chunks

1. **Rules module.** `p8_font_has_glyph` in `src/p8/gfx.c` with its case in `test_gfx`. The `pad_caption` module, and `tests/test_pad_caption.c` in `make test`. Nothing uses it yet. Every `.sha256` is unchanged.
2. **`pad_author` wiring and output.**
   - The tokenizer's raw argument for `caption`, the command, the press check in `emit`, and the closings at both run ends with the stop-label exemption.
   - The `"captions"` member through `p8_input_save`'s header members.
   - The reader tests, the six refusal proofs, the captioned record proof and the Makefile legend.
   - `make pad-author` and `make pad-author-proof` with no diff.

## Testing Decisions

- A good test drives a module through its public interface and reads only what that interface returns, or the files a binary writes. It never reads statics or depends on the order of internal calls.
- **`test_gfx`** (existing): `p8_font_has_glyph` is 1 for `a`, `A`, `0`, space, `:` and `?`, and 0 for `&`, `"`, `\`, `#`, `\t`, `\n` and a byte above 127. It is 1 for `v`: the glyph exists, it is wrong, and that is the module's separate rule.
- **`test_pad_caption`** (new, `make test`, pure, well under a millisecond):
  1. **Parse.**
     - `a/b` and `a / b` both give `a\nb`.
     - A 32-character line is accepted and a 33-character one refused with its line number.
     - Two lines are accepted, three refused.
     - `v` and `V` are refused; `&`, `"`, a tab and a UTF-8 byte are refused with the right message form.
     - An empty line in each position is refused.
     - Uppercase is accepted.
  2. **Ranges.**
     - `show` at 0, then `show` at 150, then `end` at 300 give `{0,150}` and `{150,300}`.
     - `clear` leaves a gap: `show 0`, `clear 150`, `show 200`, `end 360` give `{0,150}` and `{200,360}`.
  3. **150 rule.**
     - A caption closed at 149 frames by `show`, `clear` or `end` fails, and at exactly 150 passes.
     - `end` with `checked = 0` accepts a short caption and closes it.
     - `clear` with nothing shown fails.
  4. **90 rule.**
     - With a caption shown from `F`, `frame` with buttons at `F + 89` fails, and at `F + 90` passes.
     - Empty frames pass at any index.
     - With no caption shown, any frame passes.
  5. **Member.** The exact bytes of `pad_caption_write_member` for two captions, one of them with a line break. Nothing is written for zero captions.
- **`test_replay_source`** (existing): a fixture written by the test to `tmp/` with a `"captions"` member, two captions, one of them with `\n`, placed between `"initial_save"` and `"frames"`.
  - It loads, with the same frames and the same `initial_save`, `attract_after` (and `start`, H-5-4) as the same file without the member.
  - The same holds for a member after `"frames"`, so the reader does not care where a hand edit puts it.
- **`test_input`** (existing): `p8_input_load` loads the same frames from that fixture.
- **`make pad-author-proof`** (outside `make test-all`): the six refusal proofs each fail with their `# expect:` message, the captioned record proof passes (H-5-5), and every existing frames and end-state proof stays byte-identical.
- **`make pad-author`**: no diff in `worst-case/`, since no worst-case script has a caption.
- **Prior art:**
  - `tests/test_pad_metrics.c` (a pure devtools module tested through its interface);
  - `tests/test_replay_source.c` (the `attract_after` cases of step 3, fixtures written to `tmp/`);
  - `tests/test_gfx.c`;
  - `tools/pad_author_proofs/*.refused.pad` (`# expect:` refusals).
- **Timing:** `make test` with nothing to rebuild, three runs, minimum, not above step 1's 8.2 s beyond noise. `make e2e` is not touched.

## Out of Scope

- **Step 7's work:** the demo script `pico8/demo.pad`, `pico8/demo.json`, `pico8/demo.txt` and the section-2 caption texts as content. Also the closing card's 180 frames, the 3600-frame cap, and `make demo-check`.
- **Later steps:** the compiled demo and its generator's refusals (step 8, which may link `pad_caption`), and drawing the captions and the `demo` label (step 11, which splits the text at `\n`).
- **Fixing the `v` glyph** in the font (plan section 2).
- **Any change to `src/game/`, `replay`, `replay_source`'s parsing, `input_json`'s writer** (beyond what step 5 delivers), and the session recorder.
- **A literal `/` in a caption**, and any escape other than `\n` in the JSON.
- **A refusal for uppercase letters.** The font draws them, and the lowercase style is step 7's content choice.

## Further Notes

- Section 2's seven captions all pass the text rules. The longest line is `each one needs and makes goods`, 30 characters. No `v`, every character has a glyph, and `colonize planets:` and `end the year:` use `:`, which has one.
- The tokenizer change is the one point where `caption` differs from every other command. It is local to the line whose first word is `caption`, so `worst-case/` scripts tokenize as before.
- Step 5's investigation measured a beat line padded to section 2's pacing at 1415 frames. With the captions placed as above the same frames are emitted, so captions add no frame to the proof.

### Hypotheses

Step 5 is being implemented while this PRD is written. What this PRD assumes about its outcome, from its PRD (`docs/prd/triage/phase1-step5-pad-author-from-record.md`):

- **H-5-1:** A record-based output (`from-record <file.hex>`) has frame 0 equal to the first frame after `cart_resume_game`, with no title prefix. So `"from": 0` means "from the demo's first frame", in `pad_author`, in `replay` and in the attract mode alike. If step 5 delivers a different frame 0, the caption ranges still index the output's `"frames"` array, and only the sentence about the attract mode's frame 0 follows step 5's note.
- **H-5-2:** `p8_input_save` takes a header-members argument: `NULL` for today's bytes, otherwise bare JSON members (leading `"`, no indent, no trailing separator), several joined by `",\n  "`, written after `"input": "pad"` and before `"frames"`. A multi-line value (`[` … `]`) passes its assert, since it ends with `]`. If step 5 fixes another shape (an array of members, say), `pad_caption_write_member` produces whatever one member must be, and nothing else in this PRD changes. If its assert refuses a multi-line value, the member is written on one line (the captions on one line, `, ` between them), and step 8 parses that instead.
- **H-5-3:** `src/cmd/pad_author.c` builds the header members of a record-based output as `"start": "resume"` then `"initial_save": "<hex>"`. This step appends `"captions"` after them, and for a title-based output writes `"captions"` alone.
- **H-5-4:** `replay_source` reads `"start"` by the same exact-key lookup before `"frames"` as the other members, and `"resume"` needs `initial_save`. A `"captions"` member cannot disturb it, and the reader tests include `start` in the "same header values" comparison.
- **H-5-5:** `tools/pad_author_proofs/demo_beats.record.pad` exists, plays section 2's beats from `pico8/demo_record.hex` with idle gaps of the pacing sizes (each beat preceded by `idle 90`, padded to 150, closing `idle 180`), and the `*.record.pad` proof kind runs `pad_author` (self-check through `replay` from the resume) then `REPLAY_STARTS=2`. If the file or the kind is missing, the captioned positive proof becomes the captioned version of whatever record-based proof step 5 delivered. If none exists, the implementer adds one title-based captioned script, `tools/pad_author_proofs/captions.ok.pad`, and a `*.ok.pad` rule to `make pad-author-proof` that only requires `pad_author` to succeed, self-check included. The step note says so.
- **H-5-6:** Step 5 does not change `pad_author`'s tokenizer, `emit` or `pad_author_run`'s end beyond the `from-record` command and its byte store in `pad_author_t`. So this step's changes to those functions do not overlap with step 5's.
- **H-5-7:** `pad_author_run_line` (`--try`) keeps the main run's state, the open caption included. That is how `--try` works today for every other piece of state.

### Decisions taken without a human (guideline rules)

No human answers questions in this run. Each decision below was taken under `.claude/skills/guideline/SKILL.md` ("make the plan work", "keep it safe", "the smaller change"):

- **A button down anywhere in the first 90 frames of a caption is refused**, not only a 0-to-non-zero edge. This serves section 2's pacing goal, and it is the same code as the edge rule.
- **`"captions"` is a header member, before `"frames"`.** This reuses step 5's writer, needs no trailer argument, and keeps the captions reviewable.
- **The member is absent when there is no caption.** Every existing output stays byte-identical (CLAUDE.md, plan guardrails).
- **The line break is stored as `\n` in `text`, not as `/`.** `/` has a glyph, and `p8_print` draws no line breaks, so step 11 splits the text at `\n`.
- **The too-short refusal is located at the opening `caption` line**, and the too-early refusal at the pressing command's line.
- **A caption cut by `--stop-at` is not checked against the 150-frame rule.** The output is a prefix, used to inspect state.
- **The rules live in a separate pure module**, unit-tested in `make test` at no measurable cost, and linkable by step 8.
- **Extra refusals with unit tests only, no proof:** an empty line, `caption` without text, and `caption -` with nothing shown. The plan names six refusals for proofs, and these three are script hygiene.
- **The closing card's 180 frames and the 3600-frame cap are not checked by the tool.** Step 6 lists six rules, and those two belong to the demo's content (step 7).
