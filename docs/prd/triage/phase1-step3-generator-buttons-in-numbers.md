---
title: "Generator low level in button numbers"
description: "The .pad scripts, the generator's logs and the pad-metrics report still name PSX buttons after the replays moved to button numbers 1-12."
status: needs-triage
---

## Problem Statement

Plan 17 step 3 (Phase 1, abstract buttons). After step 2, every replay stores button numbers 1–12, but the generator's own vocabulary is still PlayStation names. A `.pad` script types `tap cross` or `hold down+right 2`. The generator's per-frame mode log prints `[cross]`. The pad-metrics report prints `dead f16 press square aim required`. The Makefile's `pad-metrics-check` expects that text word for word. The generator and the metrics tool each keep a table of the twelve names only to do this. The person writing a replay script thinks in one vocabulary and reads the replay in another. The name tables also keep the PSX vocabulary in `src/devtools/` that plan 17 is removing, before step 8 makes the high-level commands read the bindings.

## Solution

The low-level commands `tap` and `hold` take button numbers joined by `+` (`tap 7`, `hold 2+4 2`, `tap 5+6`). This is the numbering of plan 17 section 2 and of the replays: 1 up, 2 down, 3 left, 4 right, 5 cross, 6 circle, 7 square, 8 triangle, 9 L1, 10 R1, 11 start, 12 select, with button N on bit `1 << (N-1)`. All 18 committed `.pad` files are converted. The generator and the metrics tool lose their name tables and print numbers wherever they printed a button. The expected required group in the Makefile becomes `dead f16 press 7 aim required;dead f17 press 6 aim required;dead f293 press 6 jump required`: the same three frames, modes and verdicts. No frame any script produces changes, so every generated JSON, every `worst-case/` file and every e2e `.sha256` stays byte-identical.

## User Stories

1. As a replay author, I want to write `tap 7` in a `.pad` script, so that the script uses the same button numbers as the JSON it generates.
2. As a replay author, I want to write a chord as `hold 2+4 2`, so that diagonal holds keep the `+` syntax I already know.
3. As a replay author, I want `tap cross` to be refused with `tap: unknown buttons 'cross'`, so that an old-style script fails loudly and does not silently produce other frames.
4. As a replay author, I want `tap 13`, `tap 0`, `tap 07`, `tap 5+`, `tap +5` and `tap 5+5` to be refused, so that a typo never becomes a different button or no button at all.
5. As a replay author, I want `hold 13 4` to be refused with `hold: unknown buttons '13'`, so that `hold` and `tap` report the same way.
6. As a replay author, I want the high-level commands (`title`, `probe`, `fold`, `jump`, `colonise`, `slipway`, `build`, `research`, `menu`, `bar`, `score`, `attention`, `back`, `pause`, `restart`, …) to behave exactly as before, so that my existing scripts still produce the same frames.
7. As a maintainer, I want all 18 committed `.pad` files converted in the same commit as the parser, so that `make pad-author` and `make pad-author-proof` never see a script in the old format.
8. As a maintainer, I want `make pad-author` followed by `git diff --exit-code worst-case/` to show nothing, so that I know the conversion changed no pressed frame in the four full games.
9. As a maintainer, I want every `*.frames.pad` proof to keep regenerating its e2e JSON byte for byte, so that the generator is still proven against the replays.
10. As a maintainer, I want every `*.end-state.pad` proof to keep reaching its e2e end state, so that the closed-loop commands are still proven.
11. As a maintainer, I want every `*.refused.pad` proof to keep failing for its stated reason, so that the generator's refusals are still proven.
12. As a maintainer, I want the chord refusal to read `chord: buttons 5 and 6 pressed on the same frame`, so that the message matches what the script typed (`tap 5+6`).
13. As a maintainer, I want the empty-cone refusal to name the direction pressed as a button number (`empty cone: no candidate for button 1 of the hovered object in jump mode`), so that no message needs a name table.
14. As a maintainer, I want the generator's per-frame mode log (`tmp/pad-author/<name>.modes.txt`) to print the held and pressed buttons as `[5]` or `[2+4]`, so that the log reads like the script that produced it.
15. As a maintainer, I want the pad-metrics report to print `presses jump 6 …`, `repeats jump 3 …`, `dead f16 press 7 aim required` and `repeat f23 3 jump dead-other`, so that the report uses the replay's numbers.
16. As a maintainer, I want the generator's `total` lines and the report's `total` lines to change together, so that `make pad-metrics-check`'s comparison of the two still passes.
17. As a maintainer, I want `PAD_METRICS_PROOF_REQUIRED` to hold the same three entries (f16 aim required, f17 aim required, f293 jump required) in numbers, so that the check still pins the same dead presses found from game state.
18. As a maintainer, I want `make pad-metrics-worst-case` to print the same `section4` lines as before, byte for byte, so that the four games' verdicts are visibly unchanged.
19. As a maintainer, I want the `section4` labels left as they are, so that the step changes no line `pad-metrics-worst-case` prints.
20. As a maintainer, I want the exported function that returns a button's name removed from the metrics module's interface, so that nothing can bring the name table back through it.
21. As a maintainer, I want a unit test showing the report prints button 1 and button 12 for the lowest and highest bits, so that an off-by-one between bit index and button number fails in `make test`.
22. As a maintainer, I want the existing report-text assertions in the pad-metrics unit test moved to numbers, so that they pin the new format and not the old one.
23. As a maintainer, I want two new refusal proofs (a PSX name, a button out of range), so that rejecting the old format is checked by `make pad-author-proof` without adding a test binary to `make test`.
24. As a maintainer, I want comments in the `.pad` files that name a button rewritten to numbers, so that the scripts keep no PSX vocabulary outside `# expect:` lines that copy a message verbatim.
25. As a maintainer, I want the Makefile's help text for `pad-author` and `pad-metrics-check` to describe the chord and the required group in numbers, so that the legend matches what the targets check.
26. As a maintainer, I want `make test` and `make e2e` no slower than the step 1 baseline, so that the fast loop stays fast.
27. As a maintainer, I want the `--try` mode of the generator to accept the same number syntax, so that exploratory runs and scripts share one parser.
28. As a future implementer of step 8, I want `tap` and `hold` to be raw numbers that never go through bindings, so that step 8 only changes the high-level commands.

