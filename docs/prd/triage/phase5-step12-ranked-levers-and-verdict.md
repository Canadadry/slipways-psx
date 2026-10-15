---
title: "Finish the PSX levers report: step-12 checklist, a lever-gain table, a tool-computed per-frame estimate bracketed measured / measured + estimated, ranked lever entries, the verdict against 263 and the stop condition (plan 20, phase 5, step 12)"
description: "The report holds every lever's evidence spread over 3,500 lines of step sections, with overlaps, discounts and ratio conflicts left to 'step 12', and nothing says per judge frame whether the levers found reach 263, so the human cannot choose which levers to pull."
status: needs-triage
---

## Plan step (verbatim)

> 12. Finish `docs/psx-perf-levers.md`. It opens with a checklist of every
>     item the report and the step notes hand to "step 12" (overlaps such
>     as one-print with no-meter, the draw filter's discount, the meter
>     counted twice, the per-class save and `ui_update` ratios from step 9's
>     fallback, step 10's ratios differing from step 9's, the near misses,
>     candidates' gains on class frames), each resolved by a stated rule
>     with the figure it changes. The per-frame estimate is computed by a
>     tool from one explicit lever-gain table over the whole judge table,
>     and is first checked by reproducing step 9's spreading results (all
>     166 spike frames fit from a uniform 88% draw cut; with the step 7+8
>     estimated draw gains exactly one frame still fails). It is given as a
>     bracket: measured levers only, and measured plus estimated. It holds:
>     - the decisions carried to the implementation plan (section 2 above),
>       verbatim in substance;
>     - one entry per lever: where it is (file:line), the measured or
>       estimated gain in hblanks per short run, the cost to implement, and
>       the risk (e2e hashes, PC/PSX identity, RAM);
>     - the levers ranked by gain over cost;
>     - per frame of the step 5 full-run table (the judge's frames), the
>       cumulative estimate against 263, with the overlaps between levers
>       named (for example, removing the 9× print also shrinks the meter's
>       share). Each lever's short-run delta is carried over as a relative
>       gain on its part (draw, update or submit), since the short runs read
>       low in absolute terms; every frame in a step 9 spike class uses its
>       class's figures and spreading estimate instead;
>     - the spikes and their spreading estimates;
>     - which stop condition ended discovery;
>     - in the verdict, one sentence stating that the bracket is relative to
>       pcsx-redux's timing model (an interpreter charging a flat 2 cycles
>       per instruction, no instruction-cache miss or data wait), and in the
>       Risk field of every lever whose gain rests on what that model does
>       not charge (at least G2 and G3), a label saying so.
>
>     Verified when the checklist items are each resolved with their
>     figure, the tool reproduces step 9's two known results, every lever
>     entry has all four fields, the verdict states its dependence on
>     pcsx-redux's timing model with the affected levers labelled, and the
>     stop condition holds: either the cumulative estimated gain brings every
>     full-run frame to 263 with about 20% extra gain to spare
>     (estimates overlap, so their sum is optimistic), or every branch above
>     2% is explored, whichever came first.

Plan file: `docs/plan-20.md`. Context in force: section 1 (Goal), section 2
(Rules, including "Decisions carried to the implementation plan"), section
4 (Done when), and the step notes through "Step 4.11". Step 11 (phase 4) is
committed (`4306e41`, plan note "Step 4.11"; report section "Global levers",
now the last section); its outcomes are recorded as facts below, `H-11-k`
keeping the drafting numbering.

Rules in force that shape this step:

- **60 fps is strict:** every frame (update, draw and submit) fits in 263
  hblanks; the judge is pcsx-redux at 263, `frame-budget` green in `make
  psx-full-game-check` is the contract. The Miyoo is never measured.
- **The judge's frames are step 5's asserts-off full run**,
  `docs/psx-perf-levers-full-run-frames.txt` (6,059 frames).
- **The only code the plan commits is measuring tooling**, host-tested where
  the repo host-tests its scripts, outside `make test` and `make e2e`.
- **Done when** (section 4): the report is committed, complete per step 12,
  and states whether the levers found reach 263 on paper; `make test-all`
  and `psx-check` green and unchanged; no game or shipped-path code
  changed; every `*.sha256` unchanged; **then the plan stops for the
  human**, who chooses the levers to pull in a new grilling session.

## Problem Statement

The human has to choose which levers to pull, and today the report cannot
tell them:

- **The evidence is scattered.** Twenty-odd draw candidates (step 7's
  `draw_layers` list and candidates 1–15), eight backend candidates (step
  8), U1–U8 (step 9), O1–O5 (step 10) and step 11's global levers each sit in
  their own section, with bounds in different units (window sums, hblanks a
  frame, judge hblanks over a class, share of ordinary update).
- **Seven kinds of question were explicitly left to step 12** and are
  unresolved: the one-print and no-meter experiments overlap; the meter is
  counted twice in step 9's `step7+8` draw gain; the draw filter inflates
  every figure and the judge but is absent from `psx-game`; step 9's
  fallback showed the save and `ui_update` ratios differ per class from the
  two-frame calibration; step 10's ordinary-frame ratios differ from step
  9's; two near misses (`planet_hit_test_method` 16.7, `update_entities`
  16.2) were not explored; O1–O5's gains on class frames are counted
  nowhere. U and O candidates overlap and are "not ranked (step 12's job)".
- **Nothing says, frame by frame, whether 263 is reached.** Step 9's spread
  tool answers it for the 166 spike frames under a uniform draw gain only;
  no tool combines draw, update and submit levers over the 6,059 judge
  frames, and nobody can tell which frames still fail, by how much, with
  measured levers alone or with the estimates too.
- **The report reads as a lab notebook**: it opens "This file will become
  plan 20's ranked inventory", has no ranking, no verdict and no record of
  which stop condition ended discovery, and the locked decisions live only
  in the plan file, which is deleted once the plan ends.

## Solution

At `HEAD` (step 11 committed, `4306e41`), with no change to the game and no new
emulator boot:

1. **A checklist opens the report.** Every item the report and the step
   notes hand to "step 12" (found by a recorded sweep, seeded by the seven
   items above), each with the rule that resolves it and the figure it
   changes.
2. **One lever-gain table**, a committed companion file of the report, holds
   every lever's relative gain per part and scope, its kind (measured or
   estimated), its overlap group, plus the class map, the per-class ratios
   and the spreading permissions. Each row is traceable to a report figure.
