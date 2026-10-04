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

## GAP-1 — `sin`/`cos` are not bit-exact outside the 64 measured points

Lua: `sin`, `cos`.

`p8_fixed_cos`/`p8_fixed_sin` read a 16.16 quarter-wave table
(`p8_quarter_wave` in `src/p8/quarter_wave.h`, 16,385 entries, one per
turn fraction r/65536 of the first quarter, no interpolation), generated
on the host by `tools/gen_quarter_wave.c` (`make sin-table`). They are
bit-exact on the 64 points i/64 of the probe
(`tools/pico8_reference_probe_out.txt`, block E, asserted by `make test`).
Everywhere else they return the true value rounded to nearest, which is
what the old libc `double` computation returned. This has **no PICO-8
source**: PICO-8's own approximation is not published.

The pseudo-random generator falls under GAP-17.

Consequence: a game replayed here can diverge from the same game replayed
in real PICO-8. The internal replays stay deterministic, since they compare
this port with itself.

Files: `src/p8/fixed.c`, `src/p8/quarter_wave.h`.

## GAP-3 — Strings are not interned

Lua: `hy.invented`, `oj`, `jo:gg`, `ho()`, `r`, `hp.o`, `hp.dm`, `kw.qe`,
`{qe=r[c]}`.

Lua interns literals, so two identical strings are the same object. This
port copies or allocates its strings (the generated data has a separate
array per token). Two equal strings are therefore not the same pointer.

Worked around case by case: `collect_contains_string` compares by content
(the strings in the economy's `invented` list, for
`tech_gate_is_unlocked`). Any new string comparison must choose
explicitly.

**Not part of GAP-3: resources.** What the cart's data-table loader routes
through the resource table (a project's `output` and `byproduct`, hence
the `resource` of a `production_t`) and resource needs are numbers (the
index of a resource), not strings. They compare by value (`==` on
`p8_fixed_t`, `resource_equal`).

Files: `src/engine/collect.c`.

## GAP-5 — `update_entities` skips an entity when an entity removes itself

Lua: `dg()`, `hv`.

`update_entities` walks the list of live entities (`live_entities`) by
index. When an entity removes itself during its own visit
(`entity_unregister`), the following ones shift down by one, and the one
that takes its place is not visited this frame. Lua behaves differently.

Accepted divergence: a skipped entity is simply processed on the next
frame.

File: `src/engine/bootstrap.c`.

## GAP-8 — The `targeting` tag has no reader

Lua: `g_`, `jo`, `prober`, `cg("prober")`.

The probe (`probe`), the help screen and the whole planet family register
under the `TAG_TARGETING` tag, but nothing ever calls
`entities_by_tag(TAG_TARGETING)`. Kept for fidelity, not for usefulness.

## GAP-9 — No deletion of a named field

Lua: `t[nom]=nil`.

The port's dynamic-key table could remove an element by index, but not a
named field: no call site needed it. Clearing a named field therefore has
no exact equivalent. Writing a false value is not the same thing as an
absent field.

One site: the button of an opened box (`message_box_open_box`) takes the
frame's background color only if the frame has one, instead of writing an
absent color (in Lua, writing nil would clear the field). Pinned by
`test_open_box_layer_only_from_back_color` in `tests/test_button.c`.

File: `src/game/message_box.c`.

## GAP-12 — The wormhole's first ring is black instead of keeping the pen color

Lua: `mz:kj`, `self.kk[flr(sr/self.oi*8)]`, `kk`, `kk[0]`, `self.t`, `dt`.

`wormhole_draw` colors each ring from the archetype's `color_ramp`, at
index flr(sr/oi*8). The index drops to 0 as soon as sr < oi/8, that is,
for the youngest ring, on every tick where the state time has just passed
a `dt`.

The cart's ramp is a Lua table indexed from 1: index 0 is nil there, and
PICO-8 then keeps the current pen color. This port falls back to 0
(`int_seq_at` leaves the default color) and draws in black. It is not the
same image.

Found while reworking the comments: the comment at that spot claimed the
opposite, "REAL, FAITHFULLY-REPRODUCED EDGE CASE". The gap is real; the
claim was false.

