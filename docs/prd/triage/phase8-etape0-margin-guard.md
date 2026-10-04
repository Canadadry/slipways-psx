---
title: "Margin guard for make state"
description: "make state only fails when a replay stops, so a peak can eat the whole 1/8 margin plan 7 left under each zone and cap without anyone noticing."
status: needs-triage
---

## Problem Statement

Plan 7 sized every memory zone and every slot cap on the peak measured by the `make state` campaign, plus 1/8. Three sizes are proven ceilings instead and have no margin: the `button` slot size and the `row` slot size (each proven by a largest-case unit test) and the `particles_help_screen` singleton. The sum of the zones sits 1,980 bytes under `ZONE_BUDGET_BYTES`, the PSX ceiling.

Today `make state` fails only when a replay stops (a `p8_fatal`, a full zone or cap). A change in plan 8 (the button-to-mouse layer, keyboard e2e, a new worst-case game from the human trial) can push a peak from "measured + 1/8" up to "just under the limit". Every check stays green, and the margin is gone before plan 11 measures on MIPS. Plan 8's invariant says "`make state` (with the margin) passes", but nothing enforces the margin yet.

Nothing prints the space left under `ZONE_BUDGET_BYTES` either. Today it is checked only by a compile-time assert, and the number lives in a commit message.

## Solution

`make state` (the campaign, without `EVENTS`) judges each line of its peak table:

- Each zone or cap passes when its peak plus 1/8 of that peak fits within its limit. This is plan 7's sizing rule read backwards: in integers, `9 × peak ≤ 8 × limit`.
- A proven ceiling (`bytes_button`, `bytes_row`, `particles_help_screen`) passes when `peak ≤ limit`.
- A slot whose list is reserved in full when it is created (`bytes_particles`, `bytes_particles_help_screen`, `bytes_ephemerals`) also passes when `peak ≤ limit`. Its margin is judged on its `elements_*` line.

Every line that fails is named on stderr: kind, name, peak, the most the margin allows, limit, and the replay that holds the peak. The table is still written and `make state` exits non-zero. Every run also prints `state: <n> bytes left under ZONE_BUDGET_BYTES` (1,980 today). The 1,980 figure has been checked against the code: 786,432 − (20,256 + 203,961 + 73,035 + 462,520 + 24,680) = 1,980.

## User Stories

1. As a developer on plan 8, I want `make state` to fail when a peak eats its 1/8 margin, so that I learn about it in the commit that causes it, not in plan 11.
2. As a developer, I want each failing line named with its peak, the most the margin allows, its limit and the replay that holds it, so that I know which zone to look at and which replay reproduces it.
3. As a developer, I want every failing line listed, not only the first, so that one run shows the whole picture.
4. As a developer, I want the peak table written even when the guard fails, so that I can read the other lines.
5. As a developer, I want a stopped replay and a margin failure reported separately, so that I can tell a crash from a squeezed margin.
6. As a developer, I want the proven ceilings (`bytes_button`, `bytes_row`, `particles_help_screen`) judged against their limit alone, so that a ceiling proven by a largest-case test does not fail the guard by construction.
7. As a developer, I want the slots of fully reserved lists judged against their limit alone, so that a slot that is always exactly full does not fail the guard, while its element count still carries the margin.
8. As a developer, I want the margin computed in integers with exactly plan 7's rule, so that a size set to "peak × 9/8, rounded up" passes, and a peak one unit higher fails.
9. As a developer, I want `make state` to print the space left under `ZONE_BUDGET_BYTES`, so that I see how much room the next zone resize has without opening a commit message.
10. As a developer, I want that number computed from the same zone sizes the compile-time budget assert uses, so that it can never disagree with the build.
11. As a developer, I want `make state EVENTS=<file>` unchanged, so that inspecting one replay never fails on a campaign rule.
12. As a developer, I want `make test` and `make e2e` exactly as fast as before, so that the fast loop stays fast.
13. As a developer, I want every `*.sha256` in `tests/e2e/` unchanged, so that this step is provably neutral for the game.
14. As a CI maintainer, I want the guard's exit status to be the script's, so that CI can rely on `make state` failing.
15. As a plan 11 reader, I want the exempt lines listed in one place with their reason, so that I know which sizes must be re-proven on MIPS rather than re-measured.
16. As a developer on plan 8 step 5, I want a new worst-case game whose peaks eat a margin to fail `make state`, so that resizing is a decision someone makes, not an accident.
17. As a reviewer, I want the Makefile legend and the README to say that `make state` fails on a squeezed margin, so that the docs describe what is true.