3. **An estimate tool**, test-first and host-tested, reads the judge table,
   the step 9 spike frames and the gain table, and prints per judge frame
   the estimate against 263, spreading every spike class's work. It is
   first checked against step 9's two known results (uniform 88% fits all
   166; `step7+8` at 80.7% leaves exactly frame 6043).
4. **The bracket:** the tool run on measured levers only and on measured
   plus estimated, both over all 6,059 frames, with the 20%-to-spare check
   and each lever's gain alone.
5. **The finished report:** lever entries with all four fields, the
   ranking by gain over cost, the decisions carried to the implementation
   plan, the spikes and their spreading, the stop condition, and the verdict
   in its opening paragraph. The plan then stops for the human.

## User Stories

1. As the human choosing levers, I want the report to open with its verdict against 263, so that I know at once whether the levers found are enough on paper.
2. As the human, I want the verdict as a bracket (measured levers only; measured plus estimated), so that I can see how much of the answer rests on estimates.
3. As the human, I want every lever in one numbered list with its location, gain, cost and risk, so that I can choose without reading the step sections.
4. As the human, I want the levers ranked by gain over cost, so that the cheapest large gains come first.
5. As the human, I want each lever's gain over the whole judge table when pulled alone, so that the ranking compares like with like.
6. As the human, I want each lever's gain per short run where one exists, so that I can trace it to the measurement or bound behind it.
7. As the human, I want each lever's risk stated as e2e hashes, PC/PSX identity and RAM, so that I know what a lever would cost the game beyond effort.
8. As the human, I want the decisions carried to the implementation plan restated in the report, so that they survive the plan file's deletion.
9. As the human, I want the frames that still fail named with their excess, so that I see whether the remaining gap is draw, update or spikes.
10. As the human, I want the spike classes and their spreading estimates under the chosen levers, so that I know how many frames each loading or event spike would take.
11. As the human, I want to know which stop condition ended discovery, so that I know whether more discovery could still be worth it.
12. As the plan pilot, I want a checklist of every "step 12" hand-off found by a recorded sweep, so that none is silently dropped.
13. As the plan pilot, I want each checklist item resolved by a stated rule with the figure it changes, so that the resolution is checkable.
14. As the plan pilot, I want the one-print / no-meter overlap resolved by one composition rule, so that the meter is never counted on calls another lever removes.
15. As the plan pilot, I want the draw filter treated as an explicit, separately labelled item, so that its inflation of the judge is neither ignored nor silently subtracted.
16. As the plan pilot, I want class frames to use step 9's per-class fallback ratios, so that the save and `ui_update` work is read as the judge measured it.
17. As the plan pilot, I want ordinary frames to use step 10's ratios and class frames step 9's, so that the two ratio sets never mix on one frame.
18. As the plan pilot, I want the near misses stated with their figure and their effect on the stop condition, so that "not explored by the rule" is visible.
19. As the plan pilot, I want O1–O5 counted on class frames by a stated rule, so that the class frames are not estimated without the ordinary update levers they also run.
20. As the plan pilot, I want U and O overlaps resolved by group, so that two levers on the same work are never summed.
21. As a developer, I want the gain table as one line-oriented committed file, so that the estimate is reproducible from the repository alone.
22. As a developer, I want each gain-table row to name the report figure it comes from, so that a reviewer can check every number.
23. As a developer, I want the step 9 spike frames committed beside the judge table, so that the estimate does not depend on a 31-minute profile in `tmp/`.
24. As a developer, I want the estimate tool to refuse a malformed table naming the line, so that a typo never becomes a figure.
25. As a developer, I want the tool's spike spreading to reproduce step 9's `spread` mode exactly in its independent mode, so that the new tool inherits a checked result.
26. As the plan pilot, I want a known-answer check (`make lever-estimate-check`) that reproduces step 9's two results on the committed files, so that the plan's "the tool reproduces step 9's two known results" is a tool verdict.
27. As the plan pilot, I want the known-answer check to fail loudly when either result is not reproduced, so that a silent drift cannot pass.
28. As the human, I want the final estimate to consume each frame's room only once when several spikes spread into it, so that overlapping spreads are not counted twice.
29. As the human, I want the independent (step 9) allocation shown beside the sequential one, so that I see what the overlaps cost.
30. As the plan pilot, I want the 20%-to-spare condition computed by the tool (every gain cut by a fifth of its value, every frame still at 263 or under), so that stop condition (a) is checked, not argued.
31. As the plan pilot, I want stop condition (b) cited from the report's existing verification tables, so that "every branch above 2% is explored" points at evidence.
32. As the human, I want the per-frame estimate as a companion file and its summary in the report, so that 6,059 lines do not bury the verdict.
33. As the human, I want title frames, frame 12 and in-game frames to use the short run that covers them, so that a lever's gain is carried where it was measured.
34. As the human, I want in-game draw gains taken from the less favourable of the two draw runs, with the other shown as sensitivity, so that the verdict is not optimistic by choice of run.
35. As the plan pilot, I want step 11's levers entered from its committed figures (G2 measured, G4 estimated, G1 and G3 in no end), so that the global levers count once and as measured.
36. As the plan pilot, I want this step to run no emulator boot and no full run, so that it stays a synthesis of measured evidence.
37. As a developer, I want `make test` and `make e2e` untouched in content and duration, so that the fast loop stays fast.
38. As a developer, I want the new tool's tests in `make frame-profile-test`, outside `make test`, so that the plan's tooling rule holds.
39. As the plan pilot, I want the report's opening paragraph rewritten from "will become" to what it is, so that the finished document describes itself truthfully.
40. As the human, I want the companion files named in the opening paragraph and deleted with the report, so that nothing outlives the report.
41. As the plan pilot, I want `make test-all` and `psx-check` run once at the end, green and unchanged, so that the plan's "Done when" is shown, not assumed.
42. As the human, I want the report to say plainly that the plan stops here for my choice, so that no pilot continues into implementation.
43. As the human, I want the verdict to say in one sentence that the bracket is relative to pcsx-redux's timing model (flat 2 cycles per instruction, no instruction-cache miss or data wait), so that I do not read it as a statement about the Miyoo.
44. As the human, I want every lever whose gain rests on what that model does not charge labelled in its Risk field, so that I know which gains real hardware could erode (G2's +40 KB of text, G3's cache argument).
45. As the plan pilot, I want the verification table to list the labelled entries against the rule's selection, so that the plan's new "verified when" item is a count, not a claim.

