---
title: "Select button"
description: "The pad has no Select button, so step 9 has no button to cycle colonies or end the year on, and no recording or keymap can name one."
status: needs-triage
---

Plan 12 (`docs/plan-12.md`), phase 1 — Groundwork, step 3, verbatim:

> 3. **Select button.** Add `P8_PAD_SELECT` to `src/p8/input.h`, `"select"` to the name tables in `src/devtools/input_json.c` and `src/platform/keymap.c`, and a `select =` line to `keymap.cfg` (keyboard key plus `MIDDLE_LEFT`). Verified by `tests/test_keymap.c` and `tests/test_replay_source.c` reading the new name. Every e2e is unchanged (no replay presses it yet).

## Problem Statement

The plan's new scheme gives the Select button two jobs (step 9: a tap cycles colonies needing attention, a 30-frame hold ends the year). Today the pad has eleven buttons, from Up (bit 1) to Start (bit 1024), and Select exists nowhere:

- the input frame has no bit for it, so pad navigation could not read it;
- a recording (`events.json`) cannot name it: `"select"` is exactly the unknown name the loader's refusal fixture uses today;
- the keymap parser refuses a `select =` line as an unknown PSX button, so a player cannot bind a key or the gamepad's left middle button to it.

Steps 9 and later need a Select bit that the replay files, the recording writer and the keymap all carry, before any mapping reads it.

## Solution

Select becomes the pad's twelfth button, bit 2048, named `select` everywhere a button has a name:

- in recordings, `"select"` loads and saves like any other button, in bit order (after `"start"`);
- in `keymap.cfg` and its built-in copy, a `select = RIGHT_SHIFT, MIDDLE_LEFT` line binds it; a keymap without a `select` line is now refused like one missing any other button;
- the live raylib build fills the bit when one of its bindings is down.

Nothing in the game reads the bit yet: pressing Select in live play does nothing, and no replay presses it, so every e2e, end-state reference and pinned test value stays as it is.

## User Stories

1. As the agent of step 9, I want a `P8_PAD_SELECT` bit in the input frame, so that pad navigation can read Select taps and holds.
2. As the agent of step 9, I want the bit to be the next free one (2048), so that every existing mask, replay and pinned value keeps its meaning.
3. As a replay author, I want to write `"select"` in a recording frame, so that re-authored replays can tap and hold Select.
4. As a replay author, I want `"select"` combined with other buttons in one frame to load as the OR of their bits, so that a Select hold can overlap a D-pad press.
5. As a replay author, I want `"select"` listed twice in one frame to be refused, as any duplicate button is, so that a typo cannot silently pass.
6. As a developer, I want the recording writer to save the Select bit as `"select"`, after `"start"`, so that a recorded live session replays byte for byte.
7. As a developer, I want the appended recording (written frame by frame during live play) to stay byte-identical to a whole-file save with Select in it, so that the two writers keep agreeing.
8. As a developer, I want the writer to keep refusing, by assert, any bit past the last named button, so that a future thirteenth bit cannot be dropped silently from a recording.
9. As a developer, I want the loader's "unknown button" refusal still tested with a name that is really unknown, so that adding `select` does not quietly delete that test's meaning.
10. As a player on PC, I want a keyboard key for Select in the default keymap, so that I can use the step 9 features without a gamepad.
11. As a player on PC, I want that key to be the same on QWERTY and AZERTY keyboards, so that the default works on both layouts the README documents.
12. As a player on PC, I want that key to be free (not bound to another button, not one of the keys the game reserves), so that the default keymap still parses.
13. As a player with a gamepad, I want Select on the gamepad's left middle button (`MIDDLE_LEFT`), so that it sits where Select sits on a PlayStation pad, mirroring Start on `MIDDLE_RIGHT`.
14. As a player, I want the versioned `keymap.cfg` to stay identical to the built-in default text, so that deleting it or keeping it gives the same controls.
15. As a player whose own `keymap.cfg` predates this step, I want a clear error naming the missing `select` line, so that I know the one line to add.
16. As a player, I want to be able to rebind Select to any free key or gamepad button, as any other button, so that the keymap rules stay uniform.
17. As a player, I want pressing Select today to do nothing at all, so that a stray press cannot change my game before step 9 gives it a job.
18. As the agent of step 6 onward, I want a Select press not to touch the cheat sequence state, so that the cheat's activation counts in the end-state references stay valid.
19. As a reviewer, I want every e2e `.json.sha256` unchanged and `make end-state` passing on every replay, so that this step is proven to change no behaviour.
20. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
21. As a reviewer, I want the PSX-button name table of the keymap and the recording name table to list the same twelve names in the same bit order, so that a name means one bit everywhere.

## Implementation Decisions

