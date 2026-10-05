# Plan 15 — Save and resume

## 1. Goal

A `resume` row on the title screen, above the difficulties, that reloads the game in progress. The game is saved after every player action. `restart` from the pause menu still drops the current game, but the save survives it, so `resume` brings that game back.

The goals behind every rule below:
- **PSX-ready.** The save is the state to be written to a memory card later: one fixed binary record of at most 8 KiB (one PSX block), no pointers, no `FILE*` in game code.
- **Minimal state.** The save holds "where we are", never "how we got here". It holds only what cannot be recomputed after load.
- **The e2e replays stay the source of truth.** With no save, the title screen is pixel-identical to today's, so every `*.sha256` in `tests/e2e/` stays unchanged.

## 2. Rules

### Rejected options
- **Input log** (save the input frames since boot, resume by replaying them): rejected. Resume time grows with the session, and the save is not a state that a PSX memory card can hold.
- **Action log** (save the galaxy seed, the difficulty and the list of player actions, then re-apply them): rejected as far from minimal. We want the state itself, not its history.
- **Dumping derived fields** (income, incoming routes, satisfied needs, amounts…): rejected. Derived values are rebuilt by `economy_recompute()`.

### Format
- Fixed binary layout, in this order: header (magic, version, payload length), payload, then a **CRC-32 trailer** at the end, computed over everything before it.
- Every value is written byte by byte, little-endian. A struct is never `fwrite`n: padding and endianness differ between PC and PSX. Each fixed-point value is its raw 16.16 `int32`.
- Every reference is an index: a node is its index in the save, `planet_type` an index in `planet_types_table`, a tech an index in `technologies_table`, a project an index in the research project table, the difficulty an index in `difficulty_options_table`.
- Size is at most 8 KiB, asserted at encode time. Estimate: 4–6 KB, of which trade routes are the variable part.
- `docs/save-format.md` describes the layout field by field and is kept true for as long as the format lives.

### What the save holds
These come from an inventory of the code; the file:line references are as of this plan.
- **Difficulty**: the index of `g_difficulty` (`economy_score.c:245`).
- **Economy** (`main_economy.h:17`): `credits`, `research_points`, `era`, `year`, `year_ticks`, and `invented` as tech indices in their order. Not saved, because they are derived: `income`, `production`, `demand`, `happiness`, `end_status`.
- **Randomness, which is gameplay state.** Planet types are not chosen at generation: a planet stays `UNKNOWN` until a wormhole reveals it through `planet_roll_type` (`planet.c:302`), which draws from the global RNG. Save:
  - `rnd_state` (`p8/fixed.c:113`);
  - the type picker `g_type_picker`, saved as `tickets` (indices) with their count, `recent`/`recent_set`, `multiplier`, and its random walk `step`/`bias`;
  - the vanish walk `g_planet_vanish_roll` (`planet.c:20-21`).
- **The structure id counter** `g_id_counter` (`structure_icon.c:30`).
- **The probe**, once settled: its pos, map_pos and `fold_anchor`. It persists, and whether it exists decides between probe and wormhole on the next background click (`background.c:197-216`).
- **Every node** (planets, protostars, structures of each kind), in registration order, because tag lists are walked in that order. For each node:
  - kind (class), `header.state`, `pos`, `map_pos`, `planet_id`, `fold_anchor`, `infraspace`;
  - `planet_type` and `description`: `description` goes stale after terraform, so save whether it is the type's description or NULL;
  - `project`, as a project index;
  - `needs`, as an ordered list of `has_resource`/`resource` (path-dependent: `economy_tiers.c:126-133`, `planet_trade.c:95-145`);
  - `development_tier`, a ratchet that only rises;
  - `production_bonus`;
  - `production.has_resource`/`resource` and `PLANET_HAS_PRODUCTION`;
  - for a synthesizer, whether `synthesizable_resources` is spent;
  - `outgoing_routes`, each as planet and slipway index lists plus its resource fields. They are accumulated and never cleared, and recomputing them could pick another partner;
  - `state_time`, for `VANISHING` planets only (they die past 60).
  - The order of each node's `slipways` list is the attach order: rebuild it in that order.
- **Every slipway**: start and end as node indices. `infraspace` and `layer` follow from the endpoints, as in `slipway.c:130-142`.
- **The camera**: `camera_t.header.map_pos` (`scroll.h`), not the shaken `pos`.

### What the save never holds
- **Visual or transient state:** particles, emitters, fades, shakes, transit dots, `served_planets`, tooltips, `targeted_by`, `layout_row`.
- **UI boxes:** the end-of-year box, the research menu, the pause menu. Quitting with the end-of-year box open loses that message, not its effects.
- **Wormholes**, and anything in `PLACING` state (a structure or slipway being placed).
- **Comparing particles** after a resume is out of scope; a later option to turn particles off may make that possible.