## Implementation Decisions

- **The state writer** (the devtools module behind `STATE=`) gains one line, `budget zones <sum> <limit>`. The sum is frame + ui + particles + game (generation + load) zone bytes, the same expression as the compile-time budget assert; the limit is `ZONE_BUDGET_BYTES`. The line is the same in every state file. It does not start with `zone` or `cap`, so the peak aggregation ignores it.
- **The peaks script** (the shell script `make state` already calls) stays the single judge:
  - It copies the first `budget zones` line it sees to its output.
  - It prints `state: <limit − sum> bytes left under ZONE_BUDGET_BYTES` on stdout. A missing budget line (every state stopped or absent) prints nothing and fails nothing beyond what already fails.
  - After the aggregation, it judges each `zone`/`cap` peak line. In plain lines, `9 × peak ≤ 8 × limit` (integer arithmetic in awk). For lines named in a single exempt list in the script, `peak ≤ limit`.
  - Each failure writes `state: <zone|cap> <name> peak <p> over margin <floor(8 × limit / 9)> (limit <l>, <events>)` on stderr and sets exit 1. The exit status combines stopped replays and margin failures.
- **Exempt list, with two reasons in the script:**
  - Proven ceiling: `bytes_button`, `bytes_row`, `particles_help_screen`. These are the lines plan 7 sized "without margin". The `button` and `row` slot counts were sized as peak + 1/8 and stay guarded.
  - Fully reserved list, its margin on the `elements_*` line: `bytes_particles`, `bytes_particles_help_screen`, `bytes_ephemerals`.
- **Output format** of the existing table lines (`<zone|cap> <name> <peak> <limit> <events>`) and of the `replay` lines is unchanged.
- **The Makefile `state` campaign recipe** keeps calling the script the same way. The script's own exit stops the recipe, so its final echo only runs when the guard passes. Only the legend at the top of the Makefile changes, to say that the campaign also fails on a squeezed margin and prints the space left under the budget.
- **README**: the `make state` bullet says that it fails on a squeezed margin.
- **Decisions taken without a human** (the guideline skill was applied in place of the user; please confirm at triage):
  1. "Limit minus a 1/8 margin" is read as plan 7's sizing rule (`limit ≥ peak × 9/8`), not as `peak ≤ 7/8 × limit`. The second reading fails today on the `load` zone (21,936 > 21,595), even though that zone was sized at exactly peak × 9/8. It also fails on `elements_particles`, `elements_particles_help_screen`, `bytes_burst`, `bytes_message_box`, `bytes_toast` and `bytes_modal_backdrop`, and would force resizes outside this step.
  2. The plan's "`button`, `row`" means the proven slot sizes `bytes_button` and `bytes_row`, where plan 7 put its proven ceilings. It does not mean the slot counts.
  3. The three fully reserved list slots are exempt. Under rule 1 they fail by construction (peak = slot size = header + full reserve: 1,384, 5,344 and 184 bytes), and their margin is already guarded through `elements_particles`, `elements_particles_help_screen` and `elements_ephemerals`.

## Implementation Chunks

