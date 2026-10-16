# PSX steady-frame levers

Still to do: plan 22's phase 3 works down this list and updates it after each lever; plan 22's step 9 deletes it.

This is the ranked list of pixel-identical levers for the **steady frames
over 263 hblanks** that `real-game-budget` counts. It also gives the
estimate each lever is measured against. `make lever-estimate` reads the
`lever-table` block below, applies it to the committed frame table
(`psx/perf/full-run-frames.txt`, plan 22 step 6's) joined with
`psx/perf/full-run-situations.txt`, and checks that this document's order
is the greedy ranking. Everything outside that block is for people.

Starting point on the committed table: **1,132 steady frames over 263**
(map-rest 429, map-move 387, build 155, menu 91, slipway-aim 41, tech-panel
21, score-panel 8; worst steady frame 4657, `build`, 530). The one-off
frames use 1,391 of their 10,800 vsyncs, and no lever here touches them.

## Inputs

### Steady frames' mean cost and the 2% threshold

From `make lever-estimate`: **M = 218.8 hblanks** (mean `total` over the
4,278 steady frames, `title` included), so **T = 0.02 × M = 4.4 hblanks**.
Per situation, the mean of its steady frames, for reference (rule (b)
below uses the profile source's own total):

| situation | steady frames | over 263 | mean total | 2% |
|---|--:|--:|--:|--:|
| map-rest | 1,564 | 429 | 215.2 | 4.3 |
| map-move | 1,907 | 387 | 207.6 | 4.2 |
| slipway-aim | 111 | 41 | 269.4 | 5.4 |
| build | 366 | 155 | 258.7 | 5.2 |
| menu | 252 | 91 | 239.1 | 4.8 |
| tech-panel | 47 | 21 | 281.5 | 5.6 |
| score-panel | 17 | 8 | 296.9 | 5.9 |
| title | 14 | 0 | 40.1 | — |

How far over the frames are, as the share of the frame's draw that would
have to go for it to fit: map-rest's 429 frames split 41 under 5%, 63 at
5–10%, 75 at 10–15%, 90 at 15–20%, 124 at 20–30% and 36 above 30%. For
map-move's 387 it is 57 / 72 / 83 / 57 / 81 / 37, and for build's 155 it is
18 / 14 / 27 / 38 / 49 / 9. A lever worth 5% of draw clears only the first
band.

### Where each situation's figures come from

| situation | source of the function figures | PSX per-call costs | primitives/game-code split |
|---|---|---|---|
| map-move | `map-move-1057`, representative | its own | its own |
| menu | `menu-5954`, representative | its own | its own |
| tech-panel | `tech-panel-5895`, representative | its own | its own |
| map-rest | `no-run`, 429 full-game frames over | `map-rest-5581` (best try) | split from `map-rest-5581` |
| build | `no-run`, 155 full-game frames over | `build-5752` (best try) | split from `build-5752` |
| score-panel | `no-run`, 8 full-game frames over | `score-panel-5876` (best try) | split from `score-panel-5876` |
| slipway-aim | `no-run`, 41 full-game frames over | pooled representative runs | none |

All profiles come from `make steady-profile` at `4014347`. The steady
runs' boots and profiles were dated after that target's last change, so
they were used as they stood. The pooled breakdown overhead is **0.0558
hblanks a call** (six runs, 32,752 calls; `overhead.txt`).

PSX cost per call by family, over each run's window, from its breakdown
boot, minus the pooled overhead (hblanks; calls per window frame in
brackets):

| run | spr | print | rectfill | pattern | rect | line | pset | circ |
|---|--:|--:|--:|--:|--:|--:|--:|--:|
| map-rest-5581 | 0.406 (75) | 1.377 (20) | 0.140 (84) | 0.184 (18) | 0.253 (8) | 0.091 (120) | 0.052 (201) | 5.716 (1) |
| map-move-1057 | 0.395 (53) | 0.767 (8) | 0.160 (45) | 0.195 (18) | 0.253 (8) | 0.090 (40) | 0.065 (79) | 20.158 (5) |
| build-5752 | 0.398 (67) | 0.805 (8) | 0.140 (64) | 0.297 (18) | 0.255 (8) | 0.076 (117) | 0.047 (89) | — |
| menu-5954 | 0.388 (91) | 0.850 (8) | 0.125 (115) | 0.195 (18) | 0.262 (10) | 0.079 (176) | 0.035 (263) | — |
| tech-panel-5895 | 0.396 (109) | 1.464 (21) | 0.139 (115) | 0.218 (19) | 0.256 (14) | 0.084 (136) | 0.059 (244) | — |
| score-panel-5876 | 0.362 (85) | 0.893 (20) | 0.136 (86) | 0.184 (18) | 0.252 (8) | 0.083 (136) | 0.062 (160) | — |

### Known biases

- **PC hook cost.** A function's game code is its whole PC child time, so
  the profiler's hooks still weigh on call-heavy code even after the
  per-call correction. Example: `slipway_draw_method`'s game code is 45–76
  h a frame, with about 2,500 helper calls a frame on `menu-5954`. A
  `share` estimate on such code quotes its pessimistic figure as the honest
  one.
- **Resumed runs draw less.** The best tries draw 12–24% less than the full
  game on the same frames. The first map-rest try had half the `pset`
  calls, for a reason that is still not proven (`docs/psx-game.md`, Still to
  do). It shows here again. `message_box_draw_overlay_method` is absent
  from `build-5752`, but takes 13.3% of `build`'s `no-run` frames. The
  background takes 3.9% of `map-rest-5581` and 5.9% of `build-5752`,
  against 17.6% and 14.4% in `no-run`. *Lead, not proven:* this is the
  draw that resumed runs miss.
- **`no-run` shares are PC time.** They weigh each primitive by what the PC
  software renderer pays for it, not by what the PSX GPU path pays. One run
  measures that gap: on `menu-5954`, `background_draw_method` (a 256×256
  image copy plus the grid) takes 18.2% of `draw_layers`' PC time, but
  only 16.6 of 281.0 PSX draw hblanks (5.9%). This also accounts for part
  of the background's gap above. As a result, `no-run` overstates
  primitive-heavy functions (background, message box, score panel) and
  understates game code (`frame` group, particles).
- **Representative windows are not every frame of their situation.**
  `map-move-1057`'s window draws a wormhole (`wormhole_draw_method` 95.8 h,
  4.7 `circ` a frame). The full game's 388 map-move frames over draw 294
  `circ` in all, on 62 frames. A `no-run` cross-check of map-move (not a
  source, kept in `tmp/` only) puts the wormhole under 1%. Lever N3 is
  therefore sized on the full game's circle count (see its entry). The
  wormhole's 43.1 h "per steady frame" below is the rule's figure and
  overstates it.

### Profiles

Hblanks per frame of the source (run window frames, or `no-run` frames
over). Function lines under 0.5% are summed in one row. The
`in-primitives`/`game-code` columns exist only for runs with a breakdown
boot.

#### `map-rest` (source of map-rest: `no-run`, the full game's map-rest frames over 263)

`flag no-run`  
`no-run map-rest frames 429 budget 263 shares pc-time`  

| line | group | total | share |
|---|---|--:|--:|
| group | planets | 42.6 | 13.8% |
| group | slipways | 58.3 | 18.9% |
| group | particles | 5.2 | 1.7% |
| group | hud | 43.6 | 14.1% |
| group | panels | 30.2 | 9.8% |
| group | map | 55.3 | 17.9% |
| group | overlays | 0.5 | 0.2% |
| group | frame | 16.1 | 5.2% |
| `slipway_draw_method` | slipways | 56.8 | 18.4% |
| `background_draw_method` | map | 54.3 | 17.6% |
| `message_box_draw_overlay_method` | panels | 30.2 | 9.8% |
| `score_panel_draw_method` | hud | 30.2 | 9.8% |
| `planet_draw_method` | planets | 25.2 | 8.2% |
| `ascension_draw_method` | planets | 11.2 | 3.6% |
| `button_draw_method` | hud | 8.4 | 2.7% |
| `draw_layers` | frame | 6.8 | 2.2% |
| `emitter_update_method` | particles | 5.2 | 1.7% |
| `entity_geom_is_on_screen` | frame | 4.9 | 1.6% |
| `toast_draw_method` | hud | 4.9 | 1.6% |
| `burst_draw_method` | planets | 3.7 | 1.2% |
| `entity_pos` | frame | 2.7 | 0.9% |
| `structure_icon_draw_method` | planets | 2.6 | 0.9% |
| `slipway_draw_overlay_method` | slipways | 1.5 | 0.5% |
| functions under 0.5% each | | | 1.0% |
| update `update_entities` | | 17.2 | 5.6% |
| update `ui_update` | | 15.2 | 4.9% |
| update `input_step_run` | | 12.5 | 4.1% |
| update `save_at_frame_end` | | 4.2 | 1.4% |
| update `other` | | 1.4 | 0.4% |
| submit | | 6.0 | 1.9% |

`adds-up n/a no-run sum 308.3 measured 308.3`

#### `map-rest-5581` (best try for map-rest: PSX per-call costs and splits only)

`flag no-representative-run map-rest`  
`window run-frames 56-70 frames 15 overhead-per-call 0.0558 calls 7654`  

| line | group | in-primitives | game-code | total | share |
|---|---|--:|--:|--:|--:|
| group | planets | 29.2 | 37.1 | 66.3 | 23.1% |
| group | slipways | 14.3 | 52.8 | 67.0 | 23.3% |
| group | particles | 2.4 | 12.8 | 15.1 | 5.3% |
| group | hud | 12.2 | 9.5 | 21.7 | 7.6% |
| group | panels | 32.2 | 10.4 | 42.6 | 14.8% |
| group | map | 8.6 | 2.5 | 11.1 | 3.9% |
| group | overlays | 0.0 | 0.9 | 0.9 | 0.3% |
| group | frame | 0.0 | 35.0 | 35.0 | 12.2% |
| `slipway_draw_method` | slipways | 14.3 | 48.9 | 63.2 | 22.0% |
| `message_box_draw_overlay_method` | panels | 32.2 | 10.4 | 42.6 | 14.8% |
| `planet_draw_method` | planets | 19.7 | 15.2 | 34.9 | 12.2% |
| `ascension_draw_method` | planets | 7.4 | 20.5 | 27.9 | 9.7% |
| `score_panel_draw_method` | hud | 10.3 | 6.1 | 16.4 | 5.7% |
| `emitter_update_method` | particles | 2.4 | 12.8 | 15.2 | 5.3% |
| `draw_layers` | frame | 0.0 | 13.7 | 13.7 | 4.8% |
| `background_draw_method` | map | 8.6 | 2.5 | 11.1 | 3.9% |
| `entity_geom_is_on_screen` | frame | 0.0 | 9.7 | 9.7 | 3.4% |
| `entity_pos` | frame | 0.0 | 8.3 | 8.3 | 2.9% |
| `button_draw_method` | hud | 2.0 | 3.1 | 5.1 | 1.8% |
| `slipway_draw_overlay_method` | slipways | 0.0 | 3.8 | 3.8 | 1.3% |
| `structure_icon_draw_method` | planets | 2.1 | 1.3 | 3.4 | 1.2% |
| functions under 0.5% each | | | | | 1.4% |
| update `input_step_run` | | | | 7.7 | 2.7% |
| update `ui_update` | | | | 6.0 | 2.1% |
| update `update_entities` | | | | 5.6 | 2.0% |
| update `other` | | | | 0.8 | 0.3% |
| submit | | | | 6.0 | 2.1% |

`adds-up ok sum 285.7 measured 287.3 residual -1.6 noise-floor 3.0`

#### `map-move-1057` (source of map-move: representative run)

`window run-frames 168-182 frames 15 overhead-per-call 0.0558 calls 3591`  

| line | group | in-primitives | game-code | total | share |
|---|---|--:|--:|--:|--:|
| group | planets | 13.2 | 12.7 | 26.0 | 7.4% |
| group | slipways | 5.0 | 15.3 | 20.3 | 5.8% |
| group | particles | 3.2 | 11.4 | 14.6 | 4.1% |
| group | hud | 12.7 | 10.2 | 22.9 | 6.5% |
| group | panels | 0.0 | 0.0 | 0.0 | 0.0% |
| group | map | 104.4 | 4.3 | 108.7 | 30.9% |
| group | overlays | 0.7 | 0.8 | 1.5 | 0.4% |
| group | frame | 0.0 | 28.6 | 28.7 | 8.1% |
| `wormhole_draw_method` | map | 94.1 | 1.7 | 95.8 | 27.2% |
| `planet_draw_method` | planets | 12.4 | 11.9 | 24.3 | 6.9% |
| `slipway_draw_method` | slipways | 5.0 | 14.7 | 19.7 | 5.6% |
| `score_panel_draw_method` | hud | 9.9 | 6.1 | 16.0 | 4.5% |
| `emitter_update_method` | particles | 3.2 | 11.4 | 14.6 | 4.1% |
| `entity_geom_is_on_screen` | frame | 0.0 | 13.1 | 13.1 | 3.7% |
| `background_draw_method` | map | 10.4 | 2.6 | 13.0 | 3.7% |
| `draw_layers` | frame | 0.0 | 9.7 | 9.7 | 2.7% |
| `button_draw_method` | hud | 2.0 | 3.4 | 5.4 | 1.5% |
| `entity_pos` | frame | 0.0 | 2.4 | 2.4 | 0.7% |
| `structure_icon_draw_method` | planets | 0.8 | 0.8 | 1.6 | 0.5% |
| functions under 0.5% each | | | | | 2.0% |
| update `update_entities` | | | | 61.1 | 17.4% |
| update `save_at_frame_end` | | | | 23.0 | 6.5% |
| update `ui_update` | | | | 21.5 | 6.1% |
| update `input_step_run` | | | | 17.6 | 5.0% |
| update `other` | | | | 2.3 | 0.6% |
| submit | | | | 5.9 | 1.7% |

`adds-up ok sum 353.8 measured 352.0 residual 1.8 noise-floor 3.5`

#### `slipway-aim` (source of slipway-aim: `no-run`, the full game's slipway-aim frames over 263)

`flag no-run`  
`no-run slipway-aim frames 41 budget 263 shares pc-time`  

| line | group | total | share |
|---|---|--:|--:|
| group | planets | 37.5 | 11.4% |
| group | slipways | 65.3 | 19.7% |
| group | particles | 3.1 | 0.9% |
| group | hud | 37.8 | 11.4% |
| group | panels | 41.7 | 12.6% |
| group | map | 47.4 | 14.3% |
| group | overlays | 0.3 | 0.1% |
| group | frame | 14.0 | 4.2% |
| `slipway_draw_method` | slipways | 62.9 | 19.0% |
| `background_draw_method` | map | 47.4 | 14.3% |
| `message_box_draw_overlay_method` | panels | 41.7 | 12.6% |
| `score_panel_draw_method` | hud | 27.1 | 8.2% |
| `planet_draw_method` | planets | 26.8 | 8.1% |
| `button_draw_method` | hud | 7.2 | 2.2% |
| `draw_layers` | frame | 5.7 | 1.7% |
| `burst_draw_method` | planets | 5.4 | 1.6% |
| `entity_geom_is_on_screen` | frame | 4.4 | 1.3% |
| `ascension_draw_method` | planets | 3.4 | 1.0% |
| `toast_draw_method` | hud | 3.3 | 1.0% |
| `emitter_update_method` | particles | 3.1 | 0.9% |
| `entity_pos` | frame | 2.4 | 0.7% |
| `slipway_draw_overlay_method` | slipways | 2.4 | 0.7% |
| `structure_icon_draw_method` | planets | 2.0 | 0.6% |
| functions under 0.5% each | | | 0.5% |
| update `ui_update` | | 57.0 | 17.2% |
| update `input_step_run` | | 12.9 | 3.9% |
| update `update_entities` | | 6.7 | 2.0% |
| update `other` | | 1.0 | 0.3% |
| submit | | 6.0 | 1.8% |

`adds-up n/a no-run sum 330.6 measured 330.6`

#### `build` (source of build: `no-run`, the full game's build frames over 263)

`flag no-run`  
`no-run build frames 155 budget 263 shares pc-time`  

| line | group | total | share |
|---|---|--:|--:|
| group | planets | 55.2 | 17.7% |
| group | slipways | 66.4 | 21.3% |
| group | particles | 5.3 | 1.7% |
| group | hud | 34.9 | 11.2% |
| group | panels | 41.3 | 13.3% |
| group | map | 45.0 | 14.4% |
| group | overlays | 1.0 | 0.3% |
| group | frame | 13.8 | 4.4% |
| `slipway_draw_method` | slipways | 65.0 | 20.9% |
| `background_draw_method` | map | 44.8 | 14.4% |
| `message_box_draw_overlay_method` | panels | 41.3 | 13.3% |
| `planet_draw_method` | planets | 28.1 | 9.0% |
| `score_panel_draw_method` | hud | 26.0 | 8.4% |
| `structure_icon_draw_method` | planets | 13.9 | 4.5% |
| `ascension_draw_method` | planets | 12.7 | 4.1% |
| `button_draw_method` | hud | 7.0 | 2.3% |
| `draw_layers` | frame | 5.8 | 1.9% |
| `emitter_update_method` | particles | 5.3 | 1.7% |
| `entity_geom_is_on_screen` | frame | 4.1 | 1.3% |
| `entity_pos` | frame | 2.6 | 0.8% |
| `toast_draw_method` | hud | 1.7 | 0.6% |
| `slipway_draw_overlay_method` | slipways | 1.4 | 0.5% |
| functions under 0.5% each | | | 1.0% |
| update `ui_update` | | 23.8 | 7.6% |
| update `input_step_run` | | 7.7 | 2.5% |
| update `update_entities` | | 7.6 | 2.4% |
| update `other` | | 2.0 | 0.6% |
| submit | | 7.3 | 2.3% |

`adds-up n/a no-run sum 311.3 measured 311.3`

#### `build-5752` (best try for build: PSX per-call costs and splits only)

`flag no-representative-run build`  
`window run-frames 37-51 frames 15 overhead-per-call 0.0558 calls 5294`  

| line | group | in-primitives | game-code | total | share |
|---|---|--:|--:|--:|--:|
| group | planets | 21.4 | 28.9 | 50.2 | 20.8% |
| group | slipways | 11.3 | 49.1 | 60.4 | 24.9% |
| group | particles | 0.5 | 3.2 | 3.7 | 1.5% |
| group | hud | 13.5 | 9.2 | 22.6 | 9.3% |
| group | panels | 0.0 | 0.0 | 0.0 | 0.0% |
| group | map | 11.6 | 2.8 | 14.3 | 5.9% |
| group | overlays | 0.6 | 1.5 | 2.1 | 0.9% |
| group | frame | 0.0 | 34.0 | 34.0 | 14.1% |
| `slipway_draw_method` | slipways | 11.3 | 45.2 | 56.5 | 23.3% |
| `planet_draw_method` | planets | 15.2 | 14.2 | 29.4 | 12.2% |
| `ascension_draw_method` | planets | 3.7 | 13.3 | 17.0 | 7.0% |
| `score_panel_draw_method` | hud | 11.1 | 5.7 | 16.8 | 7.0% |
| `background_draw_method` | map | 11.6 | 2.8 | 14.4 | 5.9% |
| `draw_layers` | frame | 0.0 | 14.2 | 14.2 | 5.9% |
| `entity_geom_is_on_screen` | frame | 0.0 | 10.0 | 10.0 | 4.1% |
| `entity_pos` | frame | 0.0 | 6.8 | 6.8 | 2.8% |
| `button_draw_method` | hud | 2.3 | 3.2 | 5.5 | 2.3% |
| `slipway_draw_overlay_method` | slipways | 0.0 | 3.9 | 3.9 | 1.6% |
| `structure_icon_draw_method` | planets | 2.5 | 1.4 | 3.9 | 1.6% |
| `emitter_update_method` | particles | 0.5 | 3.2 | 3.7 | 1.5% |
| `targeting_tooltip_method` | overlays | 0.6 | 1.5 | 2.1 | 0.8% |
| `entity_registry_layer` | frame | 0.0 | 1.1 | 1.1 | 0.5% |
| functions under 0.5% each | | | | | 0.8% |
| update `ui_update` | | | | 33.2 | 13.7% |
| update `input_step_run` | | | | 7.9 | 3.3% |
| update `update_entities` | | | | 7.7 | 3.2% |
| update `other` | | | | 0.9 | 0.4% |
| submit | | | | 6.4 | 2.6% |

`adds-up ok sum 243.6 measured 242.1 residual 1.5 noise-floor 3.0`

#### `menu-5954` (source of menu: representative run)

`window run-frames 62-76 frames 15 overhead-per-call 0.0558 calls 9951`  

| line | group | in-primitives | game-code | total | share |
|---|---|--:|--:|--:|--:|
| group | planets | 29.8 | 51.2 | 81.0 | 26.0% |
| group | slipways | 19.3 | 78.7 | 98.0 | 31.5% |
| group | particles | 1.6 | 11.7 | 13.3 | 4.3% |
| group | hud | 17.6 | 14.4 | 32.0 | 10.3% |
| group | panels | 0.0 | 0.0 | 0.0 | 0.0% |
| group | map | 13.9 | 2.7 | 16.6 | 5.3% |
| group | overlays | 0.0 | 1.1 | 1.1 | 0.4% |
| group | frame | 0.0 | 39.0 | 39.0 | 12.5% |
| `slipway_draw_method` | slipways | 19.3 | 74.8 | 94.1 | 30.2% |
| `planet_draw_method` | planets | 20.1 | 22.7 | 42.8 | 13.7% |
| `ascension_draw_method` | planets | 7.4 | 26.8 | 34.2 | 11.0% |
| `score_panel_draw_method` | hud | 11.4 | 6.8 | 18.2 | 5.9% |
| `draw_layers` | frame | 0.0 | 17.8 | 17.8 | 5.7% |
| `background_draw_method` | map | 13.9 | 2.7 | 16.6 | 5.3% |
| `emitter_update_method` | particles | 1.6 | 11.7 | 13.3 | 4.3% |
| `button_draw_method` | hud | 5.2 | 6.6 | 11.8 | 3.8% |
| `entity_geom_is_on_screen` | frame | 0.0 | 11.0 | 11.0 | 3.5% |
| `entity_pos` | frame | 0.0 | 7.0 | 7.0 | 2.3% |
| `structure_icon_draw_method` | planets | 2.4 | 1.8 | 4.2 | 1.3% |
| `slipway_draw_overlay_method` | slipways | 0.0 | 4.0 | 4.0 | 1.3% |
| `toast_draw_method` | hud | 1.0 | 0.5 | 1.5 | 0.5% |
| functions under 0.5% each | | | | | 1.4% |
| update `update_entities` | | | | 8.6 | 2.8% |
| update `ui_update` | | | | 7.6 | 2.5% |
| update `input_step_run` | | | | 5.2 | 1.7% |
| update `other` | | | | 0.9 | 0.3% |
| submit | | | | 6.0 | 1.9% |

`adds-up ok sum 309.4 measured 311.6 residual -2.2 noise-floor 3.1`

#### `tech-panel-5895` (source of tech-panel: representative run)

`window run-frames 65-73 frames 9 overhead-per-call 0.0558 calls 5766`  

| line | group | in-primitives | game-code | total | share |
|---|---|--:|--:|--:|--:|
| group | planets | 30.3 | 49.3 | 79.5 | 21.7% |
| group | slipways | 16.9 | 66.9 | 83.8 | 22.8% |
| group | particles | 3.0 | 11.2 | 14.2 | 3.9% |
| group | hud | 27.9 | 26.4 | 54.3 | 14.8% |
| group | panels | 32.7 | 8.7 | 41.4 | 11.3% |
| group | map | 9.0 | 2.8 | 11.8 | 3.2% |
| group | overlays | 0.0 | 1.3 | 1.3 | 0.4% |
| group | frame | 0.0 | 41.2 | 41.3 | 11.3% |
| `slipway_draw_method` | slipways | 16.9 | 62.7 | 79.6 | 21.7% |
| `message_box_draw_overlay_method` | panels | 32.7 | 8.7 | 41.4 | 11.3% |
| `ascension_draw_method` | planets | 10.1 | 30.4 | 40.5 | 11.0% |
| `planet_draw_method` | planets | 17.9 | 17.4 | 35.3 | 9.6% |
| `button_draw_method` | hud | 11.2 | 14.4 | 25.6 | 7.0% |
| `draw_layers` | frame | 0.0 | 18.9 | 18.9 | 5.2% |
| `score_panel_draw_method` | hud | 10.8 | 7.4 | 18.2 | 5.0% |
| `emitter_update_method` | particles | 3.0 | 11.2 | 14.2 | 3.9% |
| `background_draw_method` | map | 9.0 | 2.8 | 11.8 | 3.2% |
| `entity_geom_is_on_screen` | frame | 0.0 | 10.9 | 10.9 | 3.0% |
| `button_draw_overlay_method` | hud | 5.9 | 4.5 | 10.4 | 2.8% |
| `entity_pos` | frame | 0.0 | 8.0 | 8.0 | 2.2% |
| `slipway_draw_overlay_method` | slipways | 0.0 | 4.2 | 4.2 | 1.1% |
| `structure_icon_draw_method` | planets | 2.3 | 1.5 | 3.8 | 1.0% |
| functions under 0.5% each | | | | | 1.3% |
| update `input_step_run` | | | | 13.8 | 3.8% |
| update `ui_update` | | | | 8.5 | 2.3% |
| update `update_entities` | | | | 8.4 | 2.3% |
| update `other` | | | | 1.1 | 0.3% |
| submit | | | | 7.8 | 2.1% |

`adds-up ok sum 367.3 measured 366.9 residual 0.4 noise-floor 3.7`

#### `score-panel` (source of score-panel: `no-run`, the full game's score-panel frames over 263)

`flag no-run`  
`no-run score-panel frames 8 budget 263 shares pc-time`  

| line | group | total | share |
|---|---|--:|--:|
| group | planets | 27.3 | 7.6% |
| group | slipways | 38.3 | 10.6% |
| group | particles | 3.9 | 1.1% |
| group | hud | 35.9 | 10.0% |
| group | panels | 97.6 | 27.1% |
| group | map | 33.8 | 9.4% |
| group | overlays | 0.2 | 0.1% |
| group | frame | 10.9 | 3.0% |
| `message_box_draw_overlay_method` | panels | 97.6 | 27.1% |
| `slipway_draw_method` | slipways | 37.3 | 10.3% |
| `background_draw_method` | map | 33.7 | 9.4% |
| `score_panel_draw_method` | hud | 23.4 | 6.5% |
| `planet_draw_method` | planets | 16.6 | 4.6% |
| `ascension_draw_method` | planets | 6.9 | 1.9% |
| `toast_draw_method` | hud | 6.8 | 1.9% |
| `button_draw_method` | hud | 5.6 | 1.5% |
| `draw_layers` | frame | 4.7 | 1.3% |
| `emitter_update_method` | particles | 3.9 | 1.1% |
| `entity_geom_is_on_screen` | frame | 3.3 | 0.9% |
| `burst_draw_method` | planets | 2.0 | 0.6% |
| `entity_pos` | frame | 1.8 | 0.5% |
| `structure_icon_draw_method` | planets | 1.8 | 0.5% |
| functions under 0.5% each | | | 0.6% |
| update `input_step_run` | | 52.8 | 14.6% |
| update `update_entities` | | 43.0 | 11.9% |
| update `ui_update` | | 9.6 | 2.7% |
| update `other` | | 1.4 | 0.4% |
| submit | | 5.9 | 1.6% |

`adds-up n/a no-run sum 360.5 measured 360.5`

#### `score-panel-5876` (best try for score-panel: PSX per-call costs and splits only)

`flag no-representative-run score-panel`  
`window run-frames 46-46 frames 1 overhead-per-call 0.0558 calls 496`  

| line | group | in-primitives | game-code | total | share |
|---|---|--:|--:|--:|--:|
| group | planets | 23.6 | 37.2 | 60.9 | 17.9% |
| group | slipways | 15.5 | 59.0 | 74.6 | 21.9% |
| group | particles | 1.9 | 7.0 | 8.9 | 2.6% |
| group | hud | 9.7 | 9.9 | 19.7 | 5.8% |
| group | panels | 24.4 | 9.0 | 33.3 | 9.8% |
| group | map | 8.2 | 2.6 | 10.8 | 3.2% |
| group | overlays | 0.0 | 1.0 | 1.0 | 0.3% |
| group | frame | 0.0 | 36.1 | 36.2 | 10.6% |
| `slipway_draw_method` | slipways | 15.5 | 54.9 | 70.4 | 20.7% |
| `message_box_draw_overlay_method` | panels | 24.4 | 9.0 | 33.4 | 9.8% |
| `planet_draw_method` | planets | 14.5 | 16.0 | 30.5 | 8.9% |
| `ascension_draw_method` | planets | 7.2 | 19.8 | 27.0 | 7.9% |
| `draw_layers` | frame | 0.0 | 15.3 | 15.3 | 4.5% |
| `score_panel_draw_method` | hud | 7.9 | 6.3 | 14.2 | 4.1% |
| `background_draw_method` | map | 8.2 | 2.6 | 10.8 | 3.2% |
| `entity_geom_is_on_screen` | frame | 0.0 | 10.3 | 10.3 | 3.0% |
| `emitter_update_method` | particles | 1.9 | 7.0 | 8.9 | 2.6% |
| `entity_pos` | frame | 0.0 | 7.2 | 7.2 | 2.1% |
| `button_draw_method` | hud | 1.9 | 3.4 | 5.3 | 1.5% |
| `slipway_draw_overlay_method` | slipways | 0.0 | 4.1 | 4.1 | 1.2% |
| `structure_icon_draw_method` | planets | 2.0 | 1.4 | 3.4 | 1.0% |
| functions under 0.5% each | | | | | 1.4% |
| update `input_step_run` | | | | 47.7 | 14.0% |
| update `update_entities` | | | | 34.0 | 10.0% |
| update `ui_update` | | | | 8.1 | 2.4% |
| update `other` | | | | 1.2 | 0.3% |
| submit | | | | 6.0 | 1.8% |

`adds-up ok sum 342.3 measured 341.0 residual 1.3 noise-floor 3.4`

## Functions above 2%

A function's cost per steady frame is its hblanks in each steady situation
with frames over, taken from that situation's source and weighted by the
situation's steady frames (map-rest 1,564, map-move 1,907, slipway-aim 111,
build 366, menu 252, tech-panel 47, score-panel 17; `title`'s 12 count as
zero), divided by 4,276. Rule (a): that cost is above T = 4.4. Rule (b):
in one situation, above 2% of its source's measured total (map-rest 308.3,
map-move 352.0, slipway-aim 330.6, build 311.3, menu 311.6, tech-panel
366.9, score-panel 360.5). **19 functions**: 16 by rule (a), 3 by rule (b)
only.

