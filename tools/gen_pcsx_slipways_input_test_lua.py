import re
import sys

WANTED = ["g_pad_connected", "g_pad_buttons", "test_harness_ready"]

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

local G_PAD_CONNECTED_ADDR = __G_PAD_CONNECTED_ADDR__
local G_PAD_BUTTONS_ADDR = __G_PAD_BUTTONS_ADDR__
local READY_ADDR = __TEST_HARNESS_READY_ADDR__
local READY_MAGIC = 0x1eadbeef

local BOOT_TIMEOUT_FRAMES = 3000
local DPAD_HOLD_FRAMES = 12
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

-- PCSX.CONSTS.PAD.BUTTON.* are bit *indices* (BTN.UP == 4), matching how
-- setOverride/clearOverride take a button index. The raw g_pad_buttons
-- bitmask read back from guest RAM uses PSn00bSDK's own psxpad.h
-- convention (PAD_UP == 1<<4), so build the mask explicitly rather than
-- reusing BTN.UP as if it were already a bitmask.
local PAD_UP_MASK = bit.lshift(1, BTN.UP)

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

    assertEq(readS32(G_PAD_CONNECTED_ADDR), 1, "g_pad_connected should be 1 with a pad present")

    PAD.setOverride(BTN.UP)
    waitFrames(DPAD_HOLD_FRAMES)
    assertEq(bit.band(readS32(G_PAD_BUTTONS_ADDR), PAD_UP_MASK), 0,
        "PAD_UP bit should be clear (pressed) while holding Up")

    PAD.clearOverride(BTN.UP)
    waitFrames(SETTLE_FRAMES)
    assertEq(bit.band(readS32(G_PAD_BUTTONS_ADDR), PAD_UP_MASK), PAD_UP_MASK,
        "PAD_UP bit should be set (not pressed) after releasing Up")

    print("OK: all psx-slipways input assertions passed")
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
        print("usage: gen_pcsx_slipways_input_test_lua.py <slipways.map> <out.lua>", file=sys.stderr)
        return 1
    map_path, out_path = sys.argv[1], sys.argv[2]
    addrs = parse_map(map_path)
    missing = [name for name in WANTED if name not in addrs]
    if missing:
        print("missing symbols in %s: %s" % (map_path, ", ".join(missing)), file=sys.stderr)
        print("(input.c's g_pad_connected/g_pad_buttons and main.c's test_harness_ready must be 'static')", file=sys.stderr)
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
