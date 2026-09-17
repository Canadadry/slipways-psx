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

local BOOT_TIMEOUT_FRAMES = 3000
local SETTLE_FRAMES = 8

-- rules_tech_step()'s edge detection only needs a single frame where
-- Select reads pressed to register the edge, but a real emulated SIO0
-- press is held for several frames here (mirroring
-- gen_pcsx_slipways_liaison_test_lua.py's own press() usage for Cross/
-- Circle) so the override is unambiguously observed by the poll that
-- samples pad state once per frame, before being released.
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

local function testBody()
    waitForBoot()
    waitFrames(SETTLE_FRAMES)

    -- (1) documented initial value after boot: the tech menu starts
    -- closed, mirroring rules_tech_init()'s g_rules_tech_open = 0.
    assertEq(readS32(G_RULES_TECH_OPEN_ADDR), 0, "g_rules_tech_open should start closed (0)")

    -- (2) a real Select press+release (SIO0 override injection, not a
    -- host-side call into rules_tech_step()) should flip the menu open,
    -- proving the live per-frame edge-toggle wiring in game_update.c.
    press(BTN.SELECT, SELECT_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readS32(G_RULES_TECH_OPEN_ADDR), 1, "g_rules_tech_open should flip to open (1) after a Select press")

    -- (3) a second Select press+release should toggle it back closed --
    -- this is the real per-press toggle proof this PRD's chunk 5 asks
    -- for, not just a single open transition.
    press(BTN.SELECT, SELECT_HOLD_FRAMES)
    waitFrames(SETTLE_FRAMES)
    assertEq(readS32(G_RULES_TECH_OPEN_ADDR), 0, "g_rules_tech_open should flip back to closed (0) after a second Select press")

    print("OK: all psx-slipways rules_tech Select-toggle assertions passed")
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
        print("usage: gen_pcsx_slipways_rules_tech_test_lua.py <slipways.map> <out.lua>", file=sys.stderr)
        return 1
    map_path, out_path = sys.argv[1], sys.argv[2]
    addrs = parse_map(map_path)
    missing = [name for name in WANTED if name not in addrs]
    if missing:
        print("missing symbols in %s: %s" % (map_path, ", ".join(missing)), file=sys.stderr)
        print("(rules_tech.c's g_rules_tech_open and main.c's test_harness_ready must be 'static' or otherwise map-visible)", file=sys.stderr)
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