### When to save
- `economy_spend`, `economy_advance_clock` and `economy_invent` only set a `save_pending` flag.
- The save is written at the **end of the frame**, after `cart_update60()`, never in the middle of an action.
- `save_pending` stays set **while any wormhole exists**. The save is written at the end of the first frame with none, so it always holds a finished reveal. Quitting during the ~25 frames of an unfolding loses that action entirely, credits included.

### Lifecycle
- `restart` (pause menu) goes back to the title screen and leaves the save untouched.
- Picking a difficulty starts a new game but does not overwrite the save. The new game's first action overwrites it.
- The end of a game (`end_status` is `"final score"` or `"bankrupt"`, `economy.c:234-243`) erases the save.
- `resume` appears only if a valid save exists.

### Title screen
- One extra row at **y=41**, right above `forgiving`, in the same style: border, background 11, sprite **31** (the pause menu's `restart` icon), label `"resume      "` (12 characters).
- The four difficulty rows stay at y=52–95, so the box grows upward only. The logo ends at y=39.
- With the keyboard cursor, `resume` is the first button reached going up.

### Layers
1. **`game/save_state.c`** is pure: `save_state_encode(buffer)` and `save_state_decode(buffer)` plus the loader. It never touches a file.
2. **The storage interface** has three functions: `save_storage_write(bytes, n)`, `save_storage_read(bytes, cap) → n`, `save_storage_erase()`. Its backends:
   - raylib (PC): the file `save/slipways.sav`, with the directory created if missing and gitignored. Writes are atomic: write `save/slipways.sav.tmp`, then `rename()`;
   - headless replay and devtools: in memory, empty at startup;
   - PSX: a memory card block, later and out of scope here.
3. **`cart`** decides when: `save_pending`, the wormhole hold-back, and the lifecycle above.

### Recordings
- When `cmd/game` starts **without** a recording, the recorder copies the current `save/slipways.sav`, if any, into the `events.json` as an optional field `"initial_save": "<hex>"`.
- The headless replay seeds its in-memory storage from `initial_save` when present, otherwise starts empty. Existing e2e replays have no such field, so nothing changes for them.
- When `cmd/game` replays a recording, the startup save comes from the recording, never from disk. Writes during and after the replay go to disk as usual.

### Loader
- It reproduces `cart_start_game` (`cart.c:193`) **without `galaxy_populate`**.
- It recreates each entity in the saved order, with the existing constructors, then overwrites the saved fields:
  - nodes: `planet_new`, `protostar_new`, `structure_new`/`processor_new`/`slipgate_new`/`infraspace_gate_new`/`trading_hub_new`/`synthesizer_new`/`ascension_new`. Structure constructors assert that their keys hold no pos, so set pos after construction;
  - slipways: `slipway_new`;
  - the probe: `probe_new`;
  - the economy: `economy_new` plus `probe_set_main_economy`.
- It **never** calls the action functions, which spend credits or advance the clock: `targeting_confirm`, `slipway_drop`, `planet_apply_project`, `structure_icon_left_press`, `planet_roll_type`. It uses their side-effect-free pieces instead:
  - `entity_set_state`;
  - `planet_install_project`;
  - `attach_slipway` on both ends, in the saved order. A synthesizer's attach may rewrite its production, so restore that after attaching.
- It ends with `economy_recompute()`.
- `slipway_new` opens its own grab arena: no other arena may be open at that moment.

### Robustness
- The decoder validates everything **before building anything**:
  - magic, version, length, CRC;
  - every index within its table: node < count, `planet_type` < 11, tech < 19, project < 42, difficulty 1–4, slipway end ≠ start, and so on.
- A bad save is a normal case, never an assert: the decoder returns "invalid", `resume` stays hidden, and the file is left alone (the next action of a new game overwrites it).
- Asserts are kept for C contract violations only, for example an encode that exceeds 8 KiB.

### Guardrails
- Every existing `*.sha256` in `tests/e2e/` stays unchanged at every step. `pause_restart` and `keyboard_menus` restart before any action, so they are unaffected. `game_end_final_score` and `keyboard_game_end` act and then return to the title through "again?": their last action both sets `save_pending` and ends the game in the same frame, so their title screen stays unchanged only if the erase wins over that frame's end-of-frame write.
- `make test` and `make e2e` must not get slower.
- The raylib app is never launched; verification is headless.
- New e2e replays respect the 180-frame limit, or get a line in `tools/e2e_long_exceptions.txt` (up to 600).

## 3. Phases and steps

### Phase 1 — The RNG bet
1. **Accessors.** Add get/set for `rnd_state`, for the type picker and both random walks (exposing what `galaxy.c:14` keeps static as `picker_random`), and a setter for the structure id counter. Verified by unit tests: a set followed by a get returns the value, and setting `rnd_state` reproduces the same `p8_fixed_rnd` sequence.
2. **RNG sync probe.** Before any format or loader exists, test the bet that saving `rnd_state` is enough. The global RNG is shared with drawing (`emitter_spawn`, `particle.c:99`; bursts, `planet_family.c:292-332`; `slipway_spawn_transit_dot`, `slipway_display.c:27`; on-screen vanishing planets, `planet_display.c:365`), and none of that visual state is saved. Play the same recording twice from boot (the game is deterministic). In the second run only, at the save frame, clear the state the save never holds (live particles and bursts, transit dots, `served_planets`, and so on). Advance both runs by the same number of frames and compare `rnd_state`. Do it once just after a colonization while its burst is still live, and once at a quiet frame. Verified by a unit test that passes in both cases.

   If it fails, stop the plan: record the result as a step note and an open question for the human. Do not work around it in the loader or a harness, since keeping the RNG in sync would mean a content decision that section 2 does not make.

### Phase 2 — State
3. **Format.** Write `save_state_encode`/`save_state_decode` for the layout in section 2, including validation, and `docs/save-format.md`. Verified by unit tests:
   - a valid buffer decodes;
   - a flipped byte fails the CRC;
   - a truncated buffer fails;
   - a wrong magic or version fails;
   - each out-of-range index fails;
   - the encode-size assert holds on the largest case in `worst-case/`.
4. **Loader, thin slice.** Build the comparison harness and the loader for the smallest game: one colonization, saved, then after load one wormhole reveal (RNG and picker) and one year end (needs and routes). Verified by:
   - `encode → load → encode` gives identical bytes;
   - **behavior after load**: game A is played to the save point and encoded, and the save is loaded into a fresh game B. Both then receive the same later actions and must give the same `devtools/state.c` dump, without its zone and cap lines (those follow the session's history), and the same framebuffer apart from particles.
5. **Loader, every kind.** Extend the loader and the step 4 checks to colonization, every structure kind, a slipgate chain, a synthesizer, a revealed wormhole area, a protostar, research, and a vanishing planet. Each case later receives at least one wormhole reveal and one year end.

   In steps 4, 5 and 8, a missing field that a check finds goes back into the step 3 format and `docs/save-format.md`, with a step note. It is never patched in the loader alone.

### Phase 3 — Game
6. **Storage.** Add the `save_storage` interface with its in-memory backend (headless/devtools) and its atomic file backend (raylib, `save/slipways.sav`, gitignored). Add `"initial_save"` to `events.json`: written by the recorder, read by `replay_source` to seed the storage. No visible change. Verified by unit tests (write, read, erase, overwrite, an empty read, a JSON round trip of `initial_save`), with every `*.sha256` unchanged.
7. **Integration in `cart`.** Add `save_pending` in the three economy functions, the end-of-frame write, the wormhole hold-back, the lifecycle (erase at game end, keep on restart, overwrite at the first action of a new game) and the conditional `resume` row at y=41. Verified by:
   - unit tests for the lifecycle and the hold-back, including an action that ends the game: no save remains after that frame's end-of-frame write;
   - `game_end_final_score` and `keyboard_game_end` unchanged (erase at game end);
   - a headless dump of the title screen with a save present: the row is at y=41–51 and rows 52–95 are unchanged;
   - every existing `*.sha256` unchanged.
8. **Full-game save fuzz.** Add a target outside `make test`, `make e2e` and `make test-all` (like `make state`) that replays `worst-case/full_game.json`. At every frame where the game writes a save, it encodes the save, loads it into a fresh game, feeds both games the rest of the recording, and compares them with the step 4 criteria. Verified by the target passing; a mismatch names the save point that broke.
9. **E2e replays.** Two new replays in `tests/e2e/`, each with its `.txt` notes and `.sha256`:
   - act, restart, then `resume`: the camera position and the map match the frame before the restart, apart from particles and animations;
   - a `resume` at frame 0 from a ready-made `initial_save`.

   Verified by `make e2e`.

## 4. Done when

- `make test-all` passes, with every pre-existing `*.sha256` unchanged and the two new e2e replays green.
- The checks of steps 2, 4 and 5 pass for every listed action, and the step 8 full-game save fuzz passes.
- `docs/save-format.md` matches the encoder.
- A human plays on PC, quits, relaunches, and resumes the same game.

## 5. Open questions

None.

## 6. Step notes