| function | per steady frame | rule | map-rest | map-move | slipway-aim | build | menu | tech-panel | score-panel |
|---|--:|---|--:|--:|--:|--:|--:|--:|--:|
| `slipway_draw_method` | 43.3 | (a) | 56.8 | 19.7 | 62.9 | 65.0 | 94.1 | 79.6 | 37.3 |
| `wormhole_draw_method` | 43.1 | (a) | 1.0 | 95.8 | 0.0 | 0.2 | 0.0 | 0.0 | 0.1 |
| update `update_entities` | 35.1 | (a) | 17.2 | 61.1 | 6.7 | 7.6 | 8.6 | 8.4 | 43.0 |
| `background_draw_method` | 32.0 | (a) | 54.3 | 13.0 | 47.4 | 44.8 | 16.6 | 11.8 | 33.7 |
| `planet_draw_method` | 26.1 | (a) | 25.2 | 24.3 | 26.8 | 28.1 | 42.8 | 35.3 | 16.6 |
| `score_panel_draw_method` | 22.5 | (a) | 30.2 | 16.0 | 27.1 | 26.0 | 18.2 | 18.2 | 23.4 |
| update `ui_update` | 19.2 | (a) | 15.2 | 21.5 | 57.0 | 23.8 | 7.6 | 8.5 | 9.6 |
| `message_box_draw_overlay_method` | 16.5 | (a) | 30.2 | 0.0 | 41.7 | 41.3 | 0.0 | 41.4 | 97.6 |
| update `input_step_run` | 14.1 | (a) | 12.5 | 17.6 | 12.9 | 7.7 | 5.2 | 13.8 | 52.8 |
| update `save_at_frame_end` | 11.8 | (a) | 4.2 | 23.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| `emitter_update_method` | 9.9 | (a) | 5.2 | 14.6 | 3.1 | 5.3 | 13.3 | 14.2 | 3.9 |
| `entity_geom_is_on_screen` | 8.9 | (a) | 4.9 | 13.1 | 4.4 | 4.1 | 11.0 | 10.9 | 3.3 |
| `draw_layers` | 8.7 | (a) | 6.8 | 9.7 | 5.7 | 5.8 | 17.8 | 18.9 | 4.7 |
| `ascension_draw_method` | 7.8 | (a) | 11.2 | 0.0 | 3.4 | 12.7 | 34.2 | 40.5 | 6.9 |
| `button_draw_method` | 7.3 | (a) | 8.4 | 5.4 | 7.2 | 7.0 | 11.8 | 25.6 | 5.6 |
| submit | 6.1 | (a) | 6.0 | 5.9 | 6.0 | 7.3 | 6.0 | 7.8 | 5.9 |
| `structure_icon_draw_method` | 3.2 | situation-only (build) | 2.6 | 1.6 | 2.0 | 13.9 | 4.2 | 3.8 | 1.8 |
| `entity_pos` | 2.9 | situation-only (menu, tech-panel) | 2.7 | 2.4 | 2.4 | 2.6 | 7.0 | 8.0 | 1.8 |
| `button_draw_overlay_method` | 0.3 | situation-only (tech-panel) | 0.1 | 0.2 | 0.1 | 0.1 | 0.3 | 10.4 | 0.1 |

