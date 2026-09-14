import re
import sys

WANTED = [
    "g_cursor_mouse_mode",
    "g_cursor_x",
    "g_cursor_y",
    "g_cursor_click_pressed",
    "g_cursor_click_held",
    "g_cursor_click_released",
    "g_cursor_click_flash_frames",
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
local G_CURSOR_X_ADDR = __G_CURSOR_X_ADDR__
local G_CURSOR_Y_ADDR = __G_CURSOR_Y_ADDR__
local G_CURSOR_CLICK_PRESSED_ADDR = __G_CURSOR_CLICK_PRESSED_ADDR__
local G_CURSOR_CLICK_HELD_ADDR = __G_CURSOR_CLICK_HELD_ADDR__
local G_CURSOR_CLICK_RELEASED_ADDR = __G_CURSOR_CLICK_RELEASED_ADDR__
local G_CURSOR_CLICK_FLASH_FRAMES_ADDR = __G_CURSOR_CLICK_FLASH_FRAMES_ADDR__
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

local BOOT_TIMEOUT_FRAMES = 3000
local SETTLE_FRAMES = 8
local DPAD_HOLD_FRAMES = 12
-- Long enough to walk from screen center to the right edge at CURSOR_STEP
-- px/frame with a comfortable margin, regardless of exactly where the
-- cursor already is when this hold starts.
local EDGE_HOLD_FRAMES = 100

-- Mirrors psx-slipways/cursor.h's CLICK_FLASH_FRAMES (cross-checked at
-- generation time, not re-derived).
local CLICK_FLASH_FRAMES = 15
-- Short window bracketing a Circle press/release edge for observePulse:
-- long enough to reliably contain the one-frame pulse despite
-- override/poll-alignment jitter, short enough that it can't also
-- contain a second, unrelated edge.
local CLICK_PULSE_WINDOW_FRAMES = 4
local CLICK_HOLD_SETTLE_FRAMES = 8

local MEM = PCSX.getMemPtr()

local function readS32(addr)
    return ffi.cast("int32_t*", MEM + bit.band(addr, 0x1fffff))[0]
end

local function waitFrames(n)
    for i = 1, n do
        coroutine.yield()
    end
end

-- Samples the given address on *every* frame across a windowFrames-frame
-- window (once per Vsync, via coroutine.yield(), same as waitFrames) and
-- returns how many of those frames saw it nonzero. This is the technique
-- this PRD's chunk 4 adds specifically to catch a single-frame edge pulse
-- (click_pressed/click_released) that waitFrames' "wait N frames, then
-- check once" style would miss entirely if the pulse comes and goes
-- inside the gap.
local function observePulse(addr, windowFrames)
    local seen = 0
    for i = 1, windowFrames do
        if readS32(addr) ~= 0 then
            seen = seen + 1
        end
        coroutine.yield()
    end
    return seen
end

local function assertEq(actual, expected, msg)
    if actual ~= expected then
        error(string.format("%s (expected %s, got %s)", msg, tostring(expected), tostring(actual)))
    end
end

