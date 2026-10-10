---
title: "Replays in numbers"
description: "Replay and recording files name pad buttons with PSX words (\"cross\", \"l1\"), which keeps the PSX vocabulary alive in the data the remappable pad is built on."
status: needs-triage
---

## Problem Statement

Plan 17 makes the pad remappable through abstract buttons 1–12. The replays are the source of truth of the port, but every `"pad"` frame in them still names buttons with PSX words: `{ "pad": ["right", "cross"] }`. The loader and the writer of the replay JSON (the devtools input JSON module, used by `replay`, the recorder of the raylib `game`, and the `.pad` generator) only know those names. As long as the files speak "cross", the later steps (bindings that map `mode.action` to button N, a fixture that swaps buttons 7 and 8) would have to translate between two vocabularies, and plan 17 explicitly rejects keeping names as aliases.

## Solution

Every `"pad"` array in the repository lists button numbers instead of names, in the bit order of plan 17 (1 up, 2 down, 3 left, 4 right, 5 cross, 6 circle, 7 square, 8 triangle, 9 L1, 10 R1, 11 start, 12 select; button N is bit `1 << (N-1)`). `{ "pad": ["right", "cross"] }` becomes `{ "pad": [4, 5] }`.

- A one-off conversion script rewrites every file with `"pad"` frames: the 17 e2e replays, the 5 worst-case games, the 17 unit-test fixtures, and the inline JSON strings in the unit tests.
- The JSON writer (recordings, `p8_input_save` used by the generator, `p8_input_append` used by the recorder) writes numbers.
- The JSON loader reads numbers 1–12 only and refuses names.

Nothing the game does changes: every e2e capture hash, every end state, every pad-metrics report and every generated worst-case file stays byte-identical.

## User Stories

1. As a port developer, I want every `"pad"` frame to list button numbers, so that the replay data speaks the same abstract-button language as the bindings to come.
2. As a port developer, I want the conversion done by a script rather than by hand, so that 22 replays of up to thousands of frames are converted without a typo.
3. As a port developer, I want the conversion to leave every `tests/e2e/*.json.sha256` byte-identical, so that I know the replays still drive the exact same frames.
4. As a port developer, I want `make end-state` to match on every replay after the conversion, so that the game state reached by each replay is proven unchanged.
5. As a port developer, I want `make pad-metrics-worst-case` unchanged, so that the input metrics of the worst-case games are proven unchanged.
6. As a port developer, I want `make pad-metrics-check` to pass unchanged, so that the generator and the replay still agree on the proof replay.
7. As a port developer, I want `make pad-author-proof` to pass, so that each `.frames.pad` proof still regenerates its e2e replay byte for byte in the new format.
8. As a port developer, I want `make pad-author` followed by `git diff --exit-code worst-case/` to show nothing, so that the generator's own writer and the conversion script agree byte for byte.
9. As a port developer, I want the generator's JSON output to switch to numbers in the same change as the data, so that there is never a commit where the generator would rewrite the worst-case files.
10. As a port developer, I want the loader to refuse `"cross"`, so that no PSX name can sneak back into a replay.
11. As a port developer, I want the loader to refuse `13`, so that a button outside 1–12 is an error and not a silent bit past the pad.
12. As a port developer, I want the loader to refuse `0`, a negative number, a number with a leading zero, a fraction, and a number glued to other characters, so that only the canonical form the writer emits is accepted.
13. As a port developer, I want a button listed twice in one frame to stay an error, so that a malformed frame is caught as before.
14. As a port developer, I want buttons listed in any order to still load, so that a hand-written replay need not be sorted.
15. As a port developer, I want the writer to keep the exact line layout (`    { "pad": [1, 5, 11] }`, ascending, `", "` between buttons), so that only the tokens change and diffs of converted files stay readable.
16. As a port developer, I want a failed load to still leave the previously loaded input untouched, so that the existing failure contract holds for the new errors.
17. As a port developer, I want the parser fixtures whose purpose is a bad button to keep their purpose in the new format (unknown button becoming `13`, a duplicate becoming a repeated number), so that the error paths stay covered.
18. As a port developer, I want every unit test that loads a fixture to pass with its assertions unchanged, so that the conversion is proven faithful by tests it did not write.
19. As a port developer, I want a grep to find no PSX button name inside any `"pad"` array in the repository, so that the conversion is proven complete.
20. As a port developer, I want `tools/e2e_limits.sh` to keep counting frames and checking the header the same way, so that the e2e limits still hold on the converted files.
21. As a player recording a game with the raylib build, I want my new recordings written in numbers, so that they replay with the same `replay` binary as the e2e files.
22. As a port developer, I want `make test` and `make e2e` no slower than the step 1 baseline (4.27–4.58 s and 0.24–0.25 s), so that the fast loop stays fast.
23. As a future implementer of the bindings steps, I want the replay format settled before the bindings exist, so that the swap-twin and unbound-action fixtures of Phase 4 can be written directly in numbers.

