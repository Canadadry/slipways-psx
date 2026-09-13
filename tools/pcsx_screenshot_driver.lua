
local FRAMES = tonumber(os.getenv("PCSX_SCREENSHOT_FRAMES")) or 90
local OUT = os.getenv("PCSX_SCREENSHOT_OUT")

if not OUT then
    error("PCSX_SCREENSHOT_OUT must be set")
end

local function u32le(n)
    return string.char(bit.band(n, 0xFF), bit.band(bit.rshift(n, 8), 0xFF), bit.band(bit.rshift(n, 16), 0xFF),
        bit.band(bit.rshift(n, 24), 0xFF))
end

local count = 0
local done = false

PCSX.Events.createEventListener("GPU::Vsync", function()
    if done then
        return
    end
    count = count + 1
    if count < FRAMES then
        return
    end
    done = true

    local ok, err = pcall(function()
        local ss = PCSX.GPU.takeScreenShot()
        local width = tonumber(ss.width)
        local height = tonumber(ss.height)
        local bpp = tonumber(ss.bpp)

        local f = io.open(OUT, "wb")
        if not f then
            error("could not open " .. OUT .. " for writing")
        end

        f:write(u32le(width))
        f:write(u32le(height))
        f:write(string.char(bpp))
        f:write(tostring(ss.data))
        f:close()

        print(string.format("OK: captured %dx%d bpp=%d frame %d to %s", width, height, bpp, count, OUT))
    end)

    if ok then
        PCSX.quit(0)
    else
        printError("FAIL: screenshot capture failed: " .. tostring(err))
        PCSX.quit(1)
    end
end)