-- Movement-delta assertions tolerate up to one held frame's worth of
-- jitter around the expected value -- the same settle-frame imprecision
-- tools/gen_pcsx_hello_input_test_lua.py's own cursor-movement assertion
-- already tolerates (override-vs-frame-poll alignment isn't exact).
local function assertRange(actual, lo, hi, msg)
    if actual < lo or actual > hi then
        error(string.format("%s (expected between %s and %s, got %s)", msg, tostring(lo), tostring(hi), tostring(actual)))
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

local function readMouseMode()
    return readS32(G_CURSOR_MOUSE_MODE_ADDR)
end

local function testBody()
    waitForBoot()
    waitFrames(SETTLE_FRAMES)

    -- (1) documented initial values after boot.
    assertEq(readMouseMode(), 0, "g_cursor_mouse_mode should start off")
    assertEq(readS32(G_CURSOR_X_ADDR), INITIAL_X, "g_cursor_x should start centered")
    assertEq(readS32(G_CURSOR_Y_ADDR), INITIAL_Y, "g_cursor_y should start centered")

    -- (2) holding Cross for a settle window flips mouse_mode on; releasing
    -- it does not flip it back.
    PAD.setOverride(BTN.CROSS)
    waitFrames(SETTLE_FRAMES)
    assertEq(readMouseMode(), 1, "holding Cross should toggle g_cursor_mouse_mode on")

    PAD.clearOverride(BTN.CROSS)
    waitFrames(SETTLE_FRAMES)
    assertEq(readMouseMode(), 1, "releasing Cross should not toggle g_cursor_mouse_mode back off")

    -- (3) D-pad Right moves g_cursor_x by CURSOR_STEP*frames while mode is
    -- on, and does not touch g_cursor_y.
    local x0 = readS32(G_CURSOR_X_ADDR)
    local y0 = readS32(G_CURSOR_Y_ADDR)

    press(BTN.RIGHT, DPAD_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)

    local x1 = readS32(G_CURSOR_X_ADDR)
    local y1 = readS32(G_CURSOR_Y_ADDR)

    local expectedDx = DPAD_HOLD_FRAMES * CURSOR_STEP
    assertRange(x1 - x0, expectedDx - CURSOR_STEP, expectedDx + CURSOR_STEP,
        "g_cursor_x did not move by the expected amount while holding D-pad Right")
    assertEq(y1, y0, "g_cursor_y should not move while holding D-pad Right")

    -- (3, continued) D-pad Down moves g_cursor_y by CURSOR_STEP*frames
    -- while mode is on, and does not touch g_cursor_x.
    press(BTN.DOWN, DPAD_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)

    local x2 = readS32(G_CURSOR_X_ADDR)
    local y2 = readS32(G_CURSOR_Y_ADDR)

    assertRange(y2 - y1, expectedDx - CURSOR_STEP, expectedDx + CURSOR_STEP,
        "g_cursor_y did not move by the expected amount while holding D-pad Down")
    assertEq(x2, x1, "g_cursor_x should not move while holding D-pad Down")

    -- (4) holding a direction long enough to reach the screen edge clamps
    -- there instead of wrapping or overshooting.
    press(BTN.RIGHT, EDGE_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)

    local xEdge = readS32(G_CURSOR_X_ADDR)
    local yEdge = readS32(G_CURSOR_Y_ADDR)

    assertEq(xEdge, SCREEN_XRES - 1, "g_cursor_x should clamp at the right edge, not wrap or overshoot")
    assertEq(yEdge, y2, "g_cursor_y should be untouched by holding D-pad Right")

    -- (5) toggling Cross off again, then continuing to hold a D-pad
    -- direction, leaves the cursor position unchanged -- proves the
    -- mouse_mode gate is real, not just true-by-coincidence above.
    press(BTN.CROSS, SETTLE_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readMouseMode(), 0, "second Cross press should toggle g_cursor_mouse_mode back off")

    local xBeforeGatedHold = readS32(G_CURSOR_X_ADDR)
    local yBeforeGatedHold = readS32(G_CURSOR_Y_ADDR)

    press(BTN.DOWN, DPAD_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)

    assertEq(readS32(G_CURSOR_X_ADDR), xBeforeGatedHold, "g_cursor_x should not move while g_cursor_mouse_mode is off")
    assertEq(readS32(G_CURSOR_Y_ADDR), yBeforeGatedHold, "g_cursor_y should not move while g_cursor_mouse_mode is off")

    -- (6) click events (psx-simulated-click PRD, chunk 4): real emulated
    -- SIO0 Circle injection, proving the press/held/release edges and the
    -- flash countdown are genuinely wired end to end, not just correct in
    -- the host-tested pure core. Mode is currently off (step 5 above left
    -- it there) -- turn it back on first.
    press(BTN.CROSS, SETTLE_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readMouseMode(), 1, "third Cross press should toggle g_cursor_mouse_mode back on for click assertions")

    -- (7) pressing Circle: click_pressed is a single-frame edge pulse, so
    -- it's observed with observePulse (sampling every frame across a
    -- short window right after the press) rather than checked once after
    -- a settle gap, which would already read back 0 by the time it's
    -- checked.
    PAD.setOverride(BTN.CIRCLE)
    local pressedSeen = observePulse(G_CURSOR_CLICK_PRESSED_ADDR, CLICK_PULSE_WINDOW_FRAMES)
    assertRange(pressedSeen, 1, 2,
        "g_cursor_click_pressed should have pulsed nonzero at least once (tolerating up to one frame of override/poll jitter) right after pressing Circle")

    -- (8) the flash countdown should still be actively running shortly
    -- after the press (not yet exhausted).
    assertRange(readS32(G_CURSOR_CLICK_FLASH_FRAMES_ADDR), 1, CLICK_FLASH_FRAMES,
        "g_cursor_click_flash_frames should still be actively counting down right after the press")

    -- (9) click_held is a level, not an edge: it should read true for as
    -- long as Circle stays physically held, well past the single press
    -- frame -- checked the same settle-and-check way the existing
    -- Cross-toggle assertions above already use.
    waitFrames(CLICK_HOLD_SETTLE_FRAMES)
    assertEq(readS32(G_CURSOR_CLICK_HELD_ADDR), 1,
        "g_cursor_click_held should be true while Circle remains held past the initial press frame")

    -- (10) releasing Circle: click_released is likewise a single-frame
    -- edge pulse, observed the same way as the press edge above.
    PAD.clearOverride(BTN.CIRCLE)
    local releasedSeen = observePulse(G_CURSOR_CLICK_RELEASED_ADDR, CLICK_PULSE_WINDOW_FRAMES)
    assertRange(releasedSeen, 1, 2,
        "g_cursor_click_released should have pulsed nonzero at least once (tolerating up to one frame of override/poll jitter) right after releasing Circle")

    waitFrames(SETTLE_FRAMES)
    assertEq(readS32(G_CURSOR_CLICK_HELD_ADDR), 0, "g_cursor_click_held should be false after releasing Circle")

    -- (11) waiting comfortably longer than CLICK_FLASH_FRAMES frames past
    -- the press, with no further press in between, the countdown must
    -- have reached and stayed at exactly 0.
    waitFrames(CLICK_FLASH_FRAMES + SETTLE_FRAMES)
    assertEq(readS32(G_CURSOR_CLICK_FLASH_FRAMES_ADDR), 0,
        "g_cursor_click_flash_frames should read back exactly 0 well after the flash window has elapsed")

    -- (12) toggling mouse mode back off, then holding Circle across the
    -- same pulse-observation window used above, proves the mode gate is
    -- real for click state too (mirrors step 5's D-pad gate check): none
    -- of the four click fields should ever go nonzero while mode is off.
    press(BTN.CROSS, SETTLE_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readMouseMode(), 0, "fourth Cross press should toggle g_cursor_mouse_mode back off")

    PAD.setOverride(BTN.CIRCLE)
    local pressedSeenGated = observePulse(G_CURSOR_CLICK_PRESSED_ADDR, CLICK_PULSE_WINDOW_FRAMES + CLICK_HOLD_SETTLE_FRAMES)
    assertEq(pressedSeenGated, 0, "g_cursor_click_pressed should never pulse while g_cursor_mouse_mode is off")
    assertEq(readS32(G_CURSOR_CLICK_HELD_ADDR), 0, "g_cursor_click_held should stay 0 while g_cursor_mouse_mode is off, even with Circle held")
    assertEq(readS32(G_CURSOR_CLICK_FLASH_FRAMES_ADDR), 0, "g_cursor_click_flash_frames should stay 0 while g_cursor_mouse_mode is off")

    PAD.clearOverride(BTN.CIRCLE)
    local releasedSeenGated = observePulse(G_CURSOR_CLICK_RELEASED_ADDR, CLICK_PULSE_WINDOW_FRAMES)
    assertEq(releasedSeenGated, 0, "g_cursor_click_released should never pulse while g_cursor_mouse_mode is off")

    print("OK: all psx-slipways cursor assertions passed")
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
        print("usage: gen_pcsx_slipways_cursor_test_lua.py <slipways.map> <out.lua>", file=sys.stderr)
        return 1
    map_path, out_path = sys.argv[1], sys.argv[2]
    addrs = parse_map(map_path)
    missing = [name for name in WANTED if name not in addrs]
    if missing:
        print("missing symbols in %s: %s" % (map_path, ", ".join(missing)), file=sys.stderr)
        print("(cursor.c's g_cursor_mouse_mode/g_cursor_x/g_cursor_y/g_cursor_click_pressed/g_cursor_click_held/g_cursor_click_released/g_cursor_click_flash_frames and main.c's test_harness_ready must be 'static' or otherwise map-visible)", file=sys.stderr)
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