Primitives/game-code split: game code as a share of the function's total
in each run with a breakdown boot. For map-rest, build and score-panel it
comes from the best try and is applied to the `no-run` total ("split from
<best-try>"). Slipway-aim has no split.

| function | map-rest (5581) | map-move | build (5752) | menu | tech-panel | score-panel (5876) |
|---|--:|--:|--:|--:|--:|--:|
| `slipway_draw_method` | 77% | 75% | 80% | 79% | 79% | 78% |
| `wormhole_draw_method` | — | 2% | — | — | — | — |
| `background_draw_method` | 23% | 20% | 19% | 16% | 24% | 24% |
| `planet_draw_method` | 44% | 49% | 48% | 53% | 49% | 52% |
| `score_panel_draw_method` | 37% | 38% | 34% | 37% | 41% | 44% |
| `message_box_draw_overlay_method` | 24% | — | — | — | 21% | 27% |
| `emitter_update_method` | 84% | 78% | 86% | 88% | 79% | 79% |
| `entity_geom_is_on_screen` | 100% | 100% | 100% | 100% | 100% | 100% |
| `draw_layers` | 100% | 100% | 100% | 100% | 100% | 100% |
| `ascension_draw_method` | 73% | — | 78% | 78% | 75% | 73% |
| `button_draw_method` | 61% | 63% | 58% | 56% | 56% | 64% |
| `structure_icon_draw_method` | 38% | 50% | 36% | 43% | 39% | 41% |
| `entity_pos` | 100% | 100% | 100% | 100% | 100% | 100% |
| `button_draw_overlay_method` | — | — | — | — | 43% | — |

`no-run` against best-try shares, for each function above that differs by
more than a fifth of its `no-run` share (|best − no-run| > no-run ÷ 5;
pairs where both are under 1% are left out):

| function | situation | no-run share | best-try share |
|---|---|--:|--:|
| `slipway_draw_method` | score-panel | 10.3% | 20.6% |
| update `update_entities` | map-rest | 5.6% | 1.9% |
| update `update_entities` | build | 2.4% | 3.2% |
| `background_draw_method` | map-rest | 17.6% | 3.9% |
| `background_draw_method` | build | 14.4% | 5.9% |
| `background_draw_method` | score-panel | 9.3% | 3.2% |
| `planet_draw_method` | map-rest | 8.2% | 12.1% |
| `planet_draw_method` | build | 9.0% | 12.1% |
| `planet_draw_method` | score-panel | 4.6% | 8.9% |
| `score_panel_draw_method` | map-rest | 9.8% | 5.7% |
| `score_panel_draw_method` | score-panel | 6.5% | 4.2% |
| update `ui_update` | map-rest | 4.9% | 2.1% |
| update `ui_update` | build | 7.6% | 13.7% |
| `message_box_draw_overlay_method` | map-rest | 9.8% | 14.8% |
| `message_box_draw_overlay_method` | build | 13.3% | 0.0% |
| `message_box_draw_overlay_method` | score-panel | 27.1% | 9.8% |
| update `input_step_run` | map-rest | 4.1% | 2.7% |
| update `input_step_run` | build | 2.5% | 3.3% |
| update `save_at_frame_end` | map-rest | 1.4% | 0.0% |
| `emitter_update_method` | map-rest | 1.7% | 5.3% |
| `emitter_update_method` | score-panel | 1.1% | 2.6% |
| `entity_geom_is_on_screen` | map-rest | 1.6% | 3.4% |
| `entity_geom_is_on_screen` | build | 1.3% | 4.1% |
| `entity_geom_is_on_screen` | score-panel | 0.9% | 3.0% |
| `draw_layers` | map-rest | 2.2% | 4.8% |
| `draw_layers` | build | 1.9% | 5.9% |
| `draw_layers` | score-panel | 1.3% | 4.5% |
| `ascension_draw_method` | map-rest | 3.6% | 9.7% |
| `ascension_draw_method` | build | 4.1% | 7.0% |
| `ascension_draw_method` | score-panel | 1.9% | 7.9% |
| `button_draw_method` | map-rest | 2.7% | 1.8% |
| `structure_icon_draw_method` | map-rest | 0.8% | 1.2% |
| `structure_icon_draw_method` | build | 4.5% | 1.6% |
| `entity_pos` | map-rest | 0.9% | 2.9% |
| `entity_pos` | build | 0.8% | 2.8% |
| `entity_pos` | score-panel | 0.5% | 2.1% |

The places below were found with `make frame-profile-subtree
ROOT=<symbol> PSX_SHORT_RUNS=<run>` (K = 2, `PART=update` for update
pieces) on the situation's run, or on its best try. Shares are of the
root's PC time.

### What each function gets

| function | lever | why |
|---|---|---|
| `slipway_draw_method` | N1, then N2 and N5 on what it leaves | `slipway_draw_line` ×2 is 41% of `slipway_draw` (menu-5954), and half of that is integer conversion and fixed-point adds around `p8_lines` |
| `wormhole_draw_method` | N3 | 94.1 of its 95.8 h are in `circ`, 20.2 h a call |
| update `update_entities` | N2 only | helper calls (`p8_fixed_add`, `p8_fixed_from_int`, `entity_has`) are 27% of it on map-move-1057. The rest is per-entity state handlers that change from window to window (`wormhole_update_unfolding` 36% on map-move-1057, `score_panel_update` 42% on score-panel-5876) |
| `background_draw_method` | no pixel-identical lever found | the grid (18 pattern fills and one rect, 3.6–5.6 h a frame) reaches 262, past the 256-pixel background image, so baking it in still leaves 18 stub fills. The image itself is one GPU copy. Its `no-run` 32–54 h is mostly PC blit time (Known biases). See picture-changing idea P1 |
| `planet_draw_method` | N2 only | every primitive is already one call (19 `spr`, 15 row images, 8 image copies a frame on menu-5954; D6, D8, D9 taken) |
| `score_panel_draw_method` | no pixel-identical lever found | D4's residual is 2.6 h a frame, under T (Plan 21's levers) |
| update `ui_update` | N6 | in build-5752, `targeting_track`'s slipway filter is 53% of 98% of `ui_handle_button`, which is 74% of `ui_update` |
| `message_box_draw_overlay_method` | D3 (taken) | 13 outlined `print` a frame at about 1.7–1.8 h each; through the text image cache, −7.8 h a frame on tech-panel-5895 |
| update `input_step_run` | N2 only | `pad_nav_step`'s panel questions are already memoised (O2). What remains is spread over `run_step`'s helpers (`foreign_panel_open` 40%, `bar_or_cycle_step` 39% on score-panel-5876): update code, each under T outside score-panel's 8 frames |
| update `save_at_frame_end` | no pixel-identical lever found | `save_state_encode` 56% and `save_state_capture` 39% on map-move-1057, once per save. U4 is taken, and the rest is the save itself (spreading work across frames is out of scope) |
| `emitter_update_method` | N2 only | helper calls are about 45% of `emitter_update`'s PC time. Its 48% self is the particle simulation (D14: no candidate), and D15 is taken |
| `entity_geom_is_on_screen` | N5 (after N2) | 145 calls a frame on menu-5954. Each looks up the main camera and rebuilds the screen bounds, and 57% of its PC time is helper calls |
| `draw_layers` | no pixel-identical lever found | L1 is taken. Its self is the walk and dispatch (2.8% of its PC time on menu-5954), and L2 and L3 are dropped |
| `ascension_draw_method` | N2 only | 71% is `emitter_update` and 27% `planet_draw` (menu-5954), both above |
| `button_draw_method` | N7 | 24 `rectfill`, 10 `rect` and 23 `spr` a frame on tech-panel-5895 for 10 buttons |
| submit | G4 (taken) | the slot uploads in `psx_gpu_frame_end`: 95% of them wrote words their slot already held; skipping them took 55.6–61.4% of submit on the representative runs |
| `structure_icon_draw_method` (build) | no pixel-identical lever found | one `spr` per icon (6 a frame); `p8_spr_default` is 67% of it |
| `entity_pos` (menu, tech-panel) | N2 only | a vector getter: call overhead only |
| `button_draw_overlay_method` (tech-panel) | no pixel-identical lever found | `tooltip_draw_at`'s `hud_draw_bar` (73%) is sprites, and its text rows already go through the text image cache (D2) |