## Hypotheses

Step 2 ("Replays in numbers") is being implemented in parallel and is not committed. This PRD was written from `HEAD` (9f0ac80) only. Each assumption about step 2's outcome is listed here. If one is false when this step starts, re-check the decisions that cite it.

- **H-2-1.** Step 2 is committed before this step starts. Every committed `"pad"` array (the 17 `tests/e2e/*.json`, the 5 `worst-case/*.json`, the fixtures, the inline JSON in tests) holds button numbers 1–12. The generator's JSON output (the frames file `pad_author` writes, through `input_json` or its own writer) emits numbers. At step 2's end, `make pad-author-proof` and `make pad-author` + `git diff --exit-code worst-case/` pass. This step's byte-identity checks compare against those step-2 files.
- **H-2-2.** Step 2 leaves the `.pad` scripts, the generator's `tap`/`hold` parser, its mode log, the metrics module's button-name table, the exported button-name function, the report text and `PAD_METRICS_PROOF_REQUIRED` as they are in `HEAD`, all still in PSX names. If step 2 already moved one of them, this step drops that part and keeps the rest.
- **H-2-3.** Step 2 uses plan 17's numbering: button N is bit `1 << (N-1)`, 1 up … 12 select. Neither step changes the bit layout, so `PAD_METRICS_BUTTON_COUNT` stays 12 and index `b` prints as `b + 1`.
- **H-2-4.** Step 2 does not rename `P8_PAD_*` (step 4 does). The generator's high-level commands still use those constants, which this step leaves alone.
- **H-2-5.** Step 2 adds no shared "button number" parsing or formatting helper that the generator is expected to reuse. If it did, and the helper takes a single token in 1–12 with no I/O, the generator's `+`-list parser may call it per token. The accepted syntax of this PRD does not change.
- **H-2-6.** Step 2 removes the name table in `input_json`. This step's check that `src/devtools/` has no PSX button-name table therefore covers `input_json` too. If step 2 kept one there, this step does not remove it: the check leaves that file out and the step note records it.

## Implementation Decisions

