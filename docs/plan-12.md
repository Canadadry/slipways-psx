# Plan 12 — Jump-cursor pad controls

## 1. Goal

A pad control scheme where reaching what you want on the map takes a few presses, not pixel-precise camera panning. The rule the player learns: **point at a thing, × acts on it, ○ backs out.**

The current scheme (`src/game/pad_nav.c`) was measured on a 30-minute human game (110,530 frames, 14 years, 30 colonies, 42 slipways), replayed headlessly with a per-frame trace of the `pad_nav_t` state. The numbers below justify every rule in section 2, and are the baseline that section 4 compares against:

- **Aiming costs the most.** 61% of game time is spent in the free-camera modes. There were 1,066 direction presses there and 35,600 px of camera travel. A third of the holds lasted 10 frames or less (1 px per frame), and 12% were followed by the opposite direction within 40 frames (overshoot). △ and □ act only on the object exactly under the centre crosshair.
- **Network mode is not spatial.** Up/Down were pressed 47 times in network mode and 41 of those did nothing (only Left/Right are mapped, and they cycle slipways clockwise). 15 Left/Right presses did nothing because the planet had no slipway. 12 ○ presses did nothing.
- **The planet menu has no obvious way out.** ○ (the only close) was never pressed in it. □ was pressed 11 times and △ 6 times without effect. The player uses △ as a toggle.
- **One button means too many things.** □ selects, starts a slipway, cancels it, or leaves the planet, depending on 6 map sub-modes. Network mode and slipway building alternated 153 times. About 100 slipways were started, 42 built and about 35 abandoned.
- **The tab ring is slow.** Map, build, end turn, tech and score sit on a ring of 5: bursts of 5–7 shoulder presses in a row. End turn was used twice in the game.
- **Bug:** □ on a slipgate that refuses new slipways gives no feedback. `begin_drag` then presses every other frame and the press falls through to a lower layer (gdb: `slipgate_left_press` returns `stop = 0`), for as long as the player stays there: about 25 s in that game.

## 2. Rules

### Guardrails

- `make test-all` passes on every commit. `make test` and `make e2e` must not get slower (`CLAUDE.md`).
- The game rules don't change: only how pad input becomes mouse presses and cursor positions in `played` (the `pad_nav_step` contract), plus the cursor's drawing. Audio is out of scope.
- **The e2e replays change.** All 17 `tests/e2e/pad_*.json` drive the pad, so a step that changes a mapping re-authors the replays it breaks, in the same commit. A re-authored replay must reach the same game end state as before: same `make state` output except the camera and ui lines. Then its `.json.sha256` is re-baselined and its `.txt` rewritten from the new frames. Never re-baseline a replay whose game end state changed.
- New inputs must not type the cheat sequence (× × ○ ○ × × ○ ○, see `src/game/cheat.c`) by accident.
- One scheme only: no option or flag keeps the old scheme alongside the new one (two schemes would double the e2e and the code).
- Never launch the raylib app. A human checks the feel (section 4).

### Locked decisions

**Cursor and camera**
- On the map, the cursor always sits on an object: a known planet, a colony, or a structure (planet-family entities in states KNOWN or SETTLED). Unexplored planets are not drawn, so they are never targets. Wormholes and probes are not targets.
- D-pad press: jump to the best target in that direction, searched over the whole map, not just the screen. The candidates lie in the 90° cone around the direction (|perpendicular| ≤ along). Score = along + 2 × |perpendicular|, lowest wins (the weight is tunable in the step). With no candidate, nothing moves, and there is no wrap-around.
- D-pad hold: repeat the jump after 18 frames, then every 8 frames (tunable).
- **The camera does not re-centre on every jump** (decided by the user). The cursor moves on screen. The camera scrolls only when the target leaves an inner rectangle: a 20 px margin, more at the bottom so the HUD bar never covers the target. It scrolls the minimum distance, at most 8 px per frame. Rejected: re-centring on every jump, which moves the whole screen at each press.
- The hovered object gets a visible marker (reuse the pad selection blink, `ui_set_pad_selection`).

**Buttons**
- **×** acts on the hovered object:
  - On a colony or structure: start a slipway from it.
  - On a known planet: open its projects (colonize) menu, as a mouse press there does.
  - If the press is refused (for example a full slipgate), nothing is held and no press is retried or passed on: it fixes the bug above by design.
- **Building a slipway:** the D-pad jumps between targets (same picker, start excluded) while the left button stays held on the target, so the game draws its own green or red line. × releases on the target: built or refused by the game's rules. When built, the cursor stays on the target, so × chains the next slipway from there. ○ cancels through the ui's own drop with no target (as `cancel_drag` does today).
- **△:** the planet menu of the hovered object. △ again, or ○, closes it. Inside menus, the D-pad and × work as today (`pad_panel_step`).
- **○ always goes back one step**, in this order: close the box or panel; cancel the slipway or the placement; leave aim mode; at the root, jump to the last object × acted on (nothing at game start).
- **□ toggles aim mode:** a free crosshair at the screen centre with the camera panned under it, the way free mode pans today (1/2/4 px per frame). × presses the background as today: probe, or fold once a probe is out; hold × to fold. □ or ○ goes back to jump mode, on the target nearest the centre. This is the only pixel aiming left.
- **L1** toggles the build bar: Left/Right pick a structure, × takes it, ○ or L1 closes the bar. Placing a structure uses the aim-mode crosshair (structures go in empty space): × places, ○ cancels.
- **R1** cycles tech panel → score panel → closed.
- **Select** (new button): a tap cycles through colonies needing attention. A hold of 30 frames ends the year: end turn was used twice in the measured game, and the hold prevents accidents.
- **Start:** pause, as today.
- The tab ring (`pad_nav_tab_t`, map/build/end turn/tech/score) and network mode disappear. The HUD highlight (`ui_set_pad_tab`) marks the button of whichever panel is open.