## Ranked levers

The order is the one `make lever-estimate` computes (greedy, steady frames
over first). "Alone" is what the lever removes by itself, nominal, on the
committed table. Percentages are of that part of the situation's frame in
its source: run window, or `no-run` frames over. Draw parts: map-rest 251.8,
map-move 222.7, slipway-aim 247.1, build 262.9, menu 281.0, tech-panel
327.6, score-panel 247.9. Update parts: 50.5, 125.5, 77.6, 41.1, 22.3,
31.8, 106.8. Submit 5.9–7.8. Costs: S under 50 changed lines in one module;
M 50–300 lines or two to three modules; L above 300 lines, or a new
runtime or backend entry.

### 1. N1 — slipway lines from integer end points

- **Function:** `slipway_draw_method` (slipways).
- **Location:** `slipway_draw_line`, `src/game/slipway_display.c`.
- **Change:** the offsets `dx`, `dy` are whole numbers, so `flr(p + d) =
  flr(p) + d`. Floor the two end points once, then build the four segments
  with integer adds. This replaces two fixed-point loops with 16 adds and
  16 `p8_fixed_flr_int` calls per pass. The `p8_lines` calls, segments and
  order stay the same.
- **Estimated gain (`share`):** in `slipway_draw_line` (menu-5954), `p8_lines`
  is 48.9%, the integer conversions 13.6%, the fixed adds 13.1% and its self
  24.6%. The change removes about 80% of the non-`p8_lines` half, about
  15% of `slipway_draw_method`. That gives map-rest 8.5 h (3.4% of draw),
  map-move 3.0 (1.3%), slipway-aim 9.4 (3.8%), build 9.8 (3.7%), menu 14.1
  (5.0%), tech-panel 11.9 (3.6%), score-panel 5.6 (2.3%). This function is
  call-heavy (PC hook bias), so quote the pessimistic figure.
