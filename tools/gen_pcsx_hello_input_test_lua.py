import re
import sys

WANTED = ["cursor_x", "cursor_y", "mouse_mode", "click_flash", "test_harness_ready"]

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

local CURSOR_X_ADDR = __CURSOR_X_ADDR__
local CURSOR_Y_ADDR = __CURSOR_Y_ADDR__
local MOUSE_MODE_ADDR = __MOUSE_MODE_ADDR__
local CLICK_FLASH_ADDR = __CLICK_FLASH_ADDR__
local READY_ADDR = __TEST_HARNESS_READY_ADDR__
local READY_MAGIC = 0x1eadbeef

local CURSOR_SPEED = 2
local CLICK_FLASH_FRAMES = 15

local BOOT_TIMEOUT_FRAMES = 3000
local PRESS_HOLD_FRAMES = 4
local SETTLE_FRAMES = 8
local DPAD_HOLD_FRAMES = 12
local CIRCLE_HOLD_FRAMES = 2
local CLICK_DECAY_FRAMES = CLICK_FLASH_FRAMES + 10

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
    error(string.format("psx-hello never reached its main loop within %d frames", BOOT_TIMEOUT_FRAMES))
end

local function readMouseMode()
    return readS32(MOUSE_MODE_ADDR)
end

-- Headless SDL/Xvfb input can deliver one spurious pre-existing keybinding
-- edge very early in boot (see docs/psx-hello-world.md), so mouse_mode is
-- not reliably 0 the moment psx-hello reaches its main loop. Force it to a
-- known state before asserting anything that depends on it, rather than
-- assuming a fresh boot starts with mouse_mode off.
local function driveMouseMode(target, msg)
    if readMouseMode() ~= target then
        press(BTN.CROSS, PRESS_HOLD_FRAMES)
        waitFrames(SETTLE_FRAMES)
    end
    assertEq(readMouseMode(), target, msg)
end

local function testBody()
    waitForBoot()
    waitFrames(SETTLE_FRAMES)

    driveMouseMode(0, "could not reach a known mouse_mode=off starting state")

    press(BTN.CROSS, PRESS_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readMouseMode(), 1, "Cross should turn mouse_mode on")

    local x0 = readS32(CURSOR_X_ADDR)
    local y0 = readS32(CURSOR_Y_ADDR)

    press(BTN.UP, DPAD_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)

    local x1 = readS32(CURSOR_X_ADDR)
    local y1 = readS32(CURSOR_Y_ADDR)

    assertEq(x1, x0, "cursor_x should not move while holding Up")
    local dy = y0 - y1
    local expectedDy = DPAD_HOLD_FRAMES * CURSOR_SPEED
    assertRange(dy, expectedDy - CURSOR_SPEED, expectedDy + CURSOR_SPEED,
        "cursor_y did not move by the expected amount while holding Up")

    press(BTN.CIRCLE, CIRCLE_HOLD_FRAMES)
    local clickFlash = readS32(CLICK_FLASH_ADDR)
    assertRange(clickFlash, 1, CLICK_FLASH_FRAMES, "Circle did not register a click (click_flash not set)")

    waitFrames(CLICK_DECAY_FRAMES)
    assertEq(readS32(CLICK_FLASH_ADDR), 0, "click_flash did not decay back to 0")

    press(BTN.CROSS, PRESS_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readMouseMode(), 0, "Cross should turn mouse_mode back off")

    print("OK: all psx-hello input assertions passed")
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
        print("usage: gen_pcsx_hello_input_test_lua.py <psx-hello.map> <out.lua>", file=sys.stderr)
        return 1
    map_path, out_path = sys.argv[1], sys.argv[2]
    addrs = parse_map(map_path)
    missing = [name for name in WANTED if name not in addrs]
    if missing:
        print("missing symbols in %s: %s" % (map_path, ", ".join(missing)), file=sys.stderr)
        print("(main.c's cursor_x/cursor_y/mouse_mode/click_flash/test_harness_ready must be 'static')", file=sys.stderr)
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
