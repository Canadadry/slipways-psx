-- Run by tools/psx_boot.sh --until-exit through -dofile. Reads
-- PSX_CEILING_FRAMES and PSX_RESULT_FILE from the environment. The program
-- quits through the -testmode exit register; at the ceiling vsync this script
-- quits instead. On quitting it writes "exit <vsyncs>" or "ceiling <vsyncs>"
-- to the file: the exit status alone cannot tell the two apart.

local frames = tonumber(os.getenv('PSX_CEILING_FRAMES'))
local path = os.getenv('PSX_RESULT_FILE')
if frames == nil or path == nil then
    error('psx_until_exit.lua: PSX_CEILING_FRAMES and PSX_RESULT_FILE must be set')
end

local vsyncs = 0
local reached = false

-- A listener the garbage collector reclaims stops firing: keep both global.
psxCeilingListener = PCSX.Events.createEventListener('GPU::Vsync', function()
    vsyncs = vsyncs + 1
    if vsyncs ~= frames then return end
    reached = true
    PCSX.quit(1)
end)

psxResultListener = PCSX.Events.createEventListener('Quitting', function()
    local file = assert(io.open(path, 'w'))
    file:write(string.format('%s %d\n', reached and 'ceiling' or 'exit', vsyncs))
    file:close()
end)