- **Residual before the change (plan 22 step 3.7):** a throwaway probe
  running the segment loops a second time into a `volatile` sink cost
  map-move-1057 +2.5 h a window frame, menu-5954 +10.3, tech-panel-5895
  +8.0: 12.9%, 11.0% and 10.1% of `slipway_draw_method`, pooled 10.8%.
  Per steady frame 4.9 h, above T = 4.4 (rule (a)); menu and tech-panel
  above 2% of their totals (rule (b)). Not covered by any taken lever.
- **Measured gain (taken, plan 22 step 3.7):** `psx-probe`, game state
  `same` on every run. map-move-1057 draw 3,308 → 3,268 (−40 over 15
  frames, −2.7 h a frame, 1.2% of draw) against 3.0 h (1.3%); menu-5954
  4,251 → 4,073 (−178 over 15, −11.9 h, 4.2%) against 14.1 h (5.0%);
  tech-panel-5895 2,878 → 2,797 (−81 over 9, −9.0 h, 2.8%) against 11.9 h
  (3.6%). Pooled ratio 23.5 / 29.0 = 0.81: map-rest, build, slipway-aim and
  score-panel get their estimates × 0.81 (2.8%, 3.0%, 3.1%, 1.9%). This is
  N1's own ratio; with N2's and G4's it sets the `scale share` line (1.11, their median). map-move-1057's −2.7 h is under its 3.5 h noise floor.
  Weaker evidence: map-rest-5581 −120 over 15 frames (−8.0 a frame),
  build-5752 −119 over 15 (−7.9), score-panel-5876 −9 on its one frame;
  update within ±4 h on every run; text −236 bytes.
- **Cost:** S. **Risk:** pixel identity `none` (same segments, same
  calls). Game state `none` (draw only).
- **Measured by:** `map-move-1057` (12 of the representative situations'
  frames). **Evidence:** `weaker`: alone 67 frames (estimate), 49 of them
  in map-rest (30), build (16) and slipway-aim (3). Measured, alone it
  removes 56. It is in the code, and no later step takes it again.

### 2. N2 — fixed-point and vector helpers inlined (after N1)

- **Function:** game code everywhere: `update_entities`, `emitter_update_method`,
  `ascension_draw_method`, `planet_draw_method`, `entity_pos`,
  `input_step_run` and others.
- **Location:** `src/p8/fixed.c` → `src/p8/fixed.h`, and `src/engine/vec.c` →
  `src/engine/vec.h`, for the one-line helpers (`p8_fixed_add`, `_sub`,
  `_from_int`, `_flr_int`, `_abs`, `_neg`, `vec_add`, `vec_sub`, …).
- **Change:** move their bodies into the headers as `static inline`. The
  PSX build has no LTO, so today every fixed-point add is a `jal` plus the
  caller's register spills.
- **Estimated gain (`share`):** helper calls are 27% (`update_entities`), about
  45% (`emitter_update`), 57% (`entity_geom_is_on_screen`) and about 12%
  (`slipway_draw`, without `slipway_draw_line`) of these functions' PC
  time. PC shares carry residual hook cost, and an inlined helper still
  does its arithmetic, so a third of it is counted: 7% of game-code time,
  in draw and in update. Draw: map-rest 10.9 h (4.3%), map-move 5.8
  (2.6%), slipway-aim 10.2 (4.1%; pooled representative split), build 12.6
  (4.8%), menu 13.9 (5.0%), tech-panel 14.5 (4.4%), score-panel 11.4
  (4.6%). Update: 7% everywhere. The figures are stated on what N1 leaves:
  `slipway_draw_line`'s helpers are left out.
