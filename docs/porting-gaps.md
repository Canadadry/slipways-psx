# Porting gaps

Known divergences between this C port and the original PICO-8 cart, plus
the pieces deliberately left undone. The code points here with a
`/* GAP-n */` comment at the relevant spot, without repeating the
explanation.

Each entry is written with today's C names and paths. The cart names and
PICO-8 API names it cites are gathered on its "Lua:" line, under the
heading.

Every entry was rechecked against the code when the comments were
reworked. A deleted entry is a closed entry: see git.

---

## GAP-17 — The pseudo-random generator is not PICO-8's

Lua: `rnd`.

`p8_fixed_rnd` draws from a home-made generator (`rnd_next_u32`), not
from PICO-8's. No sequence matches.

Direct consequence, and stronger than the `sin`/`cos` rounding outside the 64 probed points: the agreed goal "same seed →
same galaxy" **does not hold** against the real cart, and not only because
floating point drifts. Here the very first value drawn already differs.

The internal replays stay deterministic, since this port is compared with
itself.

File: `src/p8/fixed.c`.

## GAP-19 — The font is rebuilt by hand

PICO-8's glyphs belong to its runtime, not to the cart, so they are in no
exportable asset. This port redraws them (3×5 pixels in a 4-wide cell,
`font_glyph_rows`) and displays a solid block for any character not
covered.

Permanent, accepted visual divergence: the only way to close it would be
to capture the glyphs from PICO-8 itself.

File: `src/p8/gfx.c`.

## GAP-20 — An explicit `nil` as the color of `circ` is not verified on PICO-8

Lua: `ga:kj`, `circ(p.x,p.y,pt.p:qx(),self.k_[flr(pt.l/2)])`, `1 < l < 2`,
`k_`, `k_[0]`, `nil`, `circ`, `color(8) circ(64,64,10,nil)`, `rk`,
`rk_default`.

In `burst_draw`, each particle is drawn by `p8_circ`, in the color of the
trail ramp at index flr(life/2) (lua.lua:1091). Particles whose life is
between 1 and 2 give index 0. The cart's ramp is indexed from 1, so index
0 is nil there and `circ` receives an **explicit nil** fourth argument,
not an omitted one. This port then draws in color 0 (black).

This is probably not a divergence. The manual says nothing about an
explicit nil. Two emulators convert it to 0 and also set the pen to 0:
zepto8 (only lua_isnone marks the argument as absent; nil gives
lua_tonumber(nil) = 0) and fake-08 (lua_tonumber(L,4) as soon as there
are more than 3 arguments). The port therefore draws as they do. It does
not set the pen, which has no visible effect here.

Not verified on the PICO-8 binary. The check: in a cart, draw a circle
with a nil color after setting the pen to 8, and look at the color. If
PICO-8 kept the pen, the port would first need real pen state (see the
note on `pen_color` below); only 22 frames of `geoharvest_burst` would
change.

Related note: in both emulators, **every** colored draw updates the pen,
whereas the port's `pen_color` is written only by `hud_draw_bar`. A
colored draw between a `hud_draw_bar` and a `hud_draw_bar_default` would
make the port diverge. Not investigated.

Premise refuted by diagnosis in the emulators.

File: `src/game/planet_family.c`.

## GAP-22 — `tostr` below 0.0001 is not sourced

Lua: `tostr`.

`p8_tostr_fixed` writes a number with 4 decimals rounded half to even,
then strips trailing zeros and a bare point. This rule reproduces 13 of
the 14 witnesses in block C of `tools/pico8_reference_probe_out.txt`,
including 2/3 → `"0.6667"`.

The 14th is raw −6 (`0xffff.fffa`): PICO-8 displays `"-0"`, this port
`"-0.0001"`. No single rule reconciles this witness with
2/3 → `"0.6667"`, so none is guessed. This port keeps its output: raw −6 →
`"-0.0001"`, raw −1 to −3 → `"-0"`, raw 1 to 3 → `"0"`. For the other
magnitudes below 0.0001, PICO-8's rule is unknown.

Not reached by the e2e: no value the game displays has a magnitude
below 0.0001. It can be settled by one more line in
`tools/pico8_reference_probe.p8`, run on PICO-8.

File: `src/p8/str.c`.

## GAP-24 — `trade_extend` gives the new route a `false` resource when the route has no resource

Lua: `trade:kg`, `qe=false`, `self.qe`, `qe`, `trade`, `jo:cn`, `jo:ib`,
`kg`, `nr:cn`, `dh`, `kw`.

In the cart, extending a route copies its resource as is: nil stays nil,
and no route ever has a `false` resource. This port sets `false` when the
extended route has no resource. In the `trade_route_t` struct, nil and
absent are one and the same: a nil resource, whether passed explicitly or
omitted, gives `false`.

`resource_t` keeps all three states, and `resource_equal` is Lua's `==`.
The divergence shows where a route meets a need with no resource. In
`planet_offer_trade`, a route coming from `planet_rebuild_trades` with no
resource (nil) answers such a need, as in the cart. Once it has gone
through `trade_extend` (`slipgate_offer_trade`), it is `false` and no
longer answers it. In `economy_tiers_compute_tier`, the `has_resource`
guard also leaves out the nil route that the cart would count: a route
with no number never counts there.

Not reached by the e2e: every route they build has a non-nil resource.
`planet_rebuild_trades` does pass a nil resource when the planet's
`production` has no resource. `tests/test_trade.c` pins both readings.

