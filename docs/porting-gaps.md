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