- **Done (plan 22 step 3.7):** `p8_fixed_from_int`, `_add`, `_sub`,
  `_mul`, `_neg`, `_flr`, `_flr_int`, `_abs`, `_sgn`, `_band`, `_bor` and
  `vec_make`, `vec_add`, `vec_sub`, `vec_mul`, `vec_neg`, `vec_dot`,
  `vec_length_squared`, `vec_perpendicular` are `static inline` in their
  headers, bodies unchanged; division, square root, random, sine, cosine
  and the vector functions on them stay out of line.
- **Under 2%? No:** N2's own probe delta is its remaining cost (only the
  change removes call overhead). Per steady frame 31.4 h (rule (a), T =
  4.4); map-move-1057 23.1 h a frame, menu-5954 45.0, tech-panel-5895
  47.3, each far above 2% of its run's total (rule (b): 7.0, 6.2, 7.3).
  Not covered: N1 removed helper calls only inside `slipway_draw_line`.
- **Measured gain (taken, plan 22 step 3.7):** `psx-probe` against the
  baseline at N1's commit, game state `same` on all 12 runs.
  map-move-1057 draw 3,268 → 3,027 (−241 over 15 frames, −16.1 h a frame,
  7.4% of draw) against 5.8 h (2.6%), update 1,882 → 1,776 (−106, −7.1 h,
  5.6%) against 8.8 h (7%); menu-5954 draw 4,073 → 3,431 (−642, −42.8 h,
  15.8%) against 13.9 h (5.0%), update 333 → 300 (−33, −2.2 h, 9.9%)
  against 1.6 h; tech-panel-5895 draw 2,797 → 2,397 (−400 over 9, −44.4 h,
  14.3%) against 14.5 h (4.4%), update 283 → 257 (−26, −2.9 h, 9.2%)
  against 2.2 h. Submit within ±1 hblank on the window. Draw ratio
  103.3 / 34.2 = 3.02, update ratio 12.2 / 12.6 = 0.96: map-rest, build,
  slipway-aim and score-panel get their draw estimates × 3.02 (13.0%,
  14.5%, 12.4%, 13.9%) and 7% × 0.96 = 6.8% of update. N2's own ratio,
  pooled: 115.5 / 46.8 = 2.47. The PC-share estimate counted a third of
  the helpers' share as PSX cost; on PSX the call overhead (spills, the
  `vec_t` hidden return pointer, unfolded constant conversions) is far
  more. Weaker evidence: map-rest-5581 draw −543 over 15 (−36.2 a frame,
  14.7%) and update −29 (9.6%); build-5752 draw −362 (−24.1, 13.5%) and
  update −256 (−17.1, 34.5%); score-panel-5876 draw −34 (12.5%) and update
  −4 (4.4%) on its one frame; start −1,681 h of update on its one frame;
  update-4737 −30, update-5500 −44, update-5719 −3 of update; draw-5564
  −508 and draw-6000 −670 of draw over 15. Text −7,712 bytes.
- **Cost:** M (two modules). GCC's `-finstrument-functions` still
  instruments the inlined bodies, so the PC profiler keeps seeing the
  helpers as children, now one local copy per translation unit: a
  `frame-profile-subtree ROOT=` or drill-down on one of them meets several
  text symbols and reports it ambiguous, as for `entity_pos`.
- **Risk:** pixel identity `none` (same arithmetic). Game state `e2e
  decides` (update code is recompiled): every e2e hash and `.end-state`
  unchanged.
- **Measured by:** `map-move-1057`. **Evidence:** `representative`.
  Measured, alone it removes 453 frames (estimate: 157): map-rest 174,
  map-move 127, build 67, menu 64, slipway-aim 13, tech-panel 7,
  score-panel 1. It is in the code, and no later step takes it again.

### 3. N3 — PSX circle outline drawn as runs

- **Function:** `wormhole_draw_method` (map), and the `circ` of
  `planet_draw_method`.
- **Location:** `psx_tiles_circ`, `src/platform/psx_tiles.c`, called by
  `gpu_circ`, `src/platform/psx_gpu.c`.
- **Change:** today each midpoint step emits up to 8 one-pixel `TILE_1`. A
  28-comparison duplicate check and a clip run per pixel. Instead, emit
  each octant's horizontal or vertical runs as one `TILE` each (same
  pixels, same colour). Run the duplicate check only on the two steps
  where points can coincide, and clip each run once. This cuts the packets
  by about 60% and drops the per-pixel loop work. The PC test compares the
  pixel set with the software `circ`.
- **Estimated gain (`calls`):** 60% of the per-call cost (from the code: about
  540 cycles a midpoint step, against about 200 for the run form), times
  the full game's `circ` calls per frame over in the situation. Map-move:
  294 calls on 388 frames over × 20.158 h × 0.6 = 9.2 h (4.1% of draw).
  The window's own 4.7 calls a frame would give 56.8 h (25%), but only 62
  of map-move's 388 frames over draw a circle (Known biases). Map-rest:
  246/429 × 5.716 (map-rest-5581) × 0.6 = 2.0 h (0.8%). Menu: 10/91 ×
  20.158 (pooled representative) × 0.6 = 1.3 h (0.5%). Score-panel: 3/8 ×
  20.158 × 0.6 = 4.5 h (1.8%).
- **Done (plan 22 step 3.7):** `psx_tiles_circ` emits each step group's
  pixels with `|dx| >= |dy|` as column runs and the others as row runs,
  each run clipped once, no duplicate check; `gpu_circ` sends a lone pixel
  as a `TILE_1`, a longer run as a `TILE`. Packets per whole ring: radius
  24 128 → 64, radius 9 44 → 28, radius 26 140 → 68, radii 0–4 unchanged.
  Host-tested against the software `circ` pixel for pixel (pinned counts,
  200 random circles cut by every edge). `psx-check` captures a ring cut
  by the top edge (`circ 245`, accepted at differ 0 before the change).