## Implementation Decisions

### Step 11, as committed (checked on `4306e41`)

The drafting hypotheses H-11-1 to H-11-9 were checked against `git log`,
the report's "Global levers" section and the plan's "Step 4.11" note. All
hold; they are facts, with the rule each one sets for this step. No
fallback remains. Chunk 1 only re-confirms that `HEAD` has not moved past
`4306e41` in a file this step reads (`git diff --stat 4306e41 HEAD`); a
change there is recorded in the checklist and its figures re-read.

- **H-11-1, step 11 committed and last (holds).** `## Global levers` is the
  report's last section, after `## Ordinary update`; the opening paragraph
  names it; the plan's last note is "Step 4.11"; its PRD was removed from
  `docs/prd/triage/`.
- **H-11-2, lever ids (holds).** G1 asserts off, G2 `-O3`, G3 `-Os`, G4
  upload only the texture slots that changed. This step keeps them.
- **H-11-3, asserts already in the baseline (holds).** G1 is "on minus off":
  asserts on cost +3.7% pooled (update +3.2%, draw +3.9%, submit +1.2%;
  full run +4.0% of 857). The judge table is asserts off. *Rule:* G1 enters
  **neither** end of the estimate; it is listed after the ranking as
  "taken: in the baseline; the implementation plan applies it to
  `psx-game`", with its measured gains for the record.
- **H-11-4, levels valid on all six runs (holds).** Game state equal to the
  PC's, effective level as requested, every boot under its ceiling. G2
  `-O3` pooled: total −5.9%, update −13.8%, draw −3.4%, submit −3.1%. G3
  `-Os`: total +25.9% (slower). *Rule:* G2 enters **both ends** as a
  **measured `scale`** lever on all three parts at those pooled relative
  gains (group `opt-level`); the worst run's relative gain per part, read
  from step 11's per-run delta table, is the sensitivity line. G3 enters no
  end; its entry keeps its negative gain.
- **H-11-5, submit split, no piece above 2% (holds).** Submit is 7.45
  hblanks a frame shipped, 76.4% of it `upload` (6.31 a frame pooled, 5.02
  to 7.01 by run); replay-only pieces (filter frame end 0.08, breakdown
  0.02, capture 0.02 a frame) total 0.12. *Rule:* G4 enters the measured +
  estimated end as an **estimated `work`** lever on submit at its upper
  bound (upload's share of each run's submit: `start` for title frames and
  frame 12, the smaller in-game share for in-game frames); the replay-only
  pieces join `F1` (checklist item 3) as judge-harness costs.