1. **Budget line.** The state writer emits `budget zones <sum> <limit>`; the peaks script copies it and prints the bytes left. Done when a hand-written state file with `budget zones 784452 786432` makes the script print `1980 bytes left`, and the state test finds the line in a real replay's state.
2. **Margin guard.** The peaks script judges each peak line against the margin, or against its limit for an exempt line, and fails on any violation. The Makefile legend and README follow. Done when the hand-written cases below pass, and `make state` on today's tree passes and prints 1,980.

## Testing Decisions

- A good test drives the script or the replay from the outside, with hand-written inputs or a real replay, and asserts only on the output file, stderr and the exit status. It does not test awk internals.
- **The peaks script** is tested in its existing unit test (hand-written state files, no replay). These cases are a few more `system()` calls on tiny files, milliseconds in `make test`:
  - A peak exactly at peak × 9/8 = limit passes (e.g. peak 8, limit 9), and one unit more fails (peak 9, limit 10 → 81 > 80). The failure names the line, its peak, its limit and the replay holding it, and the exit is non-zero.
  - Two failing lines are both named.
  - The table is still written when the guard fails, with the same lines as before.
  - `cap particles_help_screen 1 1 1` passes, while a plain cap at `1 1` fails.
  - `cap bytes_particles - 1384 1384` passes, while `cap bytes_burst - 640 640` fails.
  - An exempt line over its limit fails.
  - The `budget zones` line is copied and the bytes left are printed. A run with no budget line prints no bytes-left line.
  - A stopped replay still fails and is still named as stopped, with the margin judgement on the remaining lines unchanged.
  - The existing four tests keep passing. Their fixtures stay within the margin, or are adjusted where a value was arbitrary (e.g. `zone game 1000 1000 1000` in the stopped-replay test, whose expected failure is the stop).
- **The state writer** is tested in its existing replay test: the `budget zones` line is present, with the sum and the budget of the build. This adds no replay, only one more line lookup.
- **Prior art:** the peaks script's unit test (hand-written state files fed to the script through `system()`), and the state test's required-line list.
- **Acceptance outside the suite:** `make state` passes on today's tree and prints `1980 bytes left under ZONE_BUDGET_BYTES`; `make test-all` is green with every `*.sha256` unchanged.

## Out of Scope

- Resizing any zone or cap, or moving `ZONE_BUDGET_BYTES`.
- Running the guard in `make test`, `make e2e` or `make test-all`: `make state` replays the full games (~seconds) and stays outside the suite.
- Changing `make state EVENTS=<file>`.
- Re-recording a worst-case game (plan 8 step 5) or re-measuring on MIPS (plan 11).
- Guarding the per-replay current values: only the campaign peaks are judged.

## Further Notes

- **Predicted result on today's tree** (from a `make state` run on 42f851f): every line passes. The tightest are `zone load` (21,936 against 24,680: two more bytes of peak fail it, since 9 × 21,938 > 8 × 24,680), `elements_particles` (140/158), `elements_particles_help_screen` (580/653), `elements_ephemerals` (7/8), `bytes_message_box` (4,488/5,056) and `bytes_burst` (568/640).
- **The campaign no longer reaches plan 7's worst case.** Since plan 12 moved the screen to 160×120, the `full_game*` replays diverge at frame 358 (an extra `rnd` draw shifts the generator). They now end with 8 colonies and 2 slipways, and their peaks are well below the ones plan 7 sized on. Examples: game zone 103,344 against 462,520; `row` 10 against 71; `exit_queue` 102 against 315; `particle` 580 against 887. Plan 12 decided to judge them only on stops and limits. So the guard is strong for the zones that are filled from the title screen (`load`, the `elements_*` reserves, the ui slot sizes) and weak for the game-sized ones until a real full game is back in `worst-case/` (plan 8 step 5 is the natural place). This PRD does not fix that; it is noted so that a green `make state` is not read as "the full game fits with margin".
- No subagent tool was available in this session to spawn a fresh `guideline` agent. The `guideline` skill was applied directly to the to-prd check-with-the-user points: the module split, which modules to test, whether to cut the step into chunks, and the three interpretation decisions above.
