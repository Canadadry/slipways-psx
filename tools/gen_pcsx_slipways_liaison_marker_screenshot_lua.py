"""Adapts gen_pcsx_slipways_liaison_test_lua.py's memory-injection driver
technique (see that file's own extensive comments for the underlying
geometry/timing derivations, reused verbatim below rather than re-derived)
to also capture a GPU screenshot at one scripted point during a live
liaison drag -- verifying psx-sprites-audio-extraction.md chunk 6's
invalid-target marker (SPRITE_LIAISON_INVALID_TARGET) actually renders
when, and only when, liaison.live.reason is one of the three
marker-firing reasons.

liaison_route_t's g_liaison_route_count is permanently 0 in this build
(liaison.c's own comment: "no chunk ever writes g_liaison_route_count above
0" -- a future colonization step populates it), so LIAISON_ALREADY_CONNECTED
and LIAISON_CROSSES_ROUTE are both currently unreachable in a live game
session; only LIAISON_BLOCKED_BY_PLANET is exercisable end-to-end today.

The generated driver takes one screenshot per PCSX-Redux process
(mirroring pcsx_screenshot_driver.lua's own proven single-capture-per-run
shape, rather than chaining several PCSX.GPU.takeScreenShot() calls into
one long-lived session -- empirically, calling it more than twice in one
run corrupts the emulator's heap in this environment), selected by the
PCSX_LIAISON_MARKER_STAGE env var:
  1 = immediately after starting the drag (target == source planet, zero
      distance): LIAISON_VALID -- green line, no marker.
  2 = after moving the drag target beyond LIAISON_RANGE_SQ:
      LIAISON_TOO_FAR -- red line, no marker.
  3 = after moving the drag target onto a second planet whose connecting
      segment a third planet blocks: LIAISON_BLOCKED_BY_PLANET -- red
      line, WITH marker.
Each stage replays the same deterministic button sequence from a cold
boot up through the stage it needs, then captures and quits -- no state
is carried between separate PCSX-Redux invocations.
"""
import re
import sys

