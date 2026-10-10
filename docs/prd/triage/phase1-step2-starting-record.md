---
title: "The demo's starting record"
description: "The attract-mode demo needs a frozen mid-game save record, cut from full_game at a year where every demo beat is possible."
status: needs-triage
---

## Problem Statement

Plan 18's attract mode plays a recorded pad stream from a save record compiled into the game. That record does not exist yet. It has to come from a real game, not be typed by hand ("cut, never typed"). It has to sit at a year where all of the demo's beats can happen: probe and fold, colonize, slipway, end of year, buy a tech. And it must never move again, even when `worst-case/full_game` is regenerated after a gameplay change. Until the record exists, nothing later in the plan (the `pad_author` record start, the demo script, the compiled demo) has anything to start from.

## Solution

Put a clock-free `label demo-record` in `worst-case/full_game.pad` at the start of year 3412 (the moment after the 7th `bar`, just before full_game's own `research slipgates`). Play the script up to that label, replay the prefix with `SAVE_HEX`, and commit the record that comes out, byte for byte, as `pico8/demo_record.hex`. The label changes no frame: `make pad-author` regenerates `worst-case/full_game.json` with no diff.

The year was picked by investigation while writing this PRD. All four conditions of the step hold there, each proven by actually running the command from the cut, not just by reading the state:

| Condition | Evidence at year 3412 (`make state` on the record) |
|---|---|
| A tech is affordable from science | science 12; `xenofoods` (cost 10) and `slipgates` (cost 12) can both be invented from the tech panel; science per year 5 |
| Unexplored space is reachable by a probe | 49 unknown planets; credits 86; a fold at (118, 307), next to p57, costs 3 and reveals 3 planets (unknown 49 → 46); 5 of the 6 folds tried next to the nearest unknown planets succeed |
| An uncolonized planet is reachable | 9 known planets (p9, p22, p32, p34, p39, p42, p58, p75, p85); `colonise` succeeds on 6 of them (p9, p22, p32, p34, p75, p85) |
| Two colonies can be linked | 14 colonized planets plus 2 structures (count colonies 16), 18 slipways; 17 new slipways between two existing colonies are built, e.g. p23–p33 (38 px), p11–p59 (30 px), p6–p25 (34 px) |

A full beat sequence from the cut also plays through in one go: `fold 118 307; idle 120; colonise p32; slipway p32 p55; year; research xenofoods`. It ends at year 3413 with 17 colonies, 19 slipways, 3 techs, credits 105 and science 8: one more colony, slipway, year and tech than the record, with no cheat.

## User Stories

1. As the plan-18 implementer, I want a committed save record at a mid-game year, so that the demo script of step 7 has a fixed starting point.
2. As the plan-18 implementer, I want that record cut from `worst-case/full_game`, so that it is a real cheat-free game and not typed bytes.
3. As the plan-18 implementer, I want the cut point named by a label in `full_game.pad`, so that anyone can reproduce the cut with two commands.
4. As a maintainer, I want the label to be clock-free, so that `worst-case/full_game.json` and every `worst-case/` replay stay byte-identical.
5. As a maintainer, I want the record frozen in `pico8/demo_record.hex`, so that regenerating `full_game` after a gameplay change never silently moves the demo.
6. As a maintainer, I want the record file in exactly the format `make replay SAVE_HEX=…` writes (the `initial_save` hex format), so that it can be pasted or loaded the same way as `tests/e2e/pad_tech_costs.json`'s and `pad_resume_at_start.json`'s records.
7. As the demo's author, I want enough science in the record to buy a tech, so that the "spend science on new tech" beat is honest, with no cheat and no money from nowhere.
8. As the demo's author, I want unexplored space within a fold's reach, so that the "probe the dark" beat reveals planets on screen.
9. As the demo's author, I want a known, uncolonized planet that can be colonized, so that the "colonize planets" beat has a target.
10. As the demo's author, I want two existing colonies that are not yet linked and can be, so that the "link colonies with slipways" beat is possible even before the demo colonizes anything.
11. As the demo's author, I want enough credits for a fold, a colony and a slipway in one year, so that the beats do not stall on money.
12. As a viewer of the attract mode, I want the demo to start from a game with a few colonies already on the map, so that the demo shows what the game looks like in play, not an empty first minute.
13. As a reviewer, I want the four conditions written with their numbers in the step note, so that I can check the choice of year without rerunning anything.
14. As a reviewer, I want to know why this year and not an earlier or later one, so that the choice is not arbitrary.
15. As a later step's implementer (step 5), I want a verified beat sequence from the cut, so that my proof script starts from something known to work.
16. As a later step's implementer (step 7), I want the exact commands that cut the record, so that I can write them in `pico8/demo.txt` as the plan asks.
17. As a maintainer, I want `make test-all` unchanged and no slower, so that this data step costs the fast loop nothing.
18. As a maintainer, I want every `tests/e2e/*.sha256`, `.end-state` and `worst-case/` file unchanged, so that the plan's guardrails hold.
19. As the plan-pilot, I want the step to stop and ask the user if no year met all four conditions, so that no condition is ever relaxed. (Not triggered: year 3412 meets all four.)
20. As a PSX porter (later), I want the record to fit one memory card block, so that the compiled demo works there too (3852 bytes, under the 8192-byte limit).

## Implementation Decisions

- **The label.** One new line, `label demo-record`, in `worst-case/full_game.pad`, right after the `bar` that closes year 3411's end-of-year browsing (`year`, `menu s2`, `menu p34`, `menu p39`, `score`, `bar`), and right before `research slipgates`. It is the 7th `bar` of the script. Nothing else in the file changes. The name says what it is for. `pause-branch`, the only label today, is used by the `include … until` lines of the sibling scripts; `demo-record` is used through `--stop-at`.
- **Year 3412 (the cut's clock: year 3412 month 1, 2908 frames into full_game).** Every end of year in full_game (16 cuts, years 3404 to 3425) was checked the same way: the state at the cut, then `pad_author --try` from the cut for every tech not yet invented, every known planet, every unlinked pair of colonies within 110 px, and folds in empty space next to the 6 unknown planets nearest a colony. Years 3410, 3412, 3414, 3419, 3420, 3422, 3424 and 3425 meet all four conditions. 3404–3408, 3411, 3421 and 3423 do not: either no tech is affordable, or (3404/3405) no two colonies can be linked. Year 3412 wins because:
  - 3410, the earliest, has science 5 against geneseeds' cost of exactly 5 (no margin) and only 49 credits;
  - 3412 has two affordable techs, 86 credits, income 48 and 16 colonies: still "a few colonies", and early enough for the map to be readable;
  - from 3419 on the map is crowded (25+ colonies, 31+ slipways), which teaches less;
  - 3414 has the same science but only xenofoods affordable, and the full_game line after its cut builds a structure, which is a less natural continuation.
- **How the four conditions are read** (decided here and recorded, since the plan states them in one line each):
  - *A tech is affordable from science*: the record's science is at least the cost of a tech that the tech panel lets you press. Proven by `research <tech>` succeeding from the cut, not by comparing numbers alone, because tier gating hides some cheaper techs (nanomaterials and drillbots cost 6 and 7 but cannot be pressed at 3412).
  - *Unexplored space remains reachable by a probe*: the record already holds a probe, and `pad_author`'s `probe` command refuses a second live one, so "reachable by a probe" means a `fold` to an empty point is paid for and reveals at least one unknown planet. A fold aimed at a planet is refused by the game (the probe takes the planet as its target), so the point must be in empty space.
  - *An uncolonized planet is reachable*: a planet in the `known` state on which `colonise` succeeds.
  - *Two colonies can be linked*: read strictly, as two planets **already colonized in the record** with no slipway between them on which `slipway` succeeds. The colony the demo creates does not count, and neither do structures (s1, s2). The strict reading satisfies the looser one too.
- **The cut is made, never typed**, with the existing tools and no new code:
  1. `make pad-author SCRIPT=worst-case/full_game.pad STOP_AT=demo-record OUT=tmp/demo_record.json`
  2. `make replay EVENTS=tmp/demo_record.json SAVE_HEX=pico8/demo_record.hex`

  The second command prints `save: bytes 3852`, `save: decodes 1`, `save: difficulty reasonable`, `save: clock year 3412 month 1`, `save: resources credits 86`, `save: count nodes 75`. The file is one line of hex plus a newline, 7705 bytes. It was reproduced three times in investigation, always with the same bytes: sha256 `61c39e5630153d0feccaa7da2fe2171e9524a6782cae2c89ee9ab4bb4b6283ae`. If the implementer's bytes differ, master has moved since this PRD was written. Then rerun the four-condition check below before committing; never edit the bytes.
- **The record is the save of the last action, not the cut frame.** The last action before the label is the year end; the score and bar browsing after it write no record. One planet, p46, is still `vanishing` in the record (count nodes 75), while at the cut frame it has already gone (74 planets in the cut's state). After a resume it finishes vanishing. This is expected and accepted, and is not a reason to move the cut. No wormhole lives at the cut, so the record is written.
- **No comment is added to `full_game.pad`.** Under `CLAUDE.md`'s comment rule, the label's name is enough. The two cut commands go in the commit message, and step 7 writes them into `pico8/demo.txt` as the plan says.
- **No code, no Makefile change, no new target.** `pico8/` is not scanned by any guard or wildcard (checked: the Makefile only names `pico8/gfx.json`), so the new file touches no check.
- **Nothing depends on later tool steps.** The record does not need the `attract_after` member, the record-start feature of `pad_author` (step 5), or the `caption` command.
- **What the step note records** (plan-pilot writes it): the label and where it sits; year 3412 and why; the two cut commands; the six `save:` lines; the four conditions with the numbers in the Solution table; the verified beat sequence and its end counts; and the p46 vanishing note.

## Testing Decisions

- No unit test: the step adds data (one script line and one record file) and no behavior. A unit test that pinned the record's bytes would freeze a fixture the plan already freezes by committing it.
- Verification, all outside `make test-all` except the first:
  1. `make test-all` passes, with no change to any `tests/e2e/*.sha256` or `.end-state`.
  2. `make pad-author` (all `worst-case/*.pad`) leaves `git status` clean in `worst-case/`: the label is clock-free. This was checked in investigation: `full_game.json` regenerates byte-identical with the label in place.
  3. Cutting is repeatable: running the two cut commands again gives a byte-identical `pico8/demo_record.hex`.
  4. `make state` on the record: build a scratch events file, never committed, from `tests/e2e/pad_resume_at_start.json` with its `initial_save` replaced by the record (48 frames; Up at f10 and cross at f12 pick `resume`). Then `make state EVENTS=<that file> SAVE_HEX=<scratch hex>` must show: clock year 3412 month 1, difficulty reasonable, credits 86, science 12, income 48, techs space folding and geneseeds, count colonies 16, count slipways 18, count techs 2, 49 unknown planets, 9 known planets, cheat activations 0. The scratch hex must equal the record byte for byte (resuming writes nothing). Apart from p46 still vanishing, its planets, slipways and techs match the cut's own state.
  5. The four conditions, rerun from the cut with `pad_author --try` (scratch only). Use the labelled script with `--stop-at demo-record` and a try file holding these lines: `research xenofoods`, `research slipgates`, `fold 118 307; idle 120`, `colonise p32`, `slipway p23 p33`, and `fold 118 307; idle 120; colonise p32; slipway p32 p55; year; research xenofoods`. All must print `ok`. The fold's try state must show fewer unknown planets than 49, and the last line's try state must show year 3413, colonies 17, slipways 19, techs 3.
- Prior art: `tests/e2e/pad_tech_costs.txt` and `tests/e2e/pad_resume_at_start.txt` describe records cut the same way (`make replay … SAVE_HEX=…`) and check them with `make state`.
- `make end-state` and `make full-game` are not required (no core change), and they cannot be affected by a clock-free label beyond what check 2 already proves.

## Out of Scope

- Starting `pad_author` from a record, and the tool that plays a record-based output from the resume (step 5). The `make state` check here goes through the title's `resume` row, which is fine for checking the record's content.
- The demo script, its captions and its pacing (steps 6–7). The beat sequence above only shows that the beats are possible; it is not the demo.
- `pico8/demo.txt` and the provenance text in it (step 7).
- The `attract_after` header member (step 3), compiling the record into the core (step 8), and anything in `src/`.
- Any change to full_game's play, its frames or its end state.

## Further Notes

- Investigation was done on master at `6e5584c`. Step 1 (base check) has not run yet, but it changes no code.
- The record holds a live probe, so in step 5/7 scripts from this record, the "probe" beat is a `fold`: `probe` is refused while a probe is live. Folds must aim at empty space, not at a planet.
- Also seen from this cut, and useful to step 5/7: `colonise p85` then `slipway p85 p5` also works. A fold at (152, 113) reveals 1 planet. The camera stored in the record is at world (43, 104), so the demo's first beat may need a pan before the fold is on screen.
- If a later gameplay change makes the record stop decoding (a save-format version bump), re-cut it with the same two commands from the same label. That is the reason for keeping the label in `full_game.pad`.