### Rejected

- **A modifier button (L2 + D-pad) for free aiming:** it needs a chord and a new button. □ as a toggle uses buttons that already exist.
- **Launching probes from a colony and steering them:** probes target a point in space, so a crosshair is needed anyway.
- **Snapping to exact pixel offsets for structure placement:** placement needs empty space, so it uses the aim crosshair.

### Out of scope

- PSX build work; mouse input (removed earlier); audio.
- Any change to what the HUD buttons, panels or menus contain.

## 3. Phases and steps

### Phase 1 — Groundwork

1. skill: fix-bug — In the current scheme, □ on a slipgate that refuses new slipways pulses a mouse press every other frame (`begin_drag` retries; `slipgate_left_press` returns `stop = 0` and the press falls through). Expected: the drag gives up until the starting planet changes, and no press reaches a lower layer. Reproduce with a short pad e2e that fills a slipgate's capacity, then presses □ on it.
2. **Select button.** Add `P8_PAD_SELECT` to `src/p8/input.h`, `"select"` to the name tables in `src/devtools/input_json.c` and `src/platform/keymap.c`, and a `select =` line to `keymap.cfg` (keyboard key plus `MIDDLE_LEFT`). Verified by `tests/test_keymap.c` and `tests/test_replay_source.c` reading the new name. Every e2e is unchanged (no replay presses it yet).
3. **Target picker.** A pure function: given an origin, a direction and the live candidate set, it returns the best target or NULL, following the cone and score rule of section 2. Unit-tested on hand-built layouts: one candidate per direction, a tie broken by score, nothing in the cone, origin excluded. No behaviour change; e2e unchanged.

### Phase 2 — Jump mode on the map

4. **Jump mode replaces network mode and plain free mode as the map's root:** D-pad jumps with hold repeat, the hovered marker, the edge scroll and △ on the hovered object. At game start the cursor is on the starting colony, or on the target nearest the screen centre when there is none. The affected e2e are re-authored under the guardrail of section 2. Verified by unit tests in `tests/test_pad_nav.c` (jump, repeat timing, edge scroll keeps the target inside the rectangle) and by the re-authored replays' `make state` matching.
5. **Aim mode on □** (today's free-mode panning and its × probe/fold behaviour, without its □ select and its drag). □ or ○ returns to jump mode on the target nearest the centre. `pad_probe` and any fold replay re-authored. Verified as in step 4.
6. **Slipways from ×:** × on a colony starts the slipway, the D-pad jumps between targets with the button held, × builds and keeps the cursor on the target, ○ cancels. A refused start holds nothing. `pad_slipway` and the replays that build slipways re-authored. Unit test: building a chain of two slipways with × D-pad × D-pad ×.

### Phase 3 — × and ○ everywhere

7. **× on a known planet** opens its projects menu. **○ back order** as in section 2, including the root jump to the last object × acted on. Unit tests: one per rung of the ○ order, and ○ at game start does nothing. Affected replays (`pad_colonise`, `pad_planet_projects`, `pad_empty_planet`) re-authored.

### Phase 4 — Direct buttons instead of the tab ring

8. **L1 build bar** and placement with the aim crosshair (× places, ○ cancels). `pad_build` re-authored.
9. **R1 cycles tech → score → closed.** `pad_research`, `pad_tech_costs` and `pad_score` re-authored.
10. **Select:** a tap cycles colonies needing attention, a 30-frame hold ends the year. Read the code for what the game already tracks as an unmet need. If it tracks nothing usable, cycle every colony in creation order and note it in step notes. `pad_end_turn` re-authored. Unit tests: tap order, a hold under 30 frames ends nothing.
11. **Remove the tab ring and network mode:** `pad_nav_tab_t` and the network fields go, and the HUD highlight follows the open panel. Remaining replays (`pad_title`, `pad_pause_restart`, `pad_message_box`, `pad_bankrupt`, `pad_game_end`, `pad_restart_resume`, `pad_resume_at_start`) re-authored where they broke. `tests/test_pad_nav.c` keeps only tests of the new scheme.

### Phase 5 — Finish

12. **README:** the Controls section rewritten for the new scheme (table plus one line per mode: jump, aim, slipway, menus).
13. skill: rework-comments — `src/game/pad_nav.c`, `src/game/pad_nav.h` and any new pad file.

## 4. Done when

- `make test-all` passes, and every e2e's game end state matches its pre-plan `make state` except the camera and ui lines.
- The README Controls section describes only the new scheme.
- The human plays a full game with the new scheme and records it (`tmp/events_recorded_*.json`). Measured on that recording, against the baseline of section 1: no dead D-pad press in jump mode except at the map's edge, no dead ○ anywhere, and less than 25% of game time in aim mode. The human judges the feel; their complaints become fix-bug runs or a new plan.

## 5. Open questions

- The tuning values (score weight 2, repeat 18/8 frames, margin 20 px, 8 px per frame scroll) are first guesses: adjust after the human playtest.
- "Colonies needing attention" depends on what the game tracks (step 10 decides, with a fallback).

## 6. Step notes