Files: `src/game/trade.c`, `src/game/planet_trade.c`,
`src/game/economy_tiers.c`.

## GAP-25 — The end panel's "score 0" row has a hole, and the port draws both of its elements

Lua: `o("",score=0,ra=v(64,21),c=5)`, `bz`, `pj:ph`, `st[3]`, `st[1]`, `lq`,
`lq:pq`, `nv`, `all(self)`, `#t`, `all`, `fs`, `w`, `a=0.5`, `pi.ld`, `[1]`,
`[3]`, `t[1]`.

The line for the panel's "score 0" row (lua.lua:803) has only one
positional entry, index 1. `score_panel_update` (lua.lua:885-891) writes index 3
into it (the empty string `score_panel_final_score_text` returns for index 0), then rewrites index 1, and passes it to
a layout row. The row therefore carries indices 1 and 3, with no 2.

In the cart, building the row walks it up to its border
(lua.lua:289-290). The port instead takes every index key in ascending
order (the loop in `score_panel_update` over `has_value`) and closes the
hole: the row's cells hold both elements, so its `placed_items` keep
both and `width` counts both. The row's centering (alignment 0.5) depends
on it. Reached by the e2e (`game_end`, 4 constructions).

Verified in the cart source: the row's line, the order of the writes in
`score_panel_update`, and the walk done when building the row. Not
verified on PICO-8: the border of this table. In the Lua 5.2
implementation, a table built with index 1 and then given index 3 has a
border of 1, and the walk would see only index 1. PICO-8 may return 3,
the other valid border (since index 1 is not nil, 0 is not a border).

Pre-existing: the row instance already walked every index key. The
conversion to a struct preserves the divergence.

Files: `src/game/layout.c` (row construction, `layout_row_construct`),
`src/game/score_panel_display.c` (`score_panel_update_add_row`).

## GAP-26 — An off-screen planet's target is cut off when the held object is released

Lua: `fo`, `jo:kj`, `self.fo=nil`, `gd()`, `ll`, `jc`, `dw`, `kj`, `np`.

`planet_draw` (lua.lua) returns before clearing `targeted_by` when the
planet is off screen (`entity_geom_is_on_screen`) or in the "vanishing"
state. Its `targeted_by` then still holds the held object (a structure
picked up by a research icon, a slipway) after that object has left the
game. If the planet comes back on screen, the cart draws one more halo
with that dead object's `attach_distance`.

The port releases the object through the exit queue and recycles its
arena. The planet's `targeted_by` is an entity id: after the flush that
releases the held object, it resolves to NULL, and that halo is not
drawn. In the "vanishing" state there is no difference: from t ≥ 60 on,
`planet_draw` always returns before reading `targeted_by`. No e2e reaches
this case.

Accepted divergence: the port does not draw a halo for an object that has
left the game.

Files: `src/game/planet_display.c` (`planet_draw`), `src/game/probe.c`
(`probe_link_target`).

## GAP-27 — The camera never shows anything outside the background grid

Lua: `qb:ph`, `mid(-64,320,…)`, `qb:md`, `camera`.

The cart clamps the camera's top-left corner to [−64, 320] on each axis,
for a 128-pixel view. The background is drawn at parallax 0.5
(`layer_fades_table[0]`) and its grid runs from 0 to
`BACKGROUND_GRID_EDGE` (262) (`background_draw`). At the clamp limits, the cart's
view shows 32 px of empty space at the top and left, and 26 px at the
right and bottom.

The port clamps the camera so that the `P8_SCREEN_WIDTH` ×
`P8_SCREEN_HEIGHT` view stays inside the grid: from 0 to
(`BACKGROUND_GRID_EDGE` + 1 − size) / 0.5, that is x ∈ [0, 206] and
y ∈ [0, 286] at 160×120. The shaken position (`pos`) is clamped too,
whereas the cart shakes past its clamp limits.

Planets are placed in [16, 368) (`galaxy_populate`). In y, the view covers
[0, 405]: all planets stay well clear of the
top and bottom HUD bars. In x, it covers
[0, 365]: a planet at the very right edge (x = 367.99, rolled in
`pause_restart`) has its center 2 px outside the view at the clamp limit, but 6
columns of its disc are visible and its click box (± 7 px) reaches
column 155.

Files: `src/game/scroll.c` (`camera_bound`), `src/game/background.h`.

## GAP-28 — A background held on the right does not drag the new game's camera on the restart frame

Lua: `ui:bh`, `ui:jz`, `rheld`, `hq`, `mo`, `ui:lj`.

A click on "restart" (or on a difficulty button) runs `reset_world()`
from inside `ui_update`, and the rest of that `ui_update` runs on the old
ui. If the right button is held at that moment, the cart's `ui:bh("r")`
still drags the old ui's `rheld`, the old world's background: its `hq`
sets the velocity of the new game's camera (`mo`) to the old drag delta.

The port's `ui_t.right_held` is an entity id, and every id of the old
world resolves to NULL after `reset_world()`: the old background is not
dragged, and the new camera keeps its archetype's zero velocity.
`reset_world()` also empties the old ui's hover layers and ephemerals
(`ui_forget_world`): a press read on that same frame reaches no entity of
the old world, where the cart would still send it to the old layers
(`ui:jz`). No e2e reaches either case.

Files: `src/game/ui.c` (`ui_handle_button`, `ui_forget_world`),
`src/engine/bootstrap.c` (`reset_world`).
