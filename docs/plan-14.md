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
  | Fixed, with a visual change | 21, 23 |

- **The `*.sha256` of `tests/e2e/` stay identical** in every step, except the two visual steps (21, 23). There, and only there, the changed sha256 are regenerated with `make e2e-baseline` **in the same commit as the fix**. The step proves with `make e2e-ref` that each changed replay first diverges where the fixed element is drawn, and the commit message names each changed replay. A sha that changes anywhere else means the step is wrong: it is never regenerated.
- **Visual change list.** Each visual step appends one line of at most 20 words, plain and concrete, under `### Visual changes` in section 6. When the user asks for the pull request, that list goes into its description under "Visual changes to check": the user checks them in the game before merging.
- **One commit per GAP.** Each step also deletes the entries it closes from `docs/porting-gaps.md` and every `GAP-n` reference to them (code, tests, e2e notes) in that same commit. Test logic doesn't change when a reference is removed, only the comment or the `assert` message.
- **Decisions and rejected options:**
  - GAP-16: when no ticket can be drawn (empty pool, or every remaining ticket is among the last 3 drawn), **refill the pool** from the planet-type table at the same multiplier and draw again. Rejected: `p8_fatal` (a player can hit it on an unlucky seed); a plain weighted draw with replacement (simpler, but it changes every galaxy and every sha, and loses the forced variety of the shuffle bag).
  - GAP-12 (black wormhole ring) is removed, not fixed: two emulators suggest the cart draws black too, and the C is the reference anyway.
  - The traps (GAP-5, GAP-14) stay traps: walking the lists without skipping would change update timing, hence the shas, for no visible gain.
  - GAP-5 and GAP-14 get the same wording at all three sites (`src/engine/bootstrap.c`, `src/game/particle.c`, `src/game/slipway_display.c`): removing an element during the walk skips the next one.
- `make test` and `make e2e` don't get slower. `make test-all` passes on every commit.
- **Out of scope:** the "GAP" literals of `tools/constants_check.sh` / `tools/constants_compare.sh` / `tools/constants_expected_gaps.txt` (PICO-8 constant comparison, a different mechanism with the same word); the pseudo-random generator and the sin/cos table themselves (GAP-1, GAP-17 only lose their entry).

## 3. Phases and steps

### Phase 1 — Fixes

1. **GAP-16, refill the ticket pool.** `weighted_picker_next` (`src/game/gen.c`) refills the pool from the table it was built from (keep the table, its count and the multiplier in `weighted_picker_t`) when no ticket can be drawn, then draws. A unit test empties the pool and checks the next draw returns a valid type; another puts only recent tickets in the pool and checks the draw ends. Verified: every sha identical, `make full-game` passes.
2. **GAP-8, remove the unread tag.** `TAG_TARGETING` leaves the enum in `src/engine/entity_class.h`, every archetype's `.tags`, and its registry reserve in `src/game/cart.c`. Verified: every sha identical; `make state` passes (a smaller peak is expected; a zone size is lowered only if a guard requires it).
3. **GAP-21, the protostar burst's own palette (visual).** `burst_draw` (`src/game/planet_family.c`) reads the instance's trail ramp when it has one; `protostar_construct` gives its burst `1,2,8,14,14,14,14,8,8,2,2,1,1`. A unit test pins both ramps. Expected changed sha: `tech_starbirth_protostar`. Visual line: "A protostar's burst now uses its own pink palette instead of the ordinary burst's."
4. **GAP-23, a nil tooltip key builds nothing (visual).** In `button_construct` (`src/game/ui_button.c`), a `side_tooltip` or `tooltip` key present but nil builds no row and no tooltip, like `false`. `test_nil_keys` in `tests/test_button.c` is updated to the new behavior. Expected changed sha: `project_tooltip_inputs` at least. Visual line: "Hovering a building's project no longer shows an empty tooltip."

### Phase 2 — Traps into the code

5. **GAP-3, 5, 14, 15, 18.** At each site the entry names, one `assert` with a message or one comment of at most 40 words states the trap, without the word GAP: strings compare by content, never by pointer (GAP-3, `src/engine/collect.c`); removal during a walk skips the next element (GAP-5, GAP-14, three sites); missing data is tolerated where Lua would crash, so making one site fatal makes the port inconsistent (GAP-15, `src/engine/entity.c`, `src/game/economy_formulas.c`); the draw palette applies to sprites only (GAP-18, `src/p8/gfx.c`). Verified: every sha identical.

### Phase 3 — Removal

6. **The 13 removed entries**, one commit per GAP: 1, 9, 12, 13, 17, 19, 20, 22, 24, 25, 26, 27, 28. Each deletes the entry, its `/* GAP-n */` markers and its mentions in tests and e2e notes (`tests/e2e/camera_drag_dpad.txt` for GAP-27; the `assert` message in `tests/test_str.c` for GAP-22 keeps its meaning without the GAP name). Verified: every sha identical, `grep -rn "GAP-<n>\b"` empty after each commit.
7. **Delete the file and its references**, in one commit:
   - `docs/porting-gaps.md` deleted.
   - `CLAUDE.md`, section "Comments": a comment stays for two reasons only, a trap or a C contract; the "known divergence from the cart" reason and the `/* GAP-7 */` example go. Section "The port": the archive sentence stays.
   - `README.md`, "How it's built": the mention of `docs/porting-gaps.md` goes.
   - `.claude/skills/fix-bug/SKILL.md`: the "divergence from the cart → numbered entry" line goes.
   - `.claude/skills/rework-comments/` (`SKILL.md`, `agent-brief.md`, `worked-example.md`, `report-template.md`, `header_residue.sh`): the GAP marker and the porting-gaps route go; `header_residue.sh` accepts only `TODO` comments as residue and its behavior is checked by running the skill's gate once.
   Verified: `grep -rn "porting-gaps\|GAP-[0-9]" --exclude-dir=.git .` returns only the out-of-scope `tools/constants_*` lines (they use "GAP" without a number) and nothing else; `make test-all` passes; every sha identical.

## 4. Done when

- `docs/porting-gaps.md` does not exist, and the grep of step 7 is clean.
- `make test-all`, `make full-game`, `make state` and `make sanitize-net` pass.
- The only `*.sha256` changed by the whole plan are those named by steps 3 and 4, each named in its commit message.
- Section 6 holds exactly two visual change lines, each at most 20 words.

## 5. Open questions

- `tools/constants_*` still compares the port's constants with real PICO-8 values. With the cart no longer a reference, that check may be worth dropping too. Not decided; not in this plan.

## 6. Step notes

### Visual changes

(each visual step appends its line here)