## Implementation Decisions

- **Module modified: the devtools input JSON module** (loader `p8_input_load`, writer `p8_input_save`, appender `p8_input_append`). Its public interface does not change: same functions, same return contracts. Only the token format inside `"pad"` arrays changes. Its PSX name table is deleted.
- **Number grammar accepted by the loader:** an array element is one or more ASCII digits, with no sign, no leading zero, no fraction or exponent, whose value is 1–12, followed by a separator (space, tab, newline, carriage return, comma) or `]`. Anything else in an element is a parse failure: a quoted string (so `"cross"` fails), `0`, `13`, `05`, `-1`, `5.0`, `5x`. The existing leniency on separators between elements is kept as is (not tightened in this step).
- **Duplicates stay an error** and order stays free, exactly as with names.
- **Writer layout:** the frame line keeps its current shape, `    { "pad": [` then the set buttons in ascending number order separated by `", "`, then `] }`. Only the token changes from a quoted name to a decimal number. The assert that guards a bit past the 12th button stays, with its message reworded to say the bit has no button number.
- **The conversion script** is a one-off, run from outside the repository (scratchpad) and not committed; its approach goes in the commit message (docs and the tree hold only what is still true). It replaces each quoted name inside a `"pad"` array with its number, in place, without reordering and without touching anything else on the line, so files that are already in canonical order stay byte-compatible with the writer. It handles both raw JSON files and the C-escaped form inside test strings (`\"pad\": [\"up\"]`); the C sites are few enough to review by eye after the script runs.
- **Files converted:** the 17 `tests/e2e/*.json`, the 5 `worst-case/*.json` (the script leaves `title_idle.json` unchanged because it has only empty frames), the 17 `tests/fixtures/*.json` with `"pad"` frames, and the inline JSON in the unit tests (the input tests, the replay source tests, the e2e limits tests). `.json.sha256`, `.end-state` and `.txt` files are not touched.
- **Bad-name fixtures rewritten for numbers:** the unknown-button fixture's `["l2"]` becomes `[13]`; the duplicate-button fixture's `["cross", "up", "cross"]` becomes `[5, 1, 5]`; the inline "select named twice" frame becomes `[12, 1, 12]`. File names and test names keep their meaning (an unknown button, a duplicate button) and are not renamed.
- **Expected writer output in the input tests** changes mechanically (`["up", "cross", "start"]` becomes `[1, 5, 11]`, and the cross-check against the committed demo recording looks for `[4, 5]`). No assertion on decoded button masks changes.
- **The generator** writes its JSON through the input JSON module, so it moves to numbers with no change in its own source. Its mode log (`tmp/pad-author/<game>.modes.txt`, `[cross+up]`) is not a `"pad"` array and keeps names until step 3 removes the generator's name tables.
- **Recordings of the raylib `game`** are written in numbers from this step on. A recording made before this step, in names, is refused by `replay` like any malformed file; no alias or migration path is provided (plan 17 rejects aliases).
- **Completeness check:** after conversion, a grep over the tree (excluding `.git` and `tmp/`) for a `"pad"` array that contains a quote or a letter, in both the raw form and the C-escaped form, finds nothing. Run once and recorded in the commit message; it is not added to `make test`.