- **H-11-6, the meter's frame end is outside step 8's no-meter delta
  (holds).** Step 11 measured it at 0.13 a frame (1.6% of submit) and
  cross-referenced step 8: no-meter kept `p8_draw_meter_frame_end`. *Rule:*
  B1 (drop the meter) does not remove it (the shipped loop calls it; step
  8's lever removes the per-call cost model) and carries no submit gain;
  the 0.13 is named in B1's entry, not counted.
- **H-11-7, nothing the estimate uses changed (holds).** `git diff --stat
  bad0a38 4306e41` touches `Makefile` (the `psx-build` / `psx-short-runs`
  recipes and legend only, no `frame-profile` line), `psx/CMakeLists.txt`,
  `tools/psx_short_run.sh`, `tools/psx_opt_level.sh`, their tests and docs;
  not `tools/frame_profile_check.sh`, its tests or the judge table.
- **H-11-8, `DrawSync` (holds).** 0.04–0.05 hblanks a frame on every run:
  it never waits, the GPU finishes within the CPU's next frame on the
  judge. *Rule:* no figure changes; the verdict says GPU-side cost is
  invisible to the judge's budget, part of the timing-model sentence below.
- **H-11-9, opening paragraph (holds).** It names "Global levers"; this step
  rewrites it anyway.
- **New in step 11: the timing-model caveat.** pcsx-redux runs here as an
  interpreter charging a flat 2 cycles per instruction (`BIAS`), with the
  instruction cache kept as state but never charged, and no data-wait cost
  ("The `-Os` caveat", citing `third_party/pcsx-redux` at `9eb5441`). Plan
  step 12 now requires (a) one sentence in the verdict saying the bracket
  is relative to that model, and (b) a label in the Risk field of every
  lever whose gain rests on what the model does not charge. See "The
  timing-model label" below.

### The timing-model label

- **Label text**, identical in every entry that carries it, at the head of
  its Risk field: **"Timing model: rests on pcsx-redux charging no
  instruction-cache miss or data wait"**, followed by one clause saying
  which way the real cost would likely move.
- **Which levers carry it** (rule, applied to every entry and recorded in the
  checklist as an item): a lever whose gain comes from, or whose cost
  includes, (1) code size or layout (G2: +40 KB text, real cache misses
  would eat part of its −5.9%; G3: its only argument, the cache, is
  invisible here), (2) trading instructions for memory traffic or larger
  tables and caches (B5's per-glyph table, B8's
  radius table, U4's 1 KB CRC table, D2/D3/D5/D6's off-screen images and
  pre-built background copied from VRAM, L1's kept sorted lists), or (3)
  GPU-side work the judge does not charge to the frame (`DrawSync` never
  waits: any lever that adds primitives or VRAM copies, D2/D3/D5's copies,
  D1/B4's larger glyph sprites). At least G2 and G3 carry it; the sweep of
  the entries in chunk 4 decides the rest by this rule and lists them in the
  checklist item. A lever that only removes instructions and calls (B1,
  D9–D13, O1–O3, U1–U3) does not carry it.
- **The verdict sentence** (one sentence, in "Verdict and stop condition"
  and echoed in the opening paragraph): the bracket is relative to
  pcsx-redux's timing model — an interpreter charging a flat 2 cycles per
  instruction, with no instruction-cache miss or data wait — so it is a
  statement about the judge, not about real hardware or the Miyoo, and the
  labelled levers are the ones most exposed to that difference.
- **Verification:** the step 12 verification table gains a row for this
  item: the sentence quoted, and the list of labelled entries with a count
  (entries labelled / entries that the rule selects, equal).

### The checklist ("Step 12 checklist", first section of the report)

- **The sweep that builds it** is recorded in the section (one command): a
  case-insensitive search of `docs/psx-perf-levers.md` and of the plan's
  step notes for `step 12`, `not ranked`, `left to`, `settles`, `flagged`,
  `discount`, `near miss`, plus the "Global levers" section's own
  hand-offs. Every hit that hands something forward becomes an item; a hit
  that does not is listed once as "not a hand-off" with its line.
- **Each item** has: where it was raised (section, line), the question, the
  **rule** that resolves it, and **the figure it changes** (before → after,
  or "no figure changes" with why). Figures that the estimate tool computes
  are filled in chunk 3; until then the item says "pending chunk 3".
- **Seed items and their rules** (decided here; the sweep may add more, each
  resolved in the same spirit and recorded):
  1. **One-print with no-meter (step 8, "Overlap").** Rule: levers that
     remove calls or work are the `work` layer; levers that shrink every
     remaining call (the meter, the filter, a compiler level) are the
     `per-call` or `scale` layer, applied to what the work layer leaves:
     `part' = part × (1 − g_work) × (1 − g_per-call) × (1 − g_scale)`. The
     meter's gain is therefore never taken on calls the one-print removes.
     Figure: the measured pair on `draw-5564`, 41.7% (one-print) and 16.3%
     (no-meter) of window draw, combine to 51.2%, not 58.0%; on `draw-6000`
     11.0% and 21.3% → 30.0%; on `start` 13.8% and 26.7% → 36.8% (the
     values step 9 already used; the item records them as the rule's
     output). Cross-check in the item: re-pricing the meter at its measured
     0.172 hblanks per remaining backend call on `draw-5564` (17769 − 3944
     calls) gives its share after one-print; the item states the difference
     against the proportional rule, and the proportional rule stays.
  2. **The meter counted twice (step 9, "Spreading": `step7+8`).** Rule: the
     same composition; the meter is `per-call`, every other draw lever
     `work`. Figure: the `step7+8` in-game gains (80.7%, 88.8%) are the
     rule's output and stay; the item says so and that the double count step
     9 flagged is the meter's share of calls the work levers remove, which
     the multiplicative form already excludes up to the proportionality
     assumption (cost ∝ draw time); the item 1 cross-check bounds that
     assumption.
  3. **The draw filter's discount (step 8 finding).** The judge's measured
     replay runs `draw_filter_install`, `psx-game` does not. The judge is the
     contract, so the filter is **not** silently subtracted. Rule: it is a
     separate lever `F1` "time the judge's replay without the draw filter"
     (a harness change: the filter only serves the capture masks), kind
     **estimated**, `per-call` layer on draw, gain equal to step 8's measured
     no-meter gain per run as an upper bound (step 8: "likely of the order
     of, and below"); it enters the measured + estimated end only, and its
     entry flags that it changes the judge's harness and needs the human's
     agreement. Replay-only submit pieces from step 11 (H-11-5) join `F1`.
     Figure: the measured + estimated end with and without `F1`, both
     printed.
  4. **Per-class save and `ui_update` ratios (step 9's fallback).** Rule:
     on class frames, the class's ratio is the fallback table's per-class
     median for the class's `cart_update60` child (`ui_update` or
     `save_at_frame_end`; C4 `ui_update` 1.01, C6 1.00, C19 none → 1), in
     place of step 9's two-frame 0.96 / 1.11. C13 uses its measured path
     (`cart_update60/save_at_frame_end`) and measured shares (50.3%, 58.4%,
     59.8%) with ratio 1, as step 9's C13 re-spread did. The known-answer
     check keeps step 9's original inputs. Figure: each class's work
     min/median/max before → after, from the tool.
  5. **Step 10's ratios differing from step 9's.** Rule: ordinary frames use
     step 10's (`input_step_run` 1.13, `update_entities` 1.28, `ui_update`
     0.83), class frames step 9's per-class ratios (item 4); never both on
     one frame. Step 10's O-lever figures are already judge-calibrated with
     its ratios, so they are carried as they are. Figure: none changes (the
     item states which figures already use which set).
  6. **The near misses (step 10).** `planet_hit_test_method` 16.7 (0.1 under
     16.8) is O3's node, so it is covered; `update_entities` 16.2 ("low"
     label, p90 43.6, above at its `start-game` ratio) has no candidate.
     Rule: a near miss enters no estimate; it is named in the stop-condition
     section as the first place to look if discovery is reopened. Figure:
     none; the item records the update share it holds (20.6% of ordinary
     update).
  7. **Candidates' gains on class frames (step 10, "Cross-reference").**
     Class frames run `pad_nav_step` and `ui_update_hover` too. Rule: O
     levers apply on a class frame as their ordinary-population absolute
     gain (O1 28.2, O3 14.4, … hblanks a frame), subtracted from the frame's
     update **outside** the class's spreadable work, never below 0; they are
     not relative gains there (36% of a 1,400-hblank update would be
     absurd). Draw and submit levers apply to class frames exactly as to
     ordinary frames. Figure: class frames' update after levers, from the
     tool.
  8. **U candidates overlapping and unranked (step 9).** Groups: U1 and U2
     act on different parts of a recompute → both `work`, different groups;
     U2 ⊂ U3 on C4 → one group, alternatives; U4 (shrinks the save) and U5
     (moves the save) → U4 as a `work` gain, U5 as a spreading permission
     on what is left; U6 and U7 on frame 12 → U6 `work`, U7 spreading of the
     rest; U8 is C19's spreading. Gains per class: the candidate's judge
     bound over the class's frames ÷ the class's summed judge update (step
     9's tables), U4 at 7/8 of its bound (a 256-entry table keeps about 1/8),
     U6 at its "about half" figure (3,980), not its 7,963 ceiling. Figure:
     each class's relative update gain, from the table.
  9. **O candidates overlapping (step 10).** O1 and O2 → one group,
     alternatives (O1 36.0% vs O2 31.9% of ordinary update: O1 counts). O3,
     O4, O5 → one group: alternative {O3 at 14.4 (its 16% overstatement
     removed), O5} against {O4}; O4's share of unchanged frames is not
     measured, so O4 enters at 0. Figure: ordinary update's relative gain.
  10. **`draw_layers#1`'s flagged self and the open selves (step 7).** Rule:
      they hold no candidate beyond DL1–DL2 and candidate 15; they are
      named in the stop-condition section as unexplored selves inside
      explored nodes. Figure: none.
  11. **Step 11's items** (H-11-3 to H-11-8: G1 in neither end, G2 as a
      measured scale lever, G3 in no end, G4 estimated on submit, the
      meter's frame end not counted, `DrawSync` never waiting), each with
      its rule above and its figure.
  12. **The timing-model label** (plan step text, step 11's caveat): the
      rule of "The timing-model label"; figure: the list of labelled
      entries (at least G2, G3) and the verdict sentence. No estimate
      figure changes: the label qualifies gains, it does not cut them.

### Lever ids

The report's own ids are kept where they exist: `U1–U8`, `O1–O5`, step 11's
`G<n>` (H-11-2). The others get ids in this step, written once in the
entries and the table: `L1–L4` for step 7's `draw_layers` candidates 1–4,
`D1–D15` for step 7's candidates 1–15 below level 2 (D9 replaces L4 for
planets, as step 7 corrected), `B1–B8` for step 8's backend candidates
(B1 the meter, B4 the outlined font), `F1` the draw filter (checklist
item 3). D1 and B4 are the same lever (the outlined font) seen from the tree
and from the backend: one entry, `D1/B4`, whose measured figure is step 8's
one-print upper bound.

### The lever-gain table (companion file `docs/psx-perf-levers-gains.txt`)

Line-oriented, whitespace-separated, `#` comments, one row a line. It is
the single input the estimate reads; every `lever` row ends with a comment
naming the report figure it comes from (section and value). Row kinds:

- `budget <hblanks>` — 263.
- `range <name> <first>-<last>` — named frame ranges: `title` 0–11,
  `start` 12–12 (both from the `start` run); everything else is in-game.
- `class <label> <path>` — maps a spikes path to its report class (`C1` …
  `C19`), one row per class (19 rows).
- `override <frame> <path> <share%>` — replaces a spike frame's path and
  share (C13's three frames, checklist item 4); the frame keeps the label
  of its original path.
- `ratio <scope> <child>=<rho>[,…]` — scope `default` or a class label; the
  class's `cart_update60` child is read from the path as `spread` does
  (`#n` suffix stripped); a class scope wins over `default`; a child with
  no ratio gets 1.
- `lever <id> <kind> <part> <layer> <group>/<alt> <scope> <gain%>` —
  `kind` `measured|estimated`; `part` `draw|update|submit`; `layer`
  `work|per-call|scale`; `scope` `all|in-game|ordinary|class:<label>|range:<name>`;
  `gain%` relative to the part on that frame, 0–100. A lever may have
  several rows with different scopes; for a frame, the most specific
  matching scope wins (`range` > `class` > `ordinary` > `in-game` > `all`).
  A lever whose gain is absolute on class frames (checklist item 7) uses
  `abs:<hblanks>` in place of `gain%`, allowed on `update` with a `class:`
  or `class:*` scope only.
- `spread <id> <kind> <class-label|*> responsible|responsible+save:<share%>`
  — permission to spread a class's work over the following frames: the
  responsible work, or (U5) also the save's portion of the update at the
  class's measured PSX save share (step 9's fallback table). The generic
  "spread candidate" every class has is `spread S <kind> * responsible`.
  Spreading is a permission, not a gain: it is allowed in both ends (the
  plan decided spreading is allowed; the room it uses is the judge's own
  frames). Its kind is recorded for the entry only.

Composition, per frame and part (this is the estimate's definition; the
tool implements it, the report states it once):

- Inside a layer: rows summed within an `alt`, the largest `alt` of a
  `group` taken, groups summed, capped at 100%.
- Across layers: `part' = part × (1 − g_work)(1 − g_per-call)(1 − g_scale)`,
  rounded half up to whole hblanks per part, as `spread` rounds draw.
- End `measured` reads `measured` rows only; end `estimated` reads all rows.

How the rows are filled (chunk 1, recorded per row):

- **Draw, per run:** a lever's window bound or measured delta ÷ the run's
  window draw (probe baseline: `start` 465, `draw-5564` 19491, `draw-6000`
  13987), by step 9's "Draw gains" rules (pixel risk "identical" or
  "identical if/only …"; "less a copy" ignored; D7 dropped as covered; D9
  counts its dispatch; D11 its batched variant; D14 0; text group: one
  group, alternatives {D1/B4} and {D2, D3, D4}), extended with L1 (L2 an
  alternative inside it), B2, B3 (B3 an alternative to D5 in the map
  group), F1 and the per-call/scale layers. B5–B8 are "not separable"
  (step 8): they enter at 0, their loose ceilings in their entries only.
- **Run → frames:** `range:title` and `range:start` take the `start` run's
  gain; `in-game` is computed once per draw run, each with that run's
  whole lever set (`draw-5564`'s gains together, then `draw-6000`'s
  together), and the less favourable run's result is the headline, the
  other the sensitivity table (`ESTIMATE_GAINS` alternative file, see
  below). Never a per-lever minimum across the two runs: it would mix two
  screens (the message box only exists on `draw-5564`) and drop gains
  both screens do have. The
  one-frame update runs are cross-checks shown in the entries, not inputs.
- **Update, ordinary frames:** O levers as their relative share of
  ordinary update (step 10's table, after checklist item 9); scope
  `ordinary`.
- **Update, class frames:** U levers as `class:<label>` relative gains
  (checklist item 8); O levers as `abs:` on `class:*` (checklist item 7).
- **Submit:** G4, estimated `work` on submit at upload's share of the
  run's submit (H-11-5).
- **Global:** G2, measured `scale` on update, draw and submit at its pooled
  −13.8% / −3.4% / −3.1% (H-11-4); G3 and G1 have no row.
- **Cross-check, recorded in the checklist:** with only step 9's levers
  (the text group, D5, D6, D8–D13, D15, and B1 for `step7+8`), the table's
  composition reproduces step 9's run gains (51.8% / 86.6% / 75.5% for
  `step7`, 74.8% / 88.8% / 80.7% for `step7+8`) within 0.1 point. A larger
  gap is a table error, fixed before chunk 3.

### The step 9 spike frames (companion file `docs/psx-perf-levers-spike-frames.txt`)

`tmp/frame-profile/spikes/spikes.txt` (step 9's K = 8 classification),
byte for byte, committed so the estimate and its check run from the
repository alone. *Check before copying:* 166 `frame` lines, and each
`class` line's frame `list` equal to the report's "Frames per class" list
of the class with the same path (C1 … C19). *Fallback:* missing or
different → `make frame-profile-spikes FRAME_PROFILE_REPEATS=8` (about 31
minutes, PC only), then the same check; if the classes still differ from
the report's, stop and report it (the report's classes are the reference
and cannot be silently replaced). It goes when the report goes, as the
judge table does.

### The estimate tool (`tools/lever_estimate.sh`)

A new POSIX sh + awk script, apart from `tools/frame_profile_check.sh` (1,140
lines, already flagged by the phase 3 warn-review). Interface:

```
sh tools/lever_estimate.sh <judge-table> <spike-frames> <gain-table> <end> [<option>=<value>...]
  end: measured | estimated
  options: allocation=sequential|independent (default sequential)
           haircut=<pct> (default 0: every gain multiplied by 1 / (1 + pct/100))
           alone=<lever-id> (only that lever's rows, spreading kept)
```

Exit 1 on a usage error (missing file, bad end or option), 2 on a malformed
input (naming the file and line), 0 otherwise, whatever the verdict (the
verdict is in the output, as `spread` does).

Per frame `f` of the judge table:

- `update'`, `draw'`, `submit'` by the composition above.
- **Not a spike frame** (not in the spike file): `total' = update' + draw' +
  submit'`, `fits` when `total' ≤ budget`, otherwise `over <excess>`.
- **A spike frame:** its class from its path (after `override`); work
  `W = ceil(share × ρ × update)` exactly as `spread` (ρ by class scope then
  `default`, portion capped at 1); update levers of `work` kind shrink it
  proportionally, `W' = ceil(W × update'' / update)` where `update''` is the
  update after its `work` and `scale` gains before the `abs:` gains; the
  `abs:` gains then come off the non-spreadable rest, floored at 0; the
  spreadable amount is `W'`, plus the save portion when a matching
  `responsible+save` row exists (capped at the update left). The frame's
  own room and the following frames' room take the spreadable amount, as
  `spread` does, up to 600 frames and the table's end, else
  `does-not-fit`.
- **Allocation.** `independent`: each spike frame computes its room alone,
  following frames counted at their own `total'` with their full update
  (step 9's `spread` behaviour, kept exactly for the known-answer check).
  `sequential` (the report's estimate): spike frames in increasing order
  draw on one shared room array, initialised to `max(0, budget − total')`
  for ordinary frames and `max(0, budget − (total' − spreadable))` for
  spike frames; work allocated to a frame consumes its room, so two spreads
  never count the same room.

Output, one line a frame then summaries:

```
frame <f> before <t> after <t'> update <u'> draw <d'> submit <s'> fits|over <x>
frame <f> before <t> class <label> work <w> spread <a> frames <n>|does-not-fit [overlaps <f>,...]
class <label> work <min>-<median>-<max> frames <min>-<median>-<max>|- does-not-fit <k>
summary end <end> allocation <a> haircut <h> frames <n> over <k> worst <f> <t'> mean <m> spikes <s> does-not-fit <d>
lever <id> alone saved <hblanks> frames-fitting <n>      (one per lever, only without alone=)
```

`lever … alone` reruns the estimate with that lever's rows only and gives
the judge-wide hblanks saved (Σ before − Σ after over ordinary frames, plus
the update removed on spike frames) and the frames that then fit: the
ranking's gain.

Make targets (legend entries added at the top of the `Makefile`, next to the
`frame-profile-*` ones; both never boot, outside `make test`, `make
test-all`, `make e2e`):

- `make lever-estimate [ESTIMATE_END=measured|estimated] [ESTIMATE_ALLOCATION=…]
  [ESTIMATE_HAIRCUT=<pct>] [ESTIMATE_GAINS=<file>] [ESTIMATE_LABEL=<label>]`
  → `tmp/lever-estimate/<label>.txt` (label default `<end>`), gains default
  `docs/psx-perf-levers-gains.txt`, spike frames
  `docs/psx-perf-levers-spike-frames.txt`, judge
  `$(FRAME_PROFILE_JUDGE_FRAMES)`.
- `make lever-estimate-check` → the known-answer check below; fails (exit 1)
  naming the result not reproduced.

### The known-answer check (`make lever-estimate-check`)

Inputs: the committed judge table and spike frames, and three small committed
gain tables under `tests/fixtures/lever_estimate/` (`uniform-88`,
`uniform-87`, `step78-low`) holding only step 9's
inputs (`class` rows, `ratio default ui_update=0.96,save_at_frame_end=1.11`,
`spread S measured * responsible`, no `override`) and one draw lever:

1. `uniform-88`: `lever K measured draw work k/k all 88` → `independent`
   allocation: `does-not-fit 0` over the 166 spike frames; `uniform-87`
   (the same at 87) → at least one does-not-fit, frame 6043 among them (step 9: "all
   166 from 88% (frame 6043 last)").
2. `step78-low`: draw `in-game` 80.7, `range:start` 74.8 (the frames before
   12 take the in-game gain, as step 9's `default=80.7,12-12=74.8` did) →
   `does-not-fit 1`, the frame 6043 (C15); and every class line's work and
   frames triples equal to step 9's `step7+8, gain 80.7%` column of the
   spreading table (C13 its "PC, superseded" row).

The check also runs `tools/frame_profile_check.sh spread` on the same judge
and spike files with `default=88` and `default=80.7,12-12=74.8` and step 9's
ratios, and requires the estimate tool's spike lines, in `independent`
allocation, to equal `spread`'s frame and class lines token for token
(`frames`, `does-not-fit`, `overlaps`). The expected step 9 figures are
written in the check, not read from the report.

### The bracket and the companion estimate (chunk 3)

- Runs, all on the committed table: `measured` and `estimated` (sequential:
  the bracket); both again `independent` (the overlaps' cost); `estimated`
  with `haircut=20` (stop condition (a)); `estimated` with `F1`'s rows
  removed (checklist item 3); `estimated` on the sensitivity table (in-game
  draw gains from the more favourable run).
- **Companion file `docs/psx-perf-levers-estimate.txt`**: the `measured` and
  `estimated` sequential outputs, concatenated with a header line each,
  byte for byte from the tool. It goes when the report goes.
- **Report section "Cumulative estimate"** (summary, not the 6,059 lines):
  the commands; the composition rule; a table per end: frames over 263,
  worst frames (ten, with their parts after), mean frame, distribution of
  the excess (≤ 1/4, ≤ 1/2, ≤ 1, > 1 vsync, as the judge's summary), the
  part dominating the excess; spike frames' does-not-fit; the independent
  vs sequential difference; the `haircut=20` result; with and without `F1`;
  the sensitivity line. Every overlap rule applied is named once with the
  levers it joins (for example "D1/B4 removes calls → B1's per-call gain
  applies to what is left").
- **Report section "Spikes and spreading"**: per class (C1–C19), label,
  responsible function, work before → after levers, frames under each end,
  does-not-fit; the U levers per class with the relative gains used; the
  four named frames.

### The finished report (chunk 4)

New top of `docs/psx-perf-levers.md`, in this order, before the existing
sections, which stay as the evidence:

1. **Opening paragraph**, rewritten: what the file is (plan 20's ranked
   inventory), the verdict in two sentences (bracket: measured levers only →
   frames over 263; measured + estimated → frames over 263, and whether
   263 is reached on paper with 20% to spare), the companion files (judge
   table, spike frames, gain table, estimate), and that the report and its
   companions are deleted once the implementation plan exists.
2. **Step 12 checklist.**
3. **Decisions carried to the implementation plan**: section 2's three
   locked decisions (spreading allowed on PC and PSX alike, e2e re-recorded
   once; pixel-identical picture with the `visual_random` exception; asserts
   off in `psx-game` and the measured replay, on in PC and tests), verbatim
   in substance, plus the locked 60 fps definition the implementation plan
   inherits (strict, every frame; the judge at 263 with no margin; the
   Miyoo never measured, said plainly in the README and
   `docs/psx-game.md`; no PSX CPU clock option).
4. **Verdict and stop condition**: the bracket's figures; stop condition
   (a) — the `haircut=20` run fits every frame or not; stop condition (b) —
   every branch above 2% explored, citing "Verification across runs" (draw),
   step 9's and step 10's verification tables (update), step 11's (global
   and submit); which came first (b held at the end of step 10, before any
   estimate existed, unless (a) holds; if both hold the report says (b)
   came first and (a) holds too); the near misses and open selves
   (checklist items 6 and 10) as where to look if discovery is reopened;
   the timing-model sentence ("The timing-model label": the bracket is
   relative to pcsx-redux's flat 2 cycles per instruction with no
   instruction-cache miss or data wait, and `DrawSync` never waits, so
   GPU-side cost is invisible to it); and one sentence: **the plan stops
   here for the human's choice**.
5. **Levers ranked**: one table, every lever, columns id, name, kind,
   judge-wide gain alone (estimated end, from `lever … alone`), frames it
   alone brings under 263, cost, gain ÷ cost, risk summary. Gain ÷ cost:
   judge hblanks saved alone ÷ cost points (S 1, M 2, L 4); ties broken by
   lower risk (no re-record before re-record). Levers already taken (the
   asserts) and levers at 0 (not separable, O4) are listed after the
   ranking, with why.
6. **Lever entries**: one per lever, with the four fields, each mandatory:
   - **Where:** `file:line` at `HEAD` (re-checked, since step sections
     cite older commits), or the CMake option and targets for a level.
   - **Gain:** measured or estimated (labelled), in hblanks per short run
     (window sum, per frame in brackets) for each of the six runs where the
     report has a figure, `—` with the reason where it has none (U levers:
     judge hblanks over their class frames instead, and `start` /
     `update-5500` where those runs hold the class frame); plus the gain-table
     rows it contributes.
   - **Cost:** S (one PRD chunk, local change), M (one PRD, several
     modules or a new data structure), L (several PRDs, a new subsystem or
     asset pipeline, or a harness/judge change needing the human's
     agreement), with one sentence saying why.
   - **Risk:** e2e hashes (unchanged / `visual_random` only, checked per
     section 2's exception / re-record), PC/PSX identity (both change alike
     / PSX only, which section 2 rejects for game behaviour), RAM (bytes or
     VRAM area, or "none"), plus pixel risk where step 7 gave one; the
     timing-model label first, where "The timing-model label" selects the
     lever (G2 and G3 at least).
7. **Cumulative estimate**, 8. **Spikes and spreading** (chunk 3).
9. **Step 12 verification table**, one row per "verified when" item: the
   checklist resolved with figures; the tool reproduces step 9's two
   results (`make lever-estimate-check` output); every entry has its four
   fields (a count: entries, and entries with each field); the verdict's
   timing-model sentence quoted and the labelled entries listed (labelled
   = selected by the rule, G2 and G3 among them); the stop condition that
   holds, with its evidence.

The existing sections are kept unchanged except: the stale "for now"
wording of the opening, and a one-line pointer at the head of "Update
spikes" → "Spikes and spreading" for the final figures.

### Not decided by a guideline call

Every question was settled from the plan, the committed report and the code:

- no new emulator boot or full run: the step is a synthesis; the draw
  filter stays estimated, as step 8 left it and the plan's "step 12 and the
  human should discount it" allows;
- the judge is not discounted for the filter; the filter becomes a
  labelled lever (`F1`) because the judge, filter included, is the
  contract;
- in-game gains from the less favourable draw run, the other as
  sensitivity: the verdict should not depend on picking the kinder run;
- sequential allocation for the verdict, independent for the check: step
  9 flagged the overlaps; the check must reproduce step 9 exactly;
- spreading allowed in both ends: it is a decided permission, its only
  estimate (work divisible at any grain) is stated;
- a separate tool rather than a ninth mode in `frame_profile_check.sh`, per
  the warn-review on that script's size;
- the asserts lever in neither end: the judge table is already asserts off;
- `-O3` as a measured lever in both ends despite the timing-model caveat:
  it is measured on the judge, the bracket is the judge's, and the label
  carries the caveat rather than a cut in its gain.

No `guideline` agent was needed.

## Implementation Chunks

1. **Pre-checks, checklist, companions, gain table.** Confirm `HEAD` has
   not moved past `4306e41` in a file this step reads, and record step 11's
   facts (H-11-1 to H-11-9) as checklist item 11. Copy and check the spike frames
   companion. Run the sweep, write the "Step 12 checklist" section with its
   rules (tool figures "pending chunk 3"). Write
   `docs/psx-perf-levers-gains.txt` with every row and its source comment;
   do the step 9 cross-check by hand or with a throwaway awk (recorded,
   not committed). Commit the docs.
2. **The estimate tool, test-first.** `tests/test_lever_estimate.c` first
   (added to `FRAME_PROFILE_TESTS`), then `tools/lever_estimate.sh`, the
   three fixtures under `tests/fixtures/lever_estimate/`, `make
   lever-estimate`, `make lever-estimate-check`, legend entries. Green:
   `make frame-profile-test` and `make lever-estimate-check` on the
   committed files. If the check fails, the tool is wrong (step 9's figures
   are the reference): fix the tool, never the expected figures. Commit.
3. **The bracket.** Run the seven estimate runs of "The bracket", write
   `docs/psx-perf-levers-estimate.txt`, the "Cumulative estimate" and
   "Spikes and spreading" sections, and fill every "pending chunk 3" figure
   of the checklist. Commit the docs.
4. **Entries, ranking, verdict, stop condition.** The `lever … alone` lines
   feed the ranking; write the entries (re-checking every `file:line` at
   `HEAD`), the ranking, the decisions carried, the verdict and stop
   condition with the timing-model sentence, the timing-model label in
   every entry the rule selects (listed in checklist item 12), the
   rewritten opening paragraph and the verification table.
   Run `make test-all` and `make psx-check` once: green and unchanged; `git
   diff --stat` over the plan's commits shows only tooling, tests, `Makefile`
   and docs. Commit. The plan then stops for the human.

## Testing Decisions

- A good test checks the tool's observable contract — exit status, the
  lines it writes, the verdict per frame — on small hand-written judge
  tables, spike files and gain tables, never its internals.
- **Tested, in `tests/test_lever_estimate.c`, run by `make
  frame-profile-test`** (outside `make test`, `make test-all`, `make e2e`):
  - usage errors exit 1 (missing file, unknown end, bad option);
    malformed gain-table rows (unknown kind, part, layer or scope; gain
    above 100; `abs:` outside an update class scope; a `class` label used
    twice; an `override` frame not in the spike file) exit 2 naming the
    line;
  - a frame with no matching lever is unchanged, `fits` at the budget and
    `over 1` one hblank above it;
  - inside a layer: rows summed within an alternative, the largest
    alternative of a group taken, groups summed, capped at 100;
  - across layers: work then per-call then scale, multiplicatively, rounded
    half up per part (a fixture where additive and multiplicative differ by
    several hblanks);
  - scope precedence (`range` over `class` over `ordinary` over `in-game`
    over `all`) and the `measured` end ignoring `estimated` rows;
  - spike work `ceil(share × ρ × update)`, the class ratio winning over
    `default`, the portion capped at 1, `override` replacing path and share
    but keeping the class label;
  - an update `work` gain shrinking the spreadable work proportionally, an
    `abs:` gain coming off the rest only and floored at 0, a
    `responsible+save` row adding the save portion;
  - `independent` allocation equal to `tools/frame_profile_check.sh spread`
    line for line on the same fixture (the test runs both scripts);
  - `sequential` allocation consuming room: two spikes sharing following
    frames need more frames than under `independent`;
  - does-not-fit past 600 frames and at the table's end;
  - `haircut=20` dividing every gain by 1.2 (a frame that fits at 0 and not
    at 20);
  - `alone=<id>` and the `lever … alone` lines (saved hblanks and frames
    fitting on a fixture computed by hand);
  - the `summary` line's counts.
  Prior art: `tests/test_frame_profile_check.c` (hand-written judge and
  spike fixtures, `run_spread`), `tests/test_psx_probe.c`.
- **Checked on real data, not unit-tested:** `make lever-estimate-check`
  (step 9's two results, and token equality with `spread`), the gain
  table's step 9 cross-check (chunk 1), the report's figures. The Makefile
  recipes are not tested (prior art: the `frame-profile-*` targets).
- `make test` and `make e2e` are untouched in content and duration; no
  `*.sha256` moves; `make test-all` and `psx-check` green and unchanged at
  the end.

## Out of Scope

- Implementing any lever, or any change to the game, engine, runtime, PSX
  code or shipped path; re-recording e2e.
- Any emulator boot, short run, probe or full run; measuring the draw
  filter, O4's unchanged-frame share, or any not-separable backend
  candidate.
- Step 11's own work (its measurements, its section, its tooling), and
  re-deriving any figure an earlier step committed, except where a
  checklist rule changes it with the figure stated.
- Refactoring `tools/frame_profile_check.sh` or the Makefile smells the
  warn-reviews listed.
- Writing the implementation plan, or editing `docs/plan-20.md` (plan-pilot
  writes the step note); deleting the report (done after the
  implementation plan exists).
- Anything on the Miyoo or PCSX-ReARMed, audio, launching the raylib app.

## Further Notes

- Expected outcome, for orientation only: step 9 found that with no draw
  lever no spike frame fits, and step 8's measured draw gains alone (36.8%
  `start`, 51.2% `draw-5564`, 30.0% `draw-6000`) leave all 166 spike frames
  does-not-fit; draw's median judge frame is 717 hblanks, so the measured
  end will likely leave nearly every frame over 263, and the verdict rests
  on the estimated end. Stop condition (b) is expected to be the one that
  held. G2's measured −3.4% on draw and −13.8% on update change that
  little; G4 is at most about 6 hblanks a frame.
- Step 9's sweep: none fits below 64% uniform draw cut, 165 at 80%, all 166
  from 88%. The in-game conservative gain chosen here (the smaller draw
  run's) is near step 9's `-low` figures, which left exactly frame 6043
  (16 frames before the run's end, end-of-game screen) does-not-fit; under
  sequential allocation more frames may fail where spreads overlap (step 9
  listed 13 to 92 overlaps by scenario).
- The end-of-game screen (frames near 6043, draw 947–1235) is covered by no
  short run; it takes the in-game gains, and the verdict names it as the
  least supported region.
- The spike frames companion, the gain table and the estimate are, like
  the judge table, part of the report: the opening paragraph names them and
  they are deleted with it.
- `FRAME_PROFILE_JUDGE_FRAMES` already reads a docs file that goes with the
  report (phase 3 warn-review); the new targets follow the same pattern
  deliberately, for the same lifetime.
