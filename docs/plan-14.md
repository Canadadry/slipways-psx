# Plan 14 — Close every porting gap and delete docs/porting-gaps.md

## 1. Goal

`docs/porting-gaps.md` no longer exists, and nothing in the repository refers to it or to a `GAP-n`.

Why: the C code is the reference (`CLAUDE.md`); the PICO-8 cart is no longer one. A difference from the cart is not a "gap", it is the game. So each of the 22 entries ends as exactly one of:

- **a bug in the C** → fixed;
- **a trap** for the next person who edits that code → an `assert` with a message, or a comment of at most 40 words at the site, with no `GAP` reference;
- **nothing** → the entry and its `/* GAP-n */` markers are deleted, the code is unchanged.

## 2. Rules

- **Fate of every entry (locked):**

  | Fate | GAP |
  |---|---|
  | Removed: entry and markers deleted, code unchanged | 1, 9, 12, 13, 17, 19, 20, 22, 24, 25, 26, 27, 28 |
  | Trap moved into the code, behavior unchanged | 3, 5, 14, 15, 18 |
  | Fixed, no visual change | 8, 16 |
  | Fixed, with a visual change | 21 (changes one sha), 23 (no replay shows it) |

- **The `*.sha256` of `tests/e2e/` stay identical** in every step, except `tech_starbirth_protostar.json.sha256` in step 3 (GAP-21). There, and only there, the sha256 is regenerated with `make e2e-baseline` **in the same commit as the fix**, after `make e2e-ref` has shown that the replay first diverges where the protostar's burst is drawn; the commit message names the replay. A sha that changes anywhere else means the step is wrong: it is never regenerated.
- **This exception overrides the chain's no-re-baseline rules** (`check-e2e`, `guideline`, `fix-bug`) for that one replay in step 3 only. A divergence of `tech_starbirth_protostar` after step 3 means the step is unfinished, not a bug to revert: restoring the class ramp to get green is forbidden.
- **Visual change list.** Each visual step appends one line of at most 20 words, plain and concrete, under `### Visual changes` in section 6. When the user asks for the pull request, that list goes into its description under "Visual changes to check": the user checks them in the game before merging.
- **One commit per GAP.** Each step also deletes the entries it closes from `docs/porting-gaps.md` and every `GAP-n` reference to them (code, headers, tests, e2e notes, and other entries of `docs/porting-gaps.md` still open, which then keep their meaning in words) in that same commit. Test logic doesn't change when a reference is removed, only the comment or the `assert` message. Markers sit at more sites than the entries list (for example GAP-5 in `src/engine/entity_registry.c`, GAP-8 in five headers): the grep below finds them, not the entry.
- **GAP grep**, part of the verification of every step 1–6, for each GAP the commit closes: `git grep -nw "GAP-<n>" -- . ':!docs/plan-14.md' ':!docs/prd/' ':!.claude/skills/'` returns nothing. Tracked files only; the plan, the PRD queue and the skills excluded (step 7 rewrites the skills, and its own grep covers them).
- **Decisions and rejected options:**
  - GAP-16: when no ticket can be drawn (empty pool, or every remaining ticket is among the last 3 drawn), **refill the pool** from the planet-type table at the same multiplier and draw again. Rejected: `p8_fatal` (a player can hit it on an unlucky seed); a plain weighted draw with replacement (simpler, but it changes every galaxy and every sha, and loses the forced variety of the shuffle bag).
  - GAP-12 (black wormhole ring) is removed, not fixed: two emulators suggest the cart draws black too, and the C is the reference anyway.
  - The traps (GAP-5, GAP-14) stay traps: walking the lists without skipping would change update timing, hence the shas, for no visible gain.
  - GAP-5 and GAP-14 get the same wording at all three sites (`src/engine/bootstrap.c`, `src/game/particle.c`, `src/game/slipway_display.c`): removing an element during the walk skips the next one.
- **No new GAP while the plan runs.** Until step 7 removes that route from `fix-bug`, no commit (a `fix-bug` one included) adds an entry to `docs/porting-gaps.md` or a `GAP` marker. A trap found on the way goes in as an `assert` or a comment of at most 40 words, as in step 5. The list of 22 fates is closed.
- `make test` and `make e2e` don't get slower. `make test-all` passes on every commit.
- **Out of scope:** the "GAP" literals of `tools/constants_check.sh` / `tools/constants_compare.sh` / `tools/constants_expected_gaps.txt` (PICO-8 constant comparison, a different mechanism with the same word); the pseudo-random generator and the sin/cos table themselves (GAP-1, GAP-17 only lose their entry).

## 3. Phases and steps

### Phase 1 — Fixes

