import re
import sys

WANTED = ["g_cursor_mouse_mode", "g_cursor_x", "g_cursor_y", "test_harness_ready"]

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
        print("(cursor.c's g_cursor_mouse_mode/g_cursor_x/g_cursor_y and main.c's test_harness_ready must be 'static' or otherwise map-visible)", file=sys.stderr)
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
