import re
import sys

WANTED = ["g_frame_count", "test_harness_ready"]

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

local G_FRAME_COUNT_ADDR = __G_FRAME_COUNT_ADDR__
local READY_ADDR = __TEST_HARNESS_READY_ADDR__
local READY_MAGIC = 0x1eadbeef

local BOOT_TIMEOUT_FRAMES = 3000
local SETTLE_FRAMES = 8

-- PRD-specified measurement window: ~3 emulated NTSC seconds (long enough
-- to average out single-frame jitter, short enough to keep the test fast).
local MEASURE_WINDOW_VSYNCS = 180

-- R3000A's fixed NTSC clock (standard PS1 hardware fact, not inferred).
local CPU_CLOCK_HZ = 33868800.0

-- Feasibility criterion from docs/plan.md.
local MIN_ACHIEVED_FPS = 30

local MEM = PCSX.getMemPtr()

local function readS32(addr)
    return ffi.cast("int32_t*", MEM + bit.band(addr, 0x1fffff))[0]
end

local function waitFrames(n)
    for i = 1, n do
        coroutine.yield()
    end
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

    -- tonumber(): getCPUCycles() returns a uint64_t cdata: mixed cdata/
    -- number arithmetic in LuaJIT truncates to integer instead of
    -- promoting to a double, and string.format("%f", ...) rejects cdata
    -- outright -- unbox to a plain Lua number right away.
    local cycles_start = tonumber(PCSX.getCPUCycles())
    local frames_start = readS32(G_FRAME_COUNT_ADDR)

    waitFrames(MEASURE_WINDOW_VSYNCS)

    local cycles_end = tonumber(PCSX.getCPUCycles())
    local frames_end = readS32(G_FRAME_COUNT_ADDR)

    -- Emulated-time FPS, derived from the cycle-accurate R3000A clock, not
    -- host wall-clock time -- see docs/prd/triage/psx-galaxy-rendering.md's
    -- "FPS measurement" section. Validates this step's static-scene GPU-
    -- submission cost only, not later phases' per-frame trigonometry /
    -- liaison-recalculation cost.
    local elapsed_emulated_seconds = (cycles_end - cycles_start) / CPU_CLOCK_HZ
    local frames_rendered = frames_end - frames_start
    local achieved_fps = frames_rendered / elapsed_emulated_seconds

    print(string.format("achieved_fps=%.3f (frames_rendered=%d, elapsed_emulated_seconds=%.6f)",
        achieved_fps, frames_rendered, elapsed_emulated_seconds))

    if achieved_fps < MIN_ACHIEVED_FPS then
        printError(string.format("FAIL: achieved_fps=%.3f is below the %d fps feasibility criterion",
            achieved_fps, MIN_ACHIEVED_FPS))
        PCSX.quit(1)
        return
    end

    print(string.format("OK: achieved_fps=%.3f meets the %d fps feasibility criterion",
        achieved_fps, MIN_ACHIEVED_FPS))
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
        print("usage: gen_pcsx_slipways_fps_test_lua.py <slipways.map> <out.lua>", file=sys.stderr)
        return 1
    map_path, out_path = sys.argv[1], sys.argv[2]
    addrs = parse_map(map_path)
    missing = [name for name in WANTED if name not in addrs]
    if missing:
        print("missing symbols in %s: %s" % (map_path, ", ".join(missing)), file=sys.stderr)
        print("(main.c's g_frame_count/test_harness_ready must be 'static')", file=sys.stderr)
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