- **Bit.** `P8_PAD_SELECT` is 2048, the bit after `P8_PAD_START`. No existing bit moves: every replay, fixture and pinned mask keeps its value.
- **Name.** `select`, lowercase like the other eleven. It is appended last in the recording name table (whose index is the bit) and in the keymap's PSX-button name table and bit table, so both tables stay in bit order. The recording writer's frame-line buffer already fits a frame holding all twelve names (about 120 of its 160 characters); no size changes.
- **Writer guard.** The writer's assert that refuses a bit past the last named button keeps its logic (it is derived from the table length) and its message now names `P8_PAD_SELECT` as the last named bit.
- **Keymap.** The keymap's button count goes from 11 to 12. The built-in default text and the versioned `keymap.cfg` gain one line, after `start`: `select = RIGHT_SHIFT, MIDDLE_LEFT`. Both stay byte-identical (an existing test checks it).
  - Keyboard key: `RIGHT_SHIFT`. It is not bound to another button, not reserved (Space, Page Up, Page Down, End, Escape), sits at the same place on QWERTY and AZERTY, and is the usual Select key of emulators, next to Enter (Start). Rejected: a letter key (letters move between QWERTY and AZERTY, so the README would need two entries), Tab and Backspace (no stronger case, and further from Enter).
  - Gamepad: `MIDDLE_LEFT` (raylib gamepad button 13), as the plan says. It is already in the name table and checked against raylib at compile time by the backend; nothing else in the backend changes, since it fills the mask through the keymap's table loop.
  - The parser's rule "every PSX button needs a line" is kept: a pre-existing personal `keymap.cfg` without a `select` line is refused at launch with the existing message naming `select`. Rejected: making `select` optional with a default — one rule for all buttons, and the versioned file is updated in the same commit.
- **Game.** No game or pad navigation code reads the bit in this step. Pad navigation already ignores bits it does not test, and the cheat only reads × and ○, so a Select press is inert. The README Controls table is not changed: Select has no action yet, and step 15 rewrites that section.
- **Refusal fixture.** The loader's unknown-button fixture uses `"select"` as its unknown name; it is changed to `"l2"` (a PSX button the pad does not model), so the refusal test keeps its meaning. The keymap test that uses `select` as an unknown PSX button switches to `l2` the same way.

## Testing Decisions

- A good test drives the public surface — a recording file through the loader, the replay source and the writer; a keymap text through the parser and the defaults; a pad mask through pad navigation's step — and asserts on bits, names and returned statuses, never on table internals.
- **`tests/test_keymap.c`** (prior art: its own tests):
  - the defaults hold the plan table in bit order, with the new row `RIGHT_SHIFT` / `MIDDLE_LEFT`;
  - the PSX button names follow the pad bits, now twelve, with `select` at bit 2048;
  - the versioned file equals the default text and loads as the defaults (existing test, now covering the new line);
  - a held `RIGHT_SHIFT` alone, and a held `MIDDLE_LEFT` alone, give `P8_PAD_SELECT`; holding every binding gives all twelve bits (24 bindings);
  - a text with every line but `select` is refused with line 0 naming `select`;
  - the "unknown PSX button" case uses `l2` instead of `select`;
  - the plain-form and whitespace-variant parse tests carry the `select` line.
- **`tests/test_replay_source.c`** (prior art: its pad-recording and hand-written-text tests): a recording written as literal text — not by the writer — holding `"select"` alone, `"select"` with `"up"` and `"start"`, and an empty frame, is served as `P8_PAD_SELECT`, the OR of the three bits, and 0, in order. The written-then-served pad test also gets a frame with the Select bit.
- **`tests/test_input.c`** (prior art: its every-button round trip and canonical-order tests): the every-button list grows to twelve, so save/load and append/save round trips cover Select; the canonical-order test gets a frame whose expected text ends with `"start", "select"`; a frame naming `"select"` twice is refused. The unknown-button fixture now names `l2`.
- **`tests/test_pad_nav.c`** (prior art: its pause and free-mode tests driving `run_frame`): one test feeds the same short sequence of masks with and without `P8_PAD_SELECT` added to every frame (including a tap and a hold of more than 30 frames) and asserts the played frames, the cursor and the nav state are identical — the bit is inert until step 9.
- **No e2e is added or changed.** `make e2e` passes with every `.json.sha256` unchanged; `make end-state` passes on all replays (cheat counts unchanged); `make test-all` passes; `make raylib` still compiles and links (the backend's compile-time name checks cover `RIGHT_SHIFT` and `MIDDLE_LEFT`). The coverage check of section 2 is not required: this step re-authors no replay.
- No test gets slower in a measurable way: the new cases are a few masks and short hand-written files.

## Hypotheses

Step 2b (feasibility study, `docs/prd/triage/phase1-step2b-feasibility.md`) runs in parallel and is not committed. This PRD assumes:

- **H-2b-1** — Step 2b changes none of the files this step touches: `src/p8/input.h`, the recording name table and writer, the keymap module and `keymap.cfg`, `tests/test_keymap.c`, `tests/test_replay_source.c`, `tests/test_input.c`, the unknown-button fixture. If it does, this step merges its edits onto 2b's version.
- **H-2b-2** — Step 2b commits nothing that presses or names Select in a replay (its Select-hold budget and `initial_save` trials stay in `tmp/`, as its PRD says), so no committed recording breaks when `"select"` becomes a known name.
- **H-2b-3** — Whatever end-state references are committed when this step lands (step 2a's `8defcba`, or ones step 2b regenerates after a rest-y fix) are the ones `make end-state` checks here; this step regenerates none, and if 2b adds an exceptions file, this step adds no entry to it.

## Out of Scope

- Any meaning for Select: the colony cycle and the year-ending hold are step 9.
- Any other pad mapping change, any e2e re-authoring, and the README Controls section (step 15).
- A Select on the PSX build (paused).

## Further Notes

- The keyboard key is a first choice, rebindable in `keymap.cfg`; step 15 documents it with the rest of the new scheme.
- Single TDD cycle: no Implementation Chunks.