- **Under 2%? No:** a throwaway probe sending every circle twice cost
  map-move-1057 +1,414 h over 15 frames (+94.3 h a window frame, 20.1 h a
  `circ` at 4.7 a frame), map-rest-5581 +91 over 15 (+6.1), every other
  run within ±16 over its window. Rule (b): 94.3 above 2% of the run's
  total (6.5). Rule (a), an upper bound (frames-over basis): 20.1 h a call
  × each situation's circles per frame over, weighted by steady frames,
  11.1 h (8.2 with map-rest-5581's own 6.1 h a call), above T = 4.4. Not
  covered by D3, N1, N2 or G4.
- **Measured gain (taken, plan 22 step 3.7):** `psx-probe` against the
  baseline at the `circ 245` capture's commit, game state `same` on all
  12 runs. map-move-1057 draw 3,023 → 2,369 (−654 over 15 frames, −43.6 h
  a frame, 21.6% of draw), total −657, against the window's own estimate
  4.7 × 20.158 × 0.6 = 56.8 h: ratio 0.77 (46% of the measured circle
  cost, against the 60% assumed). Every situation's gain is its estimate ×
  0.77: map-move 3.2%, map-rest 0.6%, menu 0.4%, score-panel 1.4%. With
  D3's 0.42 it sets the `scale calls` line (0.60, their median). Weaker
  evidence: map-rest-5581 draw −22 over 15 (−1.5 h a frame against 3.4,
  ratio 0.43, under its 3 h noise floor); every other run within ±5 h over
  its window. Text +368 bytes.
- **Cost:** M (PSX backend only, one module and its test).
- **Risk:** pixel identity `none`. A `TILE` of w×1 or 1×h writes what the
  `TILE_1` it replaces wrote, with the same mask bit. Game state `none`
  (PSX only).
- **Measured by:** `map-move-1057`. **Evidence:** `representative`: alone
  50 frames (estimate), 48 in map-move. Measured, alone it removes 36,
  all in map-move. It is in the code, and no later step takes it again.

### 4. N5 — on-screen test against the frame's screen box (after N2)

- **Function:** `entity_geom_is_on_screen` (frame).
- **Location:** `entity_geom_is_on_screen`, `src/engine/entity_geom.c`, and
  its callers `skip_draw_off_screen` (`draw_layers`) and `slipway_draw`.
- **Change:** compute the camera position and the screen box once per draw
  pass (the camera does not move during the draw). Then test each entity's
  position against it with no `main_camera()` lookup and no per-call
  constants.
- **Estimated gain (`share`):** after N2 inlines the helpers (a third of their
  57%), the camera lookup, `pos_of` and the bounds rebuild are about half
  of what is left: 40% of the function. Map-rest 2.0 h (0.8%), map-move 5.2
  (2.4%), slipway-aim 1.8 (0.7%), build 1.6 (0.6%), menu 4.4 (1.6%),
  tech-panel 4.4 (1.3%), score-panel 1.3 (0.5%). Not re-derived after
  N2: scaled by 1.11 (N1, N2, G4), map-rest 0.9%, map-move 2.7%,
  slipway-aim 0.8%, build 0.7%, menu 1.8%, tech-panel 1.4%, score-panel
  0.6%.
- **Cost:** S. **Risk:** pixel identity `none`. Game state `e2e decides`:
  the result gates `slipway_spawn_transit_dot`'s visual random draws, so
  it must stay exact.
- **Measured by:** `map-move-1057`. **Evidence:** `representative`: alone
  40 frames scaled (35 unscaled), 29 in map-move.

### 5. D3 — message-box text through the text image cache

- **Function:** `message_box_draw_overlay_method` (panels).
- **Location:** `message_box_draw_article`, `src/game/message_box.c`; its
  scalar rows go through `draw_functions_draw_row` (`print_outlined_entry`,
  `src/game/draw_functions.c`), and its layout rows through
  `layout_row_draw_at`, `src/game/layout.c`.
- **Change:** draw the box's outlined strings with
  `text_image_print_outlined` (D2's cache, 32 slots of 128×8), and its
  layout rows with `layout_row_draw_at_text_image`, at the same positions,
  per-article camera shake included.
- **Estimated gain (`calls`):** map-rest-5581 has 13 box `print` a frame at
  about 1.70 h each (20 at 1.377, less the score panel's 7 at 0.767). Each
  becomes one image copy at about the `spr` cost (0.406), which saves
  16.8 h a frame, 39.4% of the box's 42.6 h. Applied to each source's
  figure for the box: map-rest 11.9 h (4.7% of draw), build 16.3 (6.2%),
  slipway-aim 16.4 (6.6%), score-panel 38.5 (15.5%). Tech-panel, from its
  own calls (13 at about 1.83 h, image 0.396): 18.6 h (5.7%). Map-move and
  menu: 0, since their runs open no box.
- **Measured gain (taken, plan 22 step 3.7):** `psx-probe` on
  `tech-panel-5895`: window draw 2,948 → 2,878 h (−70 over 9 frames, −7.8 h
  a frame, 2.4% of draw) against the 18.6 h (5.7%) estimate, a ratio of
  0.42. The other situations' gains are their estimates × 0.42 (map-rest
  2.0%, build 2.6%, slipway-aim 2.8%, score-panel 6.5%). This is D3's own
  ratio; with N3's it sets the `scale calls` line (0.60, their median).
  Weaker evidence: `map-rest-5581` −94 h over 15 frames (−6.3 a frame),
  `draw-5564` −102 over 15, `score-panel-5876` +36 on its one frame (the
  box's text slots built that frame), `build-5752` +3 (no box). Game
  state `same` on every run; text +1,240 bytes.
- **Cost:** M. `pad_message_box` is `psx-check`'s second capture replay
  (year box f142 `print` and f143 `all`, score box f164 `all`, at
  differ 0): plan 21 skipped D3 because no capture showed box text.
- **Risk:** pixel identity `none`. Outlined glyph texels survive an image
  build and copy bit for bit (plan 21's step 3.15 spike), and the new
  capture checks it. Game state `none` (draw only). One-off text builds
  land when a box opens.
- **Measured by:** `tech-panel-5895`. Measured, alone it removes 35
  frames (estimate: 75): map-rest 15, build 14, slipway-aim 3,
  tech-panel 2, score-panel 1. Ranked 5th by the greedy order on its
  measured gain; it is in the code, and no later step takes it again.

### 6. G4 — upload only the slots that changed

- **Function:** submit.
- **Location:** `psx_gpu_frame_end`, `src/platform/psx_gpu.c`, over the
  new `src/platform/psx_slot_uploads.c`.
- **Change:** remember the words each palette CLUT line, pattern tile and
  pattern CLUT line last received, one record for both primitive buffers
  (their slot k lands on the same VRAM line or tile). Skip the
  `LoadImage` when the words are the same.
- **Estimated gain (`share`):** plan 20 measured the upload at 72–77% of
  submit. The caches refill in the same draw order each frame, so about
  70% of the slots were assumed to keep their content: 52% of submit,
  3.1–4.1 h a frame in every situation. This is plan 21's G4, re-judged in
  Plan 21's levers below.
- **Residual before the change (plan 22 step 3.7):** a throwaway probe
  uploading a second time every slot whose words equal what that VRAM slot
  already holds cost map-move-1057 +109 h over 15 frames (+7.3 h a frame),
  menu-5954 +109 over 15 (+7.3), tech-panel-5895 +89 over 9 (+9.9),
  map-rest-5581 +109 over 15 (+7.3), build-5752 +126 over 15 (+8.4);
  pooled representative 307 over 39 (+7.9) for slipway-aim and
  score-panel. Per steady frame 7.4 h, above T = 4.4 (rule (a)); every
  situation above 2% of its total (rule (b): 6.2, 7.0, 6.6, 6.2, 6.2,
  7.3, 7.2). Not covered by D3, N1 or N2. 95% of the run's uploads were
  identical to their slot (3,994 of 4,188 over the 12 runs; map-move-1057
  97%, menu-5954 95%, tech-panel-5895 96%), against the 70% assumed. The
  doubled uploads cost more than the whole baseline submit (6.0 h), so
  the residual is an upper bound, not a measure.
- **Measured gain (taken, plan 22 step 3.7):** `psx-probe`, game state
  `same` on all 12 runs. Submit map-move-1057 90 → 40 (−50 over 15 frames,
  −3.3 h a frame, 55.6% of submit), menu-5954 90 → 39 (−51, −3.4 h,
  56.7%), tech-panel-5895 70 → 27 (−43 over 9, −4.8 h, 61.4%), against
  52%. Pooled 144 / 250 = 57.6% of submit, ratio 57.6 / 52 = 1.11:
  map-rest, build, slipway-aim and score-panel get 57.6%. Draw and update
  within ±4 h on every run. Weaker evidence: map-rest-5581 −53 over 15,
  build-5752 −55 over 15, draw-5564 −59, draw-6000 −51, the one-frame
  runs −3 to −4 of submit. Text +580 bytes.
- **Guard:** asserts-on builds read back, every frame, the slots the
  previous frame drew with and assert they hold its words; `psx-check`'s
  `asserts-on` requires both GPU capture replays to be built with asserts.
- **Cost:** M. **Risk:** pixel identity `none` (a skipped upload writes
  words VRAM already holds). Game state `none` (PSX only).
- **Measured by:** `map-move-1057`. **Evidence:** `representative`.
  Measured, alone it removes 37 frames (estimate: 68 scaled, 36
  unscaled): map-move 15, build 11, map-rest 7, slipway-aim 2,
  tech-panel 2. It is in the code, and no later step takes it again.

### 7. N6 — slipway target test pre-rejected by its box (after N2)

- **Function:** update `ui_update` (build, slipway-aim).
- **Location:** `targeting_track_slipway_predicate` /
  `targeting_track`, `src/game/probe.c`; `geom_near_segment`,
  `src/engine/geom.c`.
- **Change:** reject a slipway whose end points' box, widened by the attach
  distance, cannot hold the cursor, before `geom_near_segment`'s
  projection. Stop the filter at the first match: only the first is read.
- **Estimated gain (`share`):** the filter is 53.5% of `targeting_track`
  (build-5752), which is 72% of `ui_update`. Assuming most slipways are
  rejected by the box, about 35% of `ui_update` goes, or 32.6% of what N2
  leaves. Build 7.8 h (18.9% of update), slipway-aim 18.6 h (23.9%; split
  from build-5752). Not re-derived after N2: scaled by 1.11 (N1, N2, G4),
  build 21.0%, slipway-aim 26.5% of update.
- **Cost:** S. **Risk:** pixel identity `none`. Game state `e2e decides`:
  fixed-point products wrap, so the pre-test must reject only what
  `geom_near_segment` rejects. Any hash change reverts it.
- **Measured by:** `psx-full-game-check`, **probe** `build-5752`: no
  representative run tracks a target (`ui_handle_button` is 0–2% of
  `ui_update` on map-move-1057, menu-5954 and tech-panel-5895), and build
  is where it removes the most (16 of 26 scaled). **Evidence:** `weaker`.

### 8. N7 — buttons drawn once through the image cache

- **Function:** `button_draw_method` (hud).
- **Location:** `button_draw`, `src/game/ui_button.c`, over `p8_image_build`
  / `p8_image_draw` with an `image_slot_t` per button.
- **Change:** build each button's frame (fills, border, icon, text image)
  into an image slot keyed by its look. Copy it each frame and rebuild it
  when the look changes (hover, enable).
- **Estimated gain (`calls`):** the `rectfill`, `rect` and `spr` calls of
  `button_draw_method` become one copy per button at the `spr` cost.
  Tech-panel: 24 × 0.139 + 10 × 0.256 + 23 × 0.396 − 10 × 0.396 = 11.1 h
  (3.4%). Menu: 13 × 0.125 + 6 × 0.262 + 8 × 0.388 − 6 × 0.388 = 4.0 h
  (1.4%). Map-move: 2.3 h (1.0%). Map-rest, build, score-panel from their
  best tries (8 `rectfill`, 4 `rect`, 4 `spr`): 2.1 h (0.8%). Slipway-aim:
  no call count, 0. Not re-derived: scaled by 0.60 (D3, N3), tech-panel
  2.0%, menu 0.8%, map-move 0.6%, map-rest, build and score-panel 0.5%.
- **Cost:** L (new slots and their VRAM room, a build per look change).
- **Risk:** pixel identity `capture reference`. Fills and rects copied from
  an image carry the PSX mask bit, as with D8, so this is acceptable only
  at `differ 0` on every capture of the label. Game state `none`. One-off
  builds land on hover changes.
- **Measured by:** `map-move-1057` (8 frames alone, tech-panel 3).
  **Evidence:** `representative`: alone 18 frames, 7 of them in map-rest,
  build and slipway-aim; scaled, 12 (map-move 8, build 2, tech-panel 2).

## The lever table

```lever-table
# lever <id> <estimated|measured|in-base> <S|M|L> basis <calls|share> measure <run|psx-full-game-check> [probe <run|none>] [after <id>]
# gain <id> <situation> <update|draw|submit> <percent>
# scale <calls|share> <ratio>
scale share 1.11
scale calls 0.60
lever N1 measured S basis share measure map-move-1057
gain N1 map-rest draw 2.8
gain N1 map-move draw 1.2
gain N1 build draw 3.0
gain N1 menu draw 4.2
gain N1 tech-panel draw 2.8
gain N1 score-panel draw 1.9
gain N1 slipway-aim draw 3.1
lever N2 measured M basis share measure map-move-1057 after N1
gain N2 map-rest draw 13.0
gain N2 map-move draw 7.4
gain N2 build draw 14.5
gain N2 menu draw 15.8
gain N2 tech-panel draw 14.3
gain N2 score-panel draw 13.9
gain N2 slipway-aim draw 12.4
gain N2 map-rest update 6.8
gain N2 map-move update 5.6
gain N2 build update 6.8
gain N2 menu update 9.9
gain N2 tech-panel update 9.2
gain N2 score-panel update 6.8
gain N2 slipway-aim update 6.8
lever N3 measured M basis calls measure map-move-1057
gain N3 map-move draw 3.2
gain N3 map-rest draw 0.6
gain N3 menu draw 0.4
gain N3 score-panel draw 1.4
lever N5 estimated S basis share measure map-move-1057 after N2
gain N5 map-rest draw 0.8
gain N5 map-move draw 2.4
gain N5 build draw 0.6
gain N5 menu draw 1.6
gain N5 tech-panel draw 1.3
gain N5 score-panel draw 0.5
gain N5 slipway-aim draw 0.7
lever D3 measured M basis calls measure tech-panel-5895
gain D3 map-rest draw 2.0
gain D3 build draw 2.6
gain D3 tech-panel draw 2.4
gain D3 score-panel draw 6.5
gain D3 slipway-aim draw 2.8
lever G4 measured M basis share measure map-move-1057
gain G4 map-rest submit 57.6
gain G4 map-move submit 55.6
gain G4 build submit 57.6
gain G4 menu submit 56.7
gain G4 tech-panel submit 61.4
gain G4 score-panel submit 57.6
gain G4 slipway-aim submit 57.6
lever N6 estimated S basis share measure psx-full-game-check probe build-5752 after N2
gain N6 build update 18.9
gain N6 slipway-aim update 23.9
lever N7 estimated L basis calls measure map-move-1057
gain N7 tech-panel draw 3.4
gain N7 menu draw 1.4
gain N7 map-move draw 1.0
gain N7 map-rest draw 0.8
gain N7 build draw 0.8
gain N7 score-panel draw 0.8
```

## Projection

From `make lever-estimate` on this document (the committed frame table, budget 263, one-off allowance 10,800; pessimistic is every gain × 0.8). After each lever, cumulative:

```
mean-cost steady 218.8 threshold 4.4
situation-threshold map-rest mean 215.2 threshold 4.3
situation-threshold map-move mean 207.6 threshold 4.2
situation-threshold slipway-aim mean 269.4 threshold 5.4
situation-threshold build mean 258.7 threshold 5.2
situation-threshold menu mean 239.1 threshold 4.8
situation-threshold tech-panel mean 281.5 threshold 5.6
situation-threshold score-panel mean 296.9 threshold 5.9
start steady-over 1132 one-off-vsyncs 1391
in-base none
lever 1 N1 measured basis share steady-over 1076 pessimistic 1088 saved 6754
lever 2 N2 measured basis share steady-over 597 pessimistic 701 saved 43463
lever 3 N3 measured basis calls steady-over 526 pessimistic 660 saved 46815
lever 4 N5 estimated basis share steady-over 467 pessimistic 609 saved 50855
lever 5 D3 measured basis calls steady-over 411 pessimistic 569 saved 54626
lever 6 G4 measured basis share steady-over 365 pessimistic 511 saved 58345
lever 7 N6 estimated basis share steady-over 351 pessimistic 494 saved 60535
lever 8 N7 estimated basis calls steady-over 336 pessimistic 482 saved 62089
situation title over 0 end 0 pessimistic 0
situation map-rest over 429 end 114 pessimistic 202
situation map-move over 387 end 130 pessimistic 163
situation slipway-aim over 41 end 18 pessimistic 21
situation build over 155 end 27 pessimistic 49
situation menu over 91 end 27 pessimistic 27
situation tech-panel over 21 end 14 pessimistic 14
situation score-panel over 8 end 6 pessimistic 6
order ok
end steady-over 336 pessimistic 482
nominal: real-game steady-over 336 frames, worst steady frame 3749 total 460 situation map-move
nominal: real-game one-off-over 631 frames, vsyncs 1391 of 10800
nominal: real-game-budget failed 336 steady frames over 263, one-off vsyncs 1391 of 10800
pessimistic: real-game steady-over 482 frames, worst steady frame 3749 total 469 situation map-move
pessimistic: real-game one-off-over 631 frames, vsyncs 1391 of 10800
pessimistic: real-game-budget failed 482 steady frames over 263, one-off vsyncs 1391 of 10800
```

**The pixel-identical list does not reach zero.** Nominal, it leaves 336 steady frames over 263 (pessimistic 482): map-move 130 (163), map-rest 114 (202), build 27 (49), menu 27 (27), slipway-aim 18 (21), tech-panel 14 (14), score-panel 6 (6). The worst steady frame becomes 3749 (`map-move`), 460 (469). D3, N1, N2, G4 and N3 are taken and measured, at 0.42, 0.81, 2.47, 1.11 and 0.77 of their estimates; the remaining `share` levers (N5, N6) are scaled by 1.11, the median of N1's, N2's and G4's ratios, and the remaining `calls` lever (N7) by 0.60, the median of D3's and N3's. The greedy order now puts N5 before D3 and N6 before N7: N3 measured at 36 frames alone leaves map-move's frames over for N5, and the 0.60 scale shrinks N7 to 12. Phase 3 still runs the whole list. The close then weighs the picture-changing ideas below for what is left.

## Picture-changing ideas (unranked, for the close)

None of these is pixel-identical, and nothing here is implemented by
plan 22's pass 1. Whether a visible change is acceptable is the human's
decision.

- **P1 — background grid baked into the image, cut at 256.** Put the grid
  and the border into the 256×256 background image. The grid's last
  column, last row and the stubs between 256 and 262 would no longer be
  drawn (or would be drawn differently). Saves the 18 pattern fills and
  the rect: 3.6–5.6 h a frame in every situation.
- **P2 — slipways without the black border pass.** `slipway_draw` draws
  four border segments then four line segments. Dropping the border, or
  drawing it as two segments, thins the outline. Saves up to half of the
  slipways' `line` calls: about 7 h a frame on menu-5954 and 6 h on
  map-rest-5581, plus their game code.
- **P3 — transit dots without their black 2×2 backing.** Drops the
  `rectfill` under each dot's `pset`, 48–80 a frame (map-rest-5581,
  menu-5954), so the dots lose their dark halo. Saves about 6–10 h a frame.
- **P4 — fewer wormhole reveal rings.** `wormhole_draw` draws up to five
  rings at 20 h each on the frames that show a wormhole. Drawing two or
  three changes the reveal animation. Saves up to 40–60 h on those frames
  (62 of map-move's frames over).
- **P5 — fewer particles.** Halve what the emitters spawn
  (`ascension_draw_method`, `emitter_update_method`). Saves about half of
  7–20 h a frame on menu and tech-panel. The particle density changes.

## Plan 21's levers

**Taken, in the measured code, not proposed again:** F1, G1, B1, G2, B3,
D5, D1/B4, L1, D8, D6, D2, D10, D9, D11, D15, O2, O3, O5, U4, U6, U2, U1,
U3. **D11 stays**, as plan 22 decided.

**Re-judged** against T = 4.4 and the steady profiles (plan 21's figures
are quoted for comparison only):

| lever | verdict | figure |
|---|---|---|
| D3 message-box text | taken (plan 22 step 3.7), 6th by measured gain | estimated 11.9–38.5 h a frame where a box is open (`calls`), measured 7.8 h on tech-panel-5895 (ratio 0.42); plan 21 residual 36.1 / 0 |
| D4 score-panel readouts | dropped | 7 `print` a frame through the text image saves 7 × (0.767 − 0.395) = 2.6 h (map-move-1057), under T; plan 21 residual 9.5 / 9.3 |
| D12 transit dot as a 2×2 sprite | dropped | a `spr` costs 0.36–0.41 h a call on every run, more than the `rectfill` + `pset` pair it would replace (0.16–0.22 h): a loss, before its capture-reference risk |
| D13 planet visibility reused in `slipway_draw` | dropped | the two on-screen tests are 3.2% (menu-5954) and 4.9% (map-rest-5581) of `slipway_draw`: about 1.7 h per steady frame, under T. N5 makes the same tests cheaper |
| L3 parallax camera computed once | dropped | `camera_apply_method` is 0.8–1.2 h a frame in every run (0.1–0.2% `no-run`), under T |
| G4 upload only changed slots | taken (plan 22 step 3.7), 6th by measured gain | submit is 6.1 h per steady frame, above T; estimated 52% of it, measured 57.6% pooled (ratio 1.11); residual probe 7.4 h per steady frame; plan 21 gross 5.7 / 5.1 |
| B2 off-screen calls rejected at the first test | dropped | plan 21 ceilings 8.4 / 21.8 (pooled 15.1). D11 + D15 delivered 5.2 of their 36.8 pooled ceiling (0.14); the same ratio gives B2 about 2.1 h, under T, on calls that D11 and D15 already batch |
| O1 backdrop as state | dropped | its target, `pad_panel_open`, sits under O2's memo (residual after O2 0.00 / 0.60). `input_step_run` is above T, but in `run_step`'s helpers, not in the backdrop scan |
| L2 overlay pass reuses the sort | dropped | covered by L1: the registry keeps one order per draw message. `draw_layers`' self is 2.8% of its PC time (menu-5954) |