WANTED = [
    "g_cursor_mouse_mode",
    "g_liaison_dragging",
    "g_liaison_hover_index",
    "g_liaison_live_reason",
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
local G_LIAISON_HOVER_INDEX_ADDR = __G_LIAISON_HOVER_INDEX_ADDR__
local G_LIAISON_LIVE_REASON_ADDR = __G_LIAISON_LIVE_REASON_ADDR__
local READY_ADDR = __TEST_HARNESS_READY_ADDR__
local READY_MAGIC = 0x1eadbeef

local STAGE = tonumber(os.getenv("PCSX_LIAISON_MARKER_STAGE")) or 1
local OUT = os.getenv("PCSX_LIAISON_MARKER_OUT")
if not OUT then
    error("PCSX_LIAISON_MARKER_OUT must be set")
end

-- liaison.h's liaison_reason_t enum values.
local LIAISON_VALID = 0
local LIAISON_TOO_FAR = 2
local LIAISON_BLOCKED_BY_PLANET = 3

-- Same world_init() RNG-seed determinism gen_pcsx_slipways_liaison_test_lua.py
-- relies on. Planet 0 (screen (141,112)) is the same source planet that
-- driver already uses. A throwaway host-side driver
-- (psx-slipways/host_tests/scratch_find_blocked_triple.c, run once at
-- generation time, not committed) found planet 81 (screen (189,91)) as a
-- target whose segment from planet 0 is blocked by planet 26 (screen
-- (166,101)) -- LIAISON_BLOCKED_BY_PLANET, reachable and reproducible for
-- this build's fixed seed.
local UP_HOLD_FRAMES = 4
local LEFT_HOLD_FRAMES = 10
local DOWN_HOLD_FRAMES = 50
-- From the too-far rest point (140, 212) to planet 81's screen position
-- (189, 91): dx=+49 (RIGHT ~25 frames), dy=-121 (UP ~61 frames); overshoot
-- by a pixel or two is fine, LIAISON_PICK_RADIUS_PX (8) covers it.
local RIGHT_HOLD_FRAMES = 25
local UP2_HOLD_FRAMES = 61

local BOOT_TIMEOUT_FRAMES = 3000
local SETTLE_FRAMES = 8

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

local function u32le(n)
    return string.char(bit.band(n, 0xFF), bit.band(bit.rshift(n, 8), 0xFF), bit.band(bit.rshift(n, 16), 0xFF),
        bit.band(bit.rshift(n, 24), 0xFF))
end

local function captureScreenshot(outPath)
    local ss = PCSX.GPU.takeScreenShot()
    local width = tonumber(ss.width)
    local height = tonumber(ss.height)
    local bpp = tonumber(ss.bpp)

    local f = io.open(outPath, "wb")
    if not f then
        error("could not open " .. outPath .. " for writing")
    end
    f:write(u32le(width))
    f:write(u32le(height))
    f:write(string.char(bpp))
    f:write(tostring(ss.data))
    f:close()
    print(string.format("OK: captured %dx%d bpp=%d to %s", width, height, bpp, outPath))
end

local function testBody()
    waitForBoot()
    waitFrames(SETTLE_FRAMES)

    assertEq(readS32(G_CURSOR_MOUSE_MODE_ADDR), 0, "g_cursor_mouse_mode should start off")
    assertEq(readS32(G_LIAISON_DRAGGING_ADDR), 0, "g_liaison_dragging should start at 0")

    press(BTN.CROSS, SETTLE_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readS32(G_CURSOR_MOUSE_MODE_ADDR), 1, "Cross press should toggle g_cursor_mouse_mode on")

    press(BTN.UP, UP_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)
    press(BTN.LEFT, LEFT_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)

    PAD.setOverride(BTN.CIRCLE)
    waitFrames(SETTLE_FRAMES)

    assertEq(readS32(G_LIAISON_DRAGGING_ADDR), 1, "g_liaison_dragging should be 1 after starting the drag")

    if STAGE == 1 then
        assertEq(readS32(G_LIAISON_LIVE_REASON_ADDR), LIAISON_VALID,
            "(A) g_liaison_live_reason should be LIAISON_VALID at the drag's own starting planet")
        captureScreenshot(OUT)
        PCSX.quit(0)
        return
    end

    press(BTN.DOWN, DOWN_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readS32(G_LIAISON_DRAGGING_ADDR), 1, "g_liaison_dragging should still be 1 -- Circle is still held")

    if STAGE == 2 then
        assertEq(readS32(G_LIAISON_LIVE_REASON_ADDR), LIAISON_TOO_FAR,
            "(B) g_liaison_live_reason should be LIAISON_TOO_FAR after moving far away")
        captureScreenshot(OUT)
        PCSX.quit(0)
        return
    end

    press(BTN.RIGHT, RIGHT_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)
    press(BTN.UP, UP2_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)

    assertEq(readS32(G_LIAISON_DRAGGING_ADDR), 1, "g_liaison_dragging should still be 1 -- Circle is still held")
    assertEq(readS32(G_LIAISON_HOVER_INDEX_ADDR), 81,
        "g_liaison_hover_index should be planet 81 once the cursor is within its pick radius")
    assertEq(readS32(G_LIAISON_LIVE_REASON_ADDR), LIAISON_BLOCKED_BY_PLANET,
        "(C) g_liaison_live_reason should be LIAISON_BLOCKED_BY_PLANET once the drag targets planet 81")
    captureScreenshot(OUT)
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
        print(
            "usage: gen_pcsx_slipways_liaison_marker_screenshot_lua.py <slipways.map> <out.lua>",
            file=sys.stderr,
        )
        return 1
    map_path, out_path = sys.argv[1], sys.argv[2]
    addrs = parse_map(map_path)
    missing = [name for name in WANTED if name not in addrs]
    if missing:
        print("missing symbols in %s: %s" % (map_path, ", ".join(missing)), file=sys.stderr)
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
