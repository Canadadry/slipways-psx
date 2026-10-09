# Pad modes and actions

Every pad button press is read in the current mode and maps to one action.
The table of which button triggers which action is the planned binding file
(not yet read by the game).

## Modes

- title
- jump
- aim
- slipway
- menu
- pause
- build_bar
- placement
- panels

## Actions by mode

- title
  - move_up
  - move_down
  - move_left
  - move_right
  - choose_row
  - nothing
- jump
  - jump_up
  - jump_down
  - jump_left
  - jump_right
  - act_on_hovered
  - back
  - enter_aim
  - open_menu
  - open_build_bar
  - open_tech_panel
  - select_tap_or_year
  - pause
  - nothing
- aim
  - pan_up
  - pan_down
  - pan_left
  - pan_right
  - probe_or_fold
  - back
  - leave_aim
  - open_build_bar
  - open_tech_panel
  - select_tap_or_year
  - pause
  - nothing
- slipway
  - jump_up
  - jump_down
  - jump_left
  - jump_right
  - build_on_target
  - cancel_slipway
  - open_build_bar
  - open_tech_panel
  - pause
  - nothing
- menu
  - move_up
  - move_down
  - move_left
  - move_right
  - choose_row
  - close_menu
  - open_build_bar
  - open_tech_panel
  - nothing
- pause
  - move_up
  - move_down
  - move_left
  - move_right
  - choose_row
  - close_pause
  - nothing
- build_bar
  - pick_left
  - pick_right
  - take_structure
  - close_bar
  - open_tech_panel
  - pause
  - nothing
- placement
  - pan_up
  - pan_down
  - pan_left
  - pan_right
  - place_structure
  - cancel_placement
  - open_build_bar
  - open_tech_panel
  - pause
  - nothing
- panels
  - move_up
  - move_down
  - move_left
  - move_right
  - choose_row
  - close_panel
  - next_panel
  - open_build_bar
  - nothing

## Rules across modes

- `menu` is the box of any hovered object, opened by `open_menu`; `close_menu` is both ○ and △.
- In aim, `back` and `leave_aim` have the same effect: back to jump on the object nearest the crosshair.
- From slipway, menu or placement, `open_build_bar` and `open_tech_panel` first cancel what is held or close the box; `pause` from placement keeps the structure and gives it back when pause closes.
- `close_pause` is both Start and ○.
- On the map, a ○ press first closes an open message box, such as the year box, and does nothing else.