Not turned into an `assert`: the condition is hit all the time, and the
current behavior is what the replays record. Fixing it will change the
existing recordings, which makes it a gameplay decision, not a comment
rework.


**Premise probably false** (established for GAP-20, same situation): index
0 of the ramp is an **explicit** nil, which zepto8 and fake-08 convert to
color 0. The port's black would then be faithful. Not verified on the
PICO-8 binary; same check as GAP-20.

File: `src/game/wormhole_display.c`.

## GAP-13 — `tostr` of an absent field returns the empty string

Lua: `tostr`, `s[st.score]`, `score == 0`, `ky()`.

The panel's "score 0" row reads entry 0 of the final score, that is, an
index that `economy_score_final` never sets. Lua would see nil there.

This port has no nil to turn into a string: `score_panel_final_score_text`
returns `""` for any index outside 1..6. PICO-8, on the other hand, has a
dedicated representation for nil in `tostr`, so the line does not display
the same way.

Pinned by `test_scores_box_follows_is_shown` in `tests/test_message_box.c`
(`SCORES_SEQUENCE_HASH`).

File: `src/game/score_panel_display.c`.

## GAP-14 — A loop that deletes its own element skips one

Lua: `ei()`, `qy`.

Same family as GAP-5, in other places and on other lists: `emitter_update`
walks its emitter's particles (`particles`) and deletes some in the middle
of its own loop (`pointer_list_del`) without adjusting the index. The
particle that takes the place of the one just removed is not visited this
frame. The same pattern exists in the slipway display.

No visible consequence: a particle lives one frame longer than intended,
and it will be visited on the next frame. Recorded because this is the
third site of the same pattern, and a future reader who fixes only one of
them will make the three inconsistent with each other.

Files: `src/game/particle.c`, `src/game/slipway_display.c`,
and `src/engine/bootstrap.c` for the GAP-5 variant.

## GAP-15 — Where Lua would crash, this port ignores it

Lua: `bd()`, `hg`, `lt`, `he`, `bm`, `dc`, `fa`, `ck()`.

This is a general convention of the port, not a local oversight. When the
cart indexes a missing table, Lua raises an error. This port reads the
field as absent and carries on. The known sites:

- `economy_tiers_rebuild_incoming_routes` writes into the `served_planets`
  set of a slipway that has none: the write is ignored.
- `entity_register`/`entity_unregister` test the **presence** of the
  class's message member, not its Lua truthiness: a member set to false
  still registers the entity.
- `entity_unregister` on a tag that was never registered does nothing.
- `draw_layers` leaves out of every layer an entity with no `layer`, or
  whose `layer` falls outside 1..15.
- `economy_crowding_penalty` skips a planet whose needs list is absent or
  empty.

None of these cases is reached in normal play. They describe what would
happen on malformed data. Recorded because a reader who "fixes" only one
of them by making it fatal would make the port less tolerant than the cart
in one place and not in the others.

Files: `src/game/economy_tiers.c`, `src/game/economy_formulas.c`,
`src/engine/entity.c`.

## GAP-16 — The weighted pick reads out of bounds if its pool runs empty

Lua: `gn()`, `gn_next()`, `oc.items[0]`, `df`, `bw(90)`, `jo:hh`,
`oc[flr(r()*0+1)]`, `until not gu(nm,e)`.

`weighted_picker_new` builds a **consumable** pool of tickets: each
`weighted_picker_next` removes the ticket it drew, and nothing refills it.
Once the pool is empty, `idx0` is 0 and the code reads ticket 0 of an
empty list. `tickets` can be `NULL`.

The numbers, computed on the repository's planet-type literal with `og`
at 2: **66 tickets**. `galaxy_populate` places 90 planets, and
`planet_roll_type` draws a ticket for each one that does not take the
fixed branch (about 55% of cases). The expected value stays under 66, but
the tail of the distribution goes past it.

