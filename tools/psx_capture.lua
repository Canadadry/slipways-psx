-- Run by tools/psx_boot.sh through -dofile. Reads PSX_CAPTURE_FRAMES and
-- PSX_CAPTURE_FILE from the environment; at that vsync, writes the capture
-- ("psx-vram <width> <height> <depth>\n" then takeScreenShot()'s raw bytes)
-- and quits with 0.

local frames = tonumber(os.getenv('PSX_CAPTURE_FRAMES'))
local path = os.getenv('PSX_CAPTURE_FILE')
if frames == nil or path == nil then
    error('psx_capture.lua: PSX_CAPTURE_FRAMES and PSX_CAPTURE_FILE must be set')
end

local vsyncs = 0

-- A listener the garbage collector reclaims stops firing: keep it global.
psxCaptureListener = PCSX.Events.createEventListener('GPU::Vsync', function()
    vsyncs = vsyncs + 1
    if vsyncs ~= frames then return end
    local shot = PCSX.GPU.takeScreenShot()
    local depth = tonumber(shot.bpp) == 0 and 16 or 24
    local file = assert(io.open(path, 'wb'))
    file:write(string.format('psx-vram %d %d %d\n', shot.width, shot.height, depth))
    file:write(tostring(shot.data))
    file:close()
    PCSX.quit(0)
end)
