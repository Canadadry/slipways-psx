"""Adapts gen_pcsx_slipways_rules_tech_test_lua.py's Select-toggle pad-
injection technique (opening the tech menu via a real emulated SIO0
Select press, not a host-side call into rules_tech_step()) and
gen_pcsx_slipways_liaison_marker_screenshot_lua.py's single-capture-per-
process GPU screenshot technique (see that file's own comment for why:
calling PCSX.GPU.takeScreenShot() more than twice in one PCSX-Redux run
empirically corrupts the emulator's heap in this environment) to verify
psx-sprites-audio-extraction.md chunk 8's tech-menu resource icons
(SPRITE_RESOURCE_TECH/ORGANICS/ORE/BOTS) actually render once the tech
menu is open, on top of the existing researched/unresearched TILE --
game_draw.c's tech-menu tile loop is otherwise never exercised by any
other PCSX verification driver (psx-slipways-screenshot-pcsx's own driver
never presses Select).

Single stage, single screenshot per run (no multi-stage env var needed,
unlike the liaison-marker driver -- there is only one thing to verify
here: the menu open with its 4 tiles+icons visible)."""
import re
import sys

WANTED = [
    "g_rules_tech_open",
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

local G_RULES_TECH_OPEN_ADDR = __G_RULES_TECH_OPEN_ADDR__
local READY_ADDR = __TEST_HARNESS_READY_ADDR__
local READY_MAGIC = 0x1eadbeef

local OUT = os.getenv("PCSX_TECH_ICON_OUT")
if not OUT then
    error("PCSX_TECH_ICON_OUT must be set")
end

local BOOT_TIMEOUT_FRAMES = 3000
local SETTLE_FRAMES = 8

-- Same reasoning as gen_pcsx_slipways_rules_tech_test_lua.py's own
-- SELECT_HOLD_FRAMES: rules_tech_step()'s edge detection only needs a
-- single frame where Select reads pressed, but the emulated SIO0 press is
-- held for several frames so the once-per-frame pad poll unambiguously
-- observes it.
local SELECT_HOLD_FRAMES = 4

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

    assertEq(readS32(G_RULES_TECH_OPEN_ADDR), 0, "g_rules_tech_open should start closed (0)")

    press(BTN.SELECT, SELECT_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readS32(G_RULES_TECH_OPEN_ADDR), 1, "g_rules_tech_open should flip to open (1) after a Select press")

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
            "usage: gen_pcsx_slipways_tech_icon_screenshot_lua.py <slipways.map> <out.lua>",
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