Found while reworking the comments: an agent wanted to add an `assert`
that `tickets_count` is positive, and computed the margin before writing
it. The assert would have fired. Neither the cart nor this port guards
against this case. In Lua, drawing from an empty list yields nil and the
draw loop spins forever, which is no better.

File: `src/game/gen.c`.

## GAP-17 — The pseudo-random generator is not PICO-8's

Lua: `rnd`.

`p8_fixed_rnd` draws from a home-made generator (`rnd_next_u32`), not
from PICO-8's. No sequence matches.

Direct consequence, and stronger than GAP-1: the agreed goal "same seed →
same galaxy" **does not hold** against the real cart, and not only because
floating point drifts. Here the very first value drawn already differs.

The internal replays stay deterministic, since this port is compared with
itself.

File: `src/p8/fixed.c`.

## GAP-18 — The color remap of `pal()` is ported for sprites only

Lua: `pal()`, `dv()`.

`p8_spr`, and `p8_map` which goes through it, draw each opaque pixel in the
color the draw palette maps it to (the low nibble of the byte). The other
primitives — `pset`, `line`, `rect`, `circ`, the fills and `print` — write
their color **as is**.

What would show: text or a shape drawn under a palette block other than 0
would keep its original colors.

File: `src/p8/gfx.c`.

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

## GAP-21 — The palette of a burst born from a protostar is the class's

Lua: `ga:kj`, `self.k_`, `k_`, `pm:pq()`, `ga`,
`k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1)`, `k_=o(1,2,4,9,9,10,10,7,7,7,7,7)`.

`burst_draw` reads the instance's trail ramp when it has one
(lua.lua:1091). `protostar_construct` gives one to the burst it creates
(1,2,8,14,14,14,14,8,8,2,2,1,1; lua.lua:1030, 1033), which shadows the
class's (1,2,4,9,9,10,10,7,7,7,7,7; lua.lua:1077). This port reads the
class's ramp (`burst_trail_colors_seq`), never the instance's, so a
protostar's burst gets the palette of an ordinary burst.

Probable, not verified against PICO-8. `protostar_construct` runs in the
e2e `tests/e2e/tech_starbirth_protostar.json`: its hash pins the port's
rendering, divergence included.

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

## GAP-23 — A button line that carries its side tooltip or its tooltip as nil still gets its badge or its tooltip

Lua: `h_`, `jb`, `js:pq`, `self.h_=self.h_ and lq(self.h_)`,
`self.jb=self.jb and jb(self.jb)`, `lq`, `jo:fe`, `b:jb()`, `hp:jb`, `ii`,
`jo:gh`, `kr`, `js`, `has_h_`, `has_jb`, `h_row`, `h_is_false`, `jb_row`.

In the cart, building a button builds nothing for a nil key. This port
builds an empty row (`layout_row_new`), or an empty tooltip, as soon as
the line carries the key, even as nil (GAP-9: a named key written as nil
stays present). Only `false` builds none.

Reached by the e2e: the `planet_projects_panel` line of a building
whose `research_project_tooltip` returns `NULL` (when the project has its
`icon_sprite`) gets an empty tooltip, drawn on hover. The
`planet_inspection_panel` line without `net_income` would get an empty
badge; no replay reaches it. The `button_t` struct, where nil means
absent, keeps this behavior: the presence of a nil `side_tooltip` or
`tooltip` key is carried by `has_side_tooltip`/`has_tooltip` in
`button_keys_t`, with no `side_tooltip_row` or `side_tooltip_is_false`
(no `tooltip_row`). `button_construct` then builds an empty row or
tooltip. Pinned by `test_nil_keys` in `tests/test_button.c`.

File: `src/game/ui_button.c`.

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
into it (the string from GAP-13), then rewrites index 1, and passes it to
a layout row. The row therefore carries indices 1 and 3, with no 2.

In the cart, building the row walks it up to its border
(lua.lua:289-290). The port instead takes every index key in ascending
order (the loop in `score_panel_update` over `has_value`) and closes the
hole: the row's `items` member keeps both elements, `placed_items` has
two, and `width` counts both. The row's centering (alignment 0.5) depends
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
`src/game/score_panel_display.c`.

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