1. **GAP-16, refill the ticket pool.** `weighted_picker_next` (`src/game/gen.c`) refills the pool from the static planet-type table at the multiplier it was built with when no ticket can be drawn, then draws. No new data-pointer field: `make test` passes with `tools/pointer_guard_exceptions.txt` unchanged (`src/game/gen.h` stays at 3). A unit test empties the pool and checks the next draw returns a valid type; another puts only recent tickets in the pool and checks the draw ends. Both tests fail on the code before the fix **without hanging** (today the draw loop spins forever in both cases), so the red phase can be observed. Verified: every sha identical, `make full-game` and `make sanitize-net` pass, GAP grep.
2. **GAP-8, remove the unread tag.** `TAG_TARGETING` leaves the enum in `src/engine/entity_class.h`, every archetype's `.tags`, and its registry reserve in `src/game/cart.c`. The tests are part of the step: uses of `TAG_TARGETING` in tests go, and the frozen unit-test hashes that fold class tags or the targeting list (11 binaries in a trial: `test_planet`, `test_probe`, `test_help_screen`, the planet family…) are set to their new values in the same commit, as `CLAUDE.md` allows for a test that froze the old behavior. The proof of no behavior change stays "every sha identical". Verified: every sha identical, GAP grep, `make sanitize-net` passes; `make state` passes (a smaller peak is expected; a zone size is lowered only if a guard requires it).
3. **GAP-21, the protostar burst's own palette (visual).** `burst_draw` (`src/game/planet_family.c`) reads the instance's trail ramp when it has one; `protostar_construct` gives its burst `1,2,8,14,14,14,14,8,8,2,2,1,1`. A unit test pins both ramps. The re-baseline is part of the step's own implementation, before its commit, in this order: `make e2e` shows exactly one divergent replay, `tech_starbirth_protostar`; `make e2e-ref` puts its first diverging frame where the burst is drawn (frame 75 in a trial fix); `make e2e-baseline`; `git diff --name-only -- tests/e2e` lists only that sha file. Verified: `check-e2e` then returns `E2E-GREEN` with no `fix:` commit, GAP grep. Visual line: "A protostar's burst now uses its own pink palette instead of the ordinary burst's."
4. **GAP-23, a nil tooltip key builds nothing (visual).** In `button_construct` (`src/game/ui_button.c`), a `side_tooltip` or `tooltip` key present but nil builds no row and no tooltip, like `false`. `test_nil_keys` in `tests/test_button.c` is updated to the new behavior: red before the fix, green after; it is the step's proof. Verified: every sha identical, GAP grep (a trial fix showed no replay hovers a project without a tooltip row; `project_tooltip_inputs` hovers a real one). The user checks the change in the game. Visual line: "Hovering a building's project no longer shows an empty tooltip."

### Phase 2 — Traps into the code

5. **GAP-3, 5, 14, 15, 18.** At each site the entry names, one `assert` with a message or one comment of at most 40 words states the trap, without the word GAP: strings compare by content, never by pointer (GAP-3, `src/engine/collect.c`); removal during a walk skips the next element (GAP-5, GAP-14, three sites); missing data is tolerated where Lua would crash, so making one site fatal makes the port inconsistent (GAP-15, `src/engine/entity.c`, `src/game/economy_formulas.c`); the draw palette applies to sprites only (GAP-18, `src/p8/gfx.c`). Its PRD has one Implementation Chunk per GAP, in this order, so each lands as its own commit. Verified: every sha identical, GAP grep per chunk.

### Phase 3 — Removal

