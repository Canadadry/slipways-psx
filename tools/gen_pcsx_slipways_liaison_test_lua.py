import re
import sys

WANTED = [
    "g_cursor_mouse_mode",
    "g_liaison_dragging",
    "g_liaison_from_index",
    "g_liaison_hover_index",
    "g_liaison_live_reason",
    "g_liaison_live_blocking_planet",
    "g_liaison_live_blocking_route",
    "g_liaison_last_reason",
    "g_liaison_last_blocking_planet",
    "g_liaison_last_blocking_route",
    "test_harness_ready",
]

SYM_RE = re.compile(r"^(\S+)\s+\S+\s+([0-9a-fA-F]+)\b")


def base_name(sym):
    return re.sub(r"\.\d+$", "", sym)


def parse_map(map_path):
    addrs = {}
    with open(map_path, "r") as f:
        for line in f:
            m = SYM_RE.match(line)
            if not m:
                continue
            name = base_name(m.group(1))
            if name in WANTED and name not in addrs:
                addrs[name] = int(m.group(2), 16) & 0xFFFFFFFF
    return addrs


TEMPLATE = """
local ffi = ffi

local G_CURSOR_MOUSE_MODE_ADDR = __G_CURSOR_MOUSE_MODE_ADDR__
local G_LIAISON_DRAGGING_ADDR = __G_LIAISON_DRAGGING_ADDR__
local G_LIAISON_FROM_INDEX_ADDR = __G_LIAISON_FROM_INDEX_ADDR__
local G_LIAISON_HOVER_INDEX_ADDR = __G_LIAISON_HOVER_INDEX_ADDR__
local G_LIAISON_LIVE_REASON_ADDR = __G_LIAISON_LIVE_REASON_ADDR__
local G_LIAISON_LIVE_BLOCKING_PLANET_ADDR = __G_LIAISON_LIVE_BLOCKING_PLANET_ADDR__
local G_LIAISON_LIVE_BLOCKING_ROUTE_ADDR = __G_LIAISON_LIVE_BLOCKING_ROUTE_ADDR__
local G_LIAISON_LAST_REASON_ADDR = __G_LIAISON_LAST_REASON_ADDR__
local G_LIAISON_LAST_BLOCKING_PLANET_ADDR = __G_LIAISON_LAST_BLOCKING_PLANET_ADDR__
local G_LIAISON_LAST_BLOCKING_ROUTE_ADDR = __G_LIAISON_LAST_BLOCKING_ROUTE_ADDR__
local READY_ADDR = __TEST_HARNESS_READY_ADDR__
local READY_MAGIC = 0x1eadbeef

-- Mirrors psx-slipways/screen.h and psx-slipways/cursor.h's documented
-- values (cross-checked at generation time, not re-derived): the cursor
-- starts centered and moves CURSOR_STEP px/frame per held D-pad direction.
local SCREEN_XRES = 320
local SCREEN_YRES = 240
local CURSOR_STEP = 2
local INITIAL_X = SCREEN_XRES / 2
local INITIAL_Y = SCREEN_YRES / 2

-- liaison.h's liaison_reason_t enum values (cross-checked at generation
-- time, not re-derived): LIAISON_VALID=0, LIAISON_ALREADY_CONNECTED=1,
-- LIAISON_TOO_FAR=2, LIAISON_BLOCKED_BY_PLANET=3, LIAISON_CROSSES_ROUTE=4.
local LIAISON_VALID = 0
local LIAISON_TOO_FAR = 2

-- world.c seeds its galaxy with a literal fixed RNG seed
-- (WORLD_RNG_SEED=12345u), so world_init()'s 90-planet layout is fully
-- deterministic. Planet index 0 is already a real, non-arbitrary reference
-- point in this codebase -- world.h's WORLD_WORMHOLE_INDEX names it as "a
-- real generated planet position that lands inside the galaxy's visual
-- footprint" -- so this test reuses that same planet rather than inventing
-- a second special index. A throwaway host-side driver (world_init() +
-- screen_map_planet(), the same technique this PRD's chunk 5 screenshot
-- verification used) resolved its screen position at generation time:
-- planet[0] lands at screen (141, 112) for this seed/build.
local TARGET_PLANET_INDEX = 0
local TARGET_PLANET_SCREEN_X = 141
local TARGET_PLANET_SCREEN_Y = 112

-- liaison.h's LIAISON_PICK_RADIUS_PX (cross-checked at generation time, not
-- re-derived): liaison_pick_planet's hit-test radius.
local LIAISON_PICK_RADIUS_PX = 8

local BOOT_TIMEOUT_FRAMES = 3000
local SETTLE_FRAMES = 8

-- Cursor starts at (INITIAL_X, INITIAL_Y) = (160, 120); the target planet
-- sits at (141, 112), an offset of (-19, -8) px. Holding UP for 4 frames
-- moves it by exactly -8px (dy done); holding LEFT for 10 frames then moves
-- it by -20px (dx overshoots the ideal -19 by 1px). The resulting rest
-- position, (140, 112), is 1px from the planet's screen position -- well
-- inside LIAISON_PICK_RADIUS_PX (8), so liaison_pick_planet resolves it
-- reliably regardless of the 1px overshoot.
local UP_HOLD_FRAMES = 4
local LEFT_HOLD_FRAMES = 10

-- Holding DOWN for DOWN_HOLD_FRAMES frames at CURSOR_STEP px/frame moves the
-- cursor's *screen* y by 2*DOWN_HOLD_FRAMES px from (140, 112) down to
-- (140, 212). screen_map's world<->screen mapping is not uniform per axis
-- (SCREEN_YRES's usable range is smaller than SCREEN_XRES's for the same
-- world-unit span, so a screen-y pixel covers more world-distance than a
-- screen-x pixel does), which is why this test crosses the "too far"
-- threshold by moving the D-pad *down* rather than sideways -- fewer frames
-- needed for the same world-space displacement. The same throwaway
-- host-side driver that resolved the planet's screen position above also
-- fed candidate frame counts through screen_map_unproject() and
-- liaison.h's LIAISON_RANGE_SQ directly (not re-derived here, just
-- cross-checked): 50 frames lands the cursor's unprojected world position
-- at a squared distance from planet 0 of roughly 6.36e9 (fixed-point,
-- Q16.16-squared units) against a LIAISON_RANGE_SQ threshold of
-- ~2.147e9 -- about 3x over the line, comfortable margin for any
-- override/poll-alignment jitter. The same driver confirmed no other
-- planet's screen position lands within 12px of the (140, 212) rest point,
-- so liaison_pick_planet reliably resolves hover_index back to -1 there
-- (the raw-cursor-position fallback this test needs to observe).
local DOWN_HOLD_FRAMES = 50

local MEM = PCSX.getMemPtr()

local function readS32(addr)
    return ffi.cast("int32_t*", MEM + bit.band(addr, 0x1fffff))[0]
end

local function waitFrames(n)
    for i = 1, n do
        coroutine.yield()
    end
end

local function assertEq(actual, expected, msg)
    if actual ~= expected then
        error(string.format("%s (expected %s, got %s)", msg, tostring(expected), tostring(actual)))
    end
end

local PAD = PCSX.SIO0.slots[1].pads[1]
local BTN = PCSX.CONSTS.PAD.BUTTON

local function press(button, holdFrames)
    PAD.setOverride(button)
    waitFrames(holdFrames)
    PAD.clearOverride(button)
end

local function waitForBoot()
    for i = 1, BOOT_TIMEOUT_FRAMES do
        if readS32(READY_ADDR) == READY_MAGIC then
            return
        end
        coroutine.yield()
    end
    error(string.format("psx-slipways never reached its main loop within %d frames", BOOT_TIMEOUT_FRAMES))
end

local function testBody()
    waitForBoot()
    waitFrames(SETTLE_FRAMES)

    -- (1) documented initial values after boot, mirroring
    -- tools/gen_pcsx_slipways_cursor_test_lua.py's own assertions: cursor
    -- centered, mouse mode off, no drag in progress.
    assertEq(readS32(G_CURSOR_MOUSE_MODE_ADDR), 0, "g_cursor_mouse_mode should start off")
    assertEq(readS32(G_LIAISON_DRAGGING_ADDR), 0, "g_liaison_dragging should start at 0")
    assertEq(readS32(G_LIAISON_FROM_INDEX_ADDR), -1, "g_liaison_from_index should start at -1")

    -- (2) enable mouse mode (Cross press), same technique as the cursor
    -- test.
    press(BTN.CROSS, SETTLE_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readS32(G_CURSOR_MOUSE_MODE_ADDR), 1, "Cross press should toggle g_cursor_mouse_mode on")

    -- (3) move the cursor onto the target planet's screen position (see the
    -- UP_HOLD_FRAMES/LEFT_HOLD_FRAMES comment above for the exact pixel
    -- math).
    press(BTN.UP, UP_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)
    press(BTN.LEFT, LEFT_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)

    -- (4) Circle press+hold starts a drag from the hovered planet. Held,
    -- not just a single-frame press, because liaison_drag_step's press
    -- branch and its held branch must both see click_pressed/click_held
    -- true across this window for the drag to both start and stay open
    -- into step (5)/(6) below.
    PAD.setOverride(BTN.CIRCLE)
    waitFrames(SETTLE_FRAMES)

    assertEq(readS32(G_LIAISON_DRAGGING_ADDR), 1,
        "g_liaison_dragging should be 1 after pressing Circle while hovering the target planet")
    assertEq(readS32(G_LIAISON_FROM_INDEX_ADDR), TARGET_PLANET_INDEX,
        "g_liaison_from_index should be the target planet's galaxy index")
    assertEq(readS32(G_LIAISON_HOVER_INDEX_ADDR), TARGET_PLANET_INDEX,
        "g_liaison_hover_index should still be the target planet right after starting the drag on it")

    -- (5) live reason at the drag's starting position: the held-frame
    -- target still resolves (via liaison_pick_planet) to the very planet
    -- the drag started from, so the candidate segment has zero length --
    -- well inside LIAISON_RANGE_SQ, unblocked, uncrossed -- LIAISON_VALID.
    assertEq(readS32(G_LIAISON_LIVE_REASON_ADDR), LIAISON_VALID,
        "g_liaison_live_reason should be LIAISON_VALID while the drag target is still the source planet itself")

    -- (6) still holding Circle, hold D-pad Down long enough to carry the
    -- live drag target across the LIAISON_TOO_FAR threshold (see the
    -- DOWN_HOLD_FRAMES comment above for the derivation and safety
    -- margin). This is the core proof this PRD chunk asks for: the live
    -- reason field actually changes, live, as the drag target moves.
    press(BTN.DOWN, DOWN_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)

    assertEq(readS32(G_LIAISON_DRAGGING_ADDR), 1,
        "g_liaison_dragging should still be 1 -- Circle is still held")
    assertEq(readS32(G_LIAISON_HOVER_INDEX_ADDR), -1,
        "g_liaison_hover_index should be -1 once the cursor has moved off every planet's pick radius")
    assertEq(readS32(G_LIAISON_LIVE_REASON_ADDR), LIAISON_TOO_FAR,
        "g_liaison_live_reason should have flipped to LIAISON_TOO_FAR after moving the drag target far enough away")
    assertEq(readS32(G_LIAISON_LIVE_BLOCKING_PLANET_ADDR), -1,
        "g_liaison_live_blocking_planet should be -1 for a too-far result (the range check short-circuits)")
    assertEq(readS32(G_LIAISON_LIVE_BLOCKING_ROUTE_ADDR), -1,
        "g_liaison_live_blocking_route should be -1 for a too-far result (the range check short-circuits)")

    -- (7) releasing Circle ends the drag. The release-time target is still
    -- the same too-far cursor position (nothing moved since step 6), so
    -- the final liaison_check liaison_drag_step performs at release should
    -- reproduce the same LIAISON_TOO_FAR outcome into last/last_reason.
    PAD.clearOverride(BTN.CIRCLE)
    waitFrames(SETTLE_FRAMES)

    assertEq(readS32(G_LIAISON_DRAGGING_ADDR), 0,
        "g_liaison_dragging should be 0 after releasing Circle")
    assertEq(readS32(G_LIAISON_FROM_INDEX_ADDR), -1,
        "g_liaison_from_index should reset to -1 (idle) after releasing Circle")
    assertEq(readS32(G_LIAISON_LAST_REASON_ADDR), LIAISON_TOO_FAR,
        "g_liaison_last_reason should be LIAISON_TOO_FAR, matching the live state right before release")
    assertEq(readS32(G_LIAISON_LAST_BLOCKING_PLANET_ADDR), -1,
        "g_liaison_last_blocking_planet should be -1 for a too-far result (the range check short-circuits)")
    assertEq(readS32(G_LIAISON_LAST_BLOCKING_ROUTE_ADDR), -1,
        "g_liaison_last_blocking_route should be -1 for a too-far result (the range check short-circuits)")

    print("OK: all psx-slipways liaison drag-injection assertions passed")
    PCSX.quit(0)
end

local co = coroutine.create(testBody)

PCSX.Events.createEventListener("GPU::Vsync", function()
    if coroutine.status(co) == "dead" then return end
    local ok, err = coroutine.resume(co)
    if not ok then
        printError("FAIL: " .. tostring(err))
        PCSX.quit(1)
    end
end)
"""


def main():
    if len(sys.argv) != 3:
        print("usage: gen_pcsx_slipways_liaison_test_lua.py <slipways.map> <out.lua>", file=sys.stderr)
        return 1
    map_path, out_path = sys.argv[1], sys.argv[2]
    addrs = parse_map(map_path)
    missing = [name for name in WANTED if name not in addrs]
    if missing:
        print("missing symbols in %s: %s" % (map_path, ", ".join(missing)), file=sys.stderr)
        print("(liaison.c's g_liaison_* globals, cursor.c's g_cursor_mouse_mode, and main.c's test_harness_ready must be 'static' or otherwise map-visible)", file=sys.stderr)
        return 1

    out = TEMPLATE
    for name in WANTED:
        token = "__%s_ADDR__" % name.upper()
        out = out.replace(token, "0x%08x" % addrs[name])

    with open(out_path, "w") as f:
        f.write(out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
