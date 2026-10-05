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