## Implementation Chunks

Each chunk ends with `make test-all` green and every `.sha256` unchanged.

1. **Loader reads numbers.** The loader accepts the number grammar above in addition to names. Unit tests: a frame of numbers loads to the right mask, numbers in any order load, each refused form of the grammar (`0`, `13`, `05`, `-1`, `5.0`, `5x`) fails and leaves the input untouched, a repeated number fails.
2. **Writer writes numbers, data converted.** The writer emits numbers; the conversion script runs over every file listed above, including the inline test strings; the expected writer output in the input tests is updated mechanically. Verified by `make test-all`, `make end-state`, `make pad-metrics-worst-case`, `make pad-author-proof`, `make pad-author` with `git diff --exit-code worst-case/`, and `make pad-metrics-check`, all unchanged.
3. **Loader refuses names.** The name table is deleted, the bad-name fixtures are rewritten for numbers, and a unit test refuses a frame naming `"cross"` and a frame with `13`. The grep for names in `"pad"` arrays comes back empty. The full verification list of chunk 2 runs again.

## Testing Decisions

- A good test here goes through the public interface of the input JSON module: write a small JSON text, load it, and check the decoded button masks or the failure; or save frames and compare the exact bytes written. No test reaches into the parser's helpers.
- **Tested module:** the input JSON module, in the existing input unit tests. New cases: numbers load, numbers in any order load, each refused number form fails without touching the loaded input, `"cross"` is refused, `13` is refused, a repeated number is refused. The existing round-trip test over every button (save then load all 12 bits) proves the writer and loader agree on all 12 numbers.
- **Fixture-loading tests are the faithfulness proof:** the tests that load `events_valid`, `events_valid_ox`, `events_pad_unordered`, `demo_events`, the replay source tests and the game-logic fixtures (wormholes, refused grabs/probes/slipways, title picks) keep their assertions byte for byte. Only the fixtures' bytes change.
- **E2e and devtools proofs** (not unit tests, run as the plan's guardrail): `make e2e` (every `.sha256` identical), `make end-state`, `make pad-metrics-worst-case`, `make pad-author-proof`, `make pad-author` then `git diff --exit-code worst-case/`, `make pad-metrics-check`.
- **Prior art:** the input unit tests' `assert_load_fails_and_leaves_input_untouched` helper and their inline `write_text` JSON cases; the save-shape test that compares the writer's exact output string.
- Timing: `make test` and `make e2e` are timed once after the change and compared with the step 1 baseline.

## Out of Scope

- The low level of `.pad` scripts (`tap`, `hold` with names), the generator's and pad-metrics' name tables, the generator's mode log and the `PAD_METRICS_PROOF_REQUIRED` text: step 3.
- Renaming `P8_PAD_*` constants: step 4. Unit tests keep using today's constant names.
- The keymap file and its PSX names (`keymap.cfg`, the keymap module and its tests): step 15.
- Any `"bindings"` header member, fixtures for remapped replays, personal-recording flags: Phase 4.
- Tightening the separator leniency of the loader, or any other change to the JSON format beyond the button tokens.
- Converting recordings that live outside the repository.

## Further Notes

- Plan 17 section 2 ("Replay format", "Rejected: PSX button names in the files or the replays") is the source of these decisions.
- The e2e `.sha256` hashes the frame capture, not the JSON, so a faithful conversion cannot move it; a moved hash means a wrong number.
- The pad-author frame proofs compare the generator's output with the e2e JSON byte for byte, which is why the writer and the data must change in the same chunk.