- **Syntax of a button list.** One token: decimal button numbers joined by `+`. Each number is 1–12, digits only, with no sign, no leading zero and no whitespace. Refused: an empty token, an empty element (`5+`, `+5`, `5++6`), a duplicate number (`5+5`), any name. Every refusal uses the existing message `<command>: unknown buttons '<token>'`. Duplicates and empty elements are new refusals: the old name parser quietly accepted `cross+` and `cross+cross`. No committed script relies on them.
- **Parser placement.** The generator owns its list parser, a small static function replacing the name lookup. It returns the bit mask, or 0 for a refused token, as today. The script loop and the `--try` line runner share it (they already share `tap`/`hold` handling).
- **The generator's mode log** prints a button set as `[` + numbers joined by `+` + `]`, lowest number first (`[5]`, `[2+4]`, `[]` for none). It keeps the same frame, mode, `pressed` and `effect` fields.
- **Generator messages.** Only messages that printed a button through the name table, or that describe the cheat chord the scripts type, change:
  - the chord refusal becomes `chord: buttons 5 and 6 pressed on the same frame`;
  - the empty-cone refusal becomes `empty cone: no candidate for button N of the hovered object in jump mode`, with N the first direction pressed (1–4).
  Other literal messages that name a PSX button for a high-level command's internal step (`dead circle: …`, `square: no aim mode`, `menu: closed by circle`, `title: no … game started on the cross`, …) stay. They belong to the high-level path, which step 8 rewrites around actions.
- **Metrics module.** The twelve-name table and the exported button-name function are deleted from the interface. Every report line that printed a button prints its number: the `presses` lines (totals block and detail block), the `repeats` lines, the listed `dead f<frame> press <N> <mode> <required|other>` rows and the `repeat f<frame> <N> <mode> <verdict>` rows. The mode names table stays, since modes are not buttons. The `section4 …` lines keep their wording (including "dead circle presses"), so `make pad-metrics-worst-case`'s output stays byte-identical. Renaming those labels is not this step's job.
- **Shared totals writer.** The generator's `total …` lines and the report's `total …` lines come from the same metrics writer, so both move to numbers at once. `pad-metrics-check`'s diff of the two keeps passing with no Makefile change.
- **Makefile.** `PAD_METRICS_PROOF_REQUIRED` becomes `dead f16 press 7 aim required;dead f17 press 6 aim required;dead f293 press 6 jump required`. The target's closing echo and the legend at the top describe the group in numbers (f16's button 7, f17's button 6, f293's button 6). The `pad-author` legend describes the refused chord as buttons 5 + 6 and the dead press as button 6. The recipes do not change.
- **Converting the scripts.** A one-off substitution (not committed) maps names to numbers on the `tap`/`hold` lines: up 1, down 2, left 3, right 4, cross 5, circle 6, square 7, triangle 8, l1 9, r1 10, start 11, select 12. Six files carry such lines: `pad_colonise.frames.pad`, `pad_end_turn.frames.pad`, `pad_probe.frames.pad`, `pad_restart_resume.frames.pad`, `chord.refused.pad`, `empty_cone.refused.pad`. Comments in any of the 18 files that name a pressed button are rewritten by hand to numbers (for example "title cross on f12" becomes "title button 5 on f12", "circle at the root" becomes "button 6 at the root", "then Start" becomes "then button 11"). A `# expect:` line copies its message verbatim: `chord.refused.pad` and `empty_cone.refused.pad` follow their new messages, and `dead_circle.refused.pad` keeps its unchanged one. High-level command lines (`restart`, `title 12 reasonable`, …) are untouched.
- **New refusal proofs** in the proofs directory, each starting with `title 12 reasonable` then `idle 4`, like `chord.refused.pad`:
  - a script with `tap cross`, expecting `tap: unknown buttons 'cross'`;
  - a script with `hold 13 4`, expecting `hold: unknown buttons '13'`.
- **No change to frames.** The conversion is a pure renaming of input, so every generated frames file, every `worst-case/*.json`, every e2e `.sha256` and every end state stays byte-identical (relies on H-2-1).
- **Byte identity of other output.** The `.modes.txt` files and the pad-metrics report change by design: button names become numbers, and that is all. `make pad-metrics-worst-case`'s printed `section4` lines do not change.

## Implementation Chunks