6. **The 13 removed entries**, one commit per GAP: 1, 9, 12, 13, 17, 19, 20, 22, 24, 25, 26, 27, 28. Its PRD has one Implementation Chunk per GAP, in this order, so each lands as its own commit. Each deletes the entry, its `/* GAP-n */` markers and its mentions in tests and e2e notes (`tests/e2e/camera_drag_dpad.txt` for GAP-27; the `assert` message in `tests/test_str.c` for GAP-22 keeps its meaning without the GAP name). Verified: every sha identical, GAP grep after each commit.
7. **Delete the file and its references**, in one commit:
   - `docs/porting-gaps.md` deleted.
   - `CLAUDE.md`, section "Comments": a comment stays for two reasons only, a trap or a C contract; the "known divergence from the cart" reason and the `/* GAP-7 */` example go. Section "The port": the archive sentence stays.
   - `README.md`, "How it's built": the mention of `docs/porting-gaps.md` goes.
   - `.claude/skills/fix-bug/SKILL.md`: the "divergence from the cart → numbered entry" line goes.
   - `.claude/skills/rework-comments/` (`SKILL.md`, `agent-brief.md`, `worked-example.md`, `report-template.md`, `header_residue.sh`): the GAP marker and the porting-gaps route go; `header_residue.sh` accepts only `TODO` comments as residue. Checked on scratch headers under `tmp/` only: one with a `TODO` comment prints nothing, one with a bare `/* GAP-1 */` is printed and exits 1. What the script prints over the real tree (about 55 contract comments today) is not a criterion of this step.
   Verified: `git grep -n "porting-gaps\|GAP-[0-9]" -- . ':!docs/plan-14.md' ':!docs/prd/'` returns nothing (tracked files only, so `tmp/` build output is skipped; the `tools/constants_*` lines use "GAP" without a number and don't match); `make test-all` passes; every sha identical.

## 4. Done when

- `docs/porting-gaps.md` does not exist, and the `git grep` of step 7 returns nothing.
- `make test-all`, `make full-game`, `make state` and `make sanitize-net` pass.
- The only `*.sha256` changed by the whole plan is `tests/e2e/tech_starbirth_protostar.json.sha256`, named in step 3's commit message.
- Section 6 holds exactly two visual change lines, each at most 20 words.

## 5. Open questions

- `tools/constants_*` still compares the port's constants with real PICO-8 values. With the cart no longer a reference, that check may be worth dropping too. Not decided; not in this plan.

## 6. Step notes

### Visual changes

- A protostar's burst now uses its own pink palette instead of the ordinary burst's.
- Hovering a building's project no longer shows an empty tooltip.
- A planet's inspection line without net income no longer shows an empty side badge.

### Step 1.3

`burst_draw` reads `self->emitter.trail_colors` and asserts every burst holds a ramp (the archetype's default or its construct keys'), so there is no fallback to the class ramp; `protostar_construct` and the ramp data needed no change. `make e2e` showed only `tech_starbirth_protostar` divergent, first at frame 75 (`make e2e-ref`, reference 4de72ac); re-baselined in the fix commit, the only sha changed. `LIFE_HASH` moved in `test_burst` and `test_protostar`. No `guideline` decision.

### Step 1.1

`weighted_picker_new` lost its table and count parameters: it always pools `planet_types_table` and stores only the multiplier (`p8_fixed_t`), so `gen.h` stays at 3 pointers. A drawability check that consumes no random number triggers the refill; recent draws survive it. Two tests in `tests/test_map_tables.c` (no new binary) use a random source that asserts after 1000 calls, so the red phase aborts instead of hanging. All 30 sha identical; `make full-game` and `make sanitize-net` pass; no other entry cited GAP-16. No `guideline` decision.

### Step 1.2

`TAG_TARGETING` was the last tag before `TAG_COUNT`, so no other tag was renumbered. 27 files: enum, 11 archetype masks, cart reserve, five header markers, GAP-8 entry, 14 test files. Frozen hashes moved in exactly the 11 predicted binaries; `test_burst`, `test_score_panel`, `test_ui` only lost a `tags[]` entry. All 30 sha identical; `make sanitize-net` clean; `make state` passes, game-zone peak 101688 → 100776, no zone size changed. No `guideline` decision.

### Step 1.4

Only a real row builds a side tooltip or a tooltip; a nil key now leaves the button unmarked, `false` keeps its meaning (marked, no row). `button_keys_t` kept its shape. Frozen value moved: `BUTTON_MEMBERS_HASH` in `test_button` only. All 30 sha identical. Deviation: the fix has a second visible effect (no empty side badge on a planet line without net income, reached by no replay); recorded as its own visual line so the user's list is complete. No `guideline` decision.
Phase 1 review: `warn-review` raised 8 warnings; `plan-review` found no plan change. Seven predate plan 14 (frozen pointer counts). One is new and left as a follow-up outside the plan: `button_keys_t.has_tooltip` is still set by callers but no longer read since step 1.4 (harmless: every caller also sets the row).

### Step 2.5

Five comment-only commits, one per GAP, all sha identical, no test logic changed. No trap could be checked at run time, so all are comments, none an `assert`. Deviation: GAP-15's tolerant code lives in `src/engine/entity_registry.c` and `src/game/economy.c` (the entry's `entity.c`/`economy_formulas.c` were stale); the comments went there. GAP-5/GAP-14 share one wording at `update_entities`, the particle loop (replacing an older comment) and the transit-dot loop. Bare markers in `collect.h`, `tech_gate.c`, `entity_registry.c`, `entity_geom.h` deleted without replacement. Comment-only chunks were checked by the pilot (diff, `make test-all`, GAP grep) instead of a separate verifier agent. No `guideline` decision.