1. **Generator in numbers.** Number parser for `tap`/`hold`, mode log and the two messages in numbers. Convert the 18 `.pad` files (lines and comments), add the two refusal proofs, update the `pad-author` legend. The generator stops calling the metrics module's button-name function, which still exists after this chunk. Verified: `make test`, `make pad-author-proof`, `make pad-author` + `git diff --exit-code worst-case/`, `make pad-metrics-check` (the required group still in names at this point), `make pad-metrics-worst-case`.
2. **Report in numbers.** Delete the metrics module's name table and the exported name function. Print numbers in every report line listed above. Move the pad-metrics unit test's report assertions to numbers and add the button 1 / button 12 test. Move `PAD_METRICS_PROOF_REQUIRED`, the target's echo and the `pad-metrics-check` legend to numbers. Verified: `make test`, then the four commands of the step (`make pad-author-proof`; `make pad-author` + `git diff --exit-code worst-case/`; `make pad-metrics-check`; `make pad-metrics-worst-case`), plus `make e2e`.

Each chunk leaves `make test-all` and the four generator commands green on its own.

## Testing Decisions

- A good test here checks observable output: the frames a script produces, the message a refused script prints, the report's text. It never checks how a parser walks a string.
- **Frames: no new test.** Identity is proven by the existing proof targets, outside `make test-all`. `*.frames.pad` must regenerate the e2e JSON byte for byte, `*.end-state.pad` must reach the e2e end state, and `make pad-author` + `git diff --exit-code worst-case/` must show nothing.
- **Parser refusals** are proven by the two new `*.refused.pad` proofs plus the converted `chord.refused.pad` and `empty_cone.refused.pad`. No unit-test binary is added for the generator: it would link the whole engine into `make test` for a parser of a dozen lines, against the rule that `make test` never gets slower. Prior art: the existing `tools/pad_author_proofs/*.refused.pad` and their `# expect:` lines.
- **Metrics report**: `tests/test_pad_metrics.c` (prior art: its `report_has` assertions on the written report).
  - Every existing assertion on a line that names a button moves to its number, e.g. `dead f0 press 6 jump required`, `repeat f23 3 jump dead-other`, `presses jump 6 1 chords 0 live 0 dead-required 1 dead-other 0`, `repeats jump 3 1 …`, `dead f0 press 2 jump required`, `dead f0 press 6 aim required`, `dead f1 press 7 aim required`. These assertions froze the old text format, not game behaviour, so changing them is allowed (`CLAUDE.md`). Plan 17's "mechanical changes only" guardrail covers the pad tests in steps 4 and 9–13, not this format.
  - New: a press of the lowest bit (up) and of the highest bit (select) in one run prints `presses <mode> 1 …` and `presses <mode> 12 …`. This catches an off-by-one between bit index and button number.
  - Assertions on `section4 …` lines stay unchanged.
- **The `make pad-metrics-check` required group** is the end-to-end check that the report's numbers land on the same three frames and verdicts.
- Timing: `make test` and `make e2e` against step 1's baseline (`make test` 4.27–4.58 s, `make e2e` 0.24–0.25 s on a warm build). The step adds one small test function and no binary.

## Out of Scope

- The JSON `"pad"` format, `input_json`, the e2e/`worst-case/`/fixture JSON files, and the generator's JSON writer: step 2.
- Renaming `P8_PAD_*` to `P8_PAD_BUTTON_*`, and the direction enum: step 4.
- The high-level commands' buttons and their messages that name PSX buttons (`dead circle`, `square: …`, `menu: closed by circle`, …): step 8 moves the high-level path to bindings.
- The `section4` labels of the pad-metrics report: they stay so that `make pad-metrics-worst-case` prints the same lines.
- `src/platform/keymap.*` and its PSX name table: step 15.
- Any change to pressed frames, game behaviour or e2e baselines.

## Further Notes

- Plan 17 section 2 guardrail: this step touches `src/devtools/pad_*`, so the four generator commands are mandatory. The step changes exactly two of their outputs: the required-group text of `pad-metrics-check` and the text of the `tmp/` logs. It changes no frames.
- The step note should record which hypotheses held and the numbers-only `PAD_METRICS_PROOF_REQUIRED` value as committed.
- A grep for the twelve PSX names in quotes (`"cross"`, `"circle"`, …) inside `src/devtools/pad_author.c` and `src/devtools/pad_metrics.c` should find nothing after the step. The literal messages kept above contain the words but not those quoted table entries.
