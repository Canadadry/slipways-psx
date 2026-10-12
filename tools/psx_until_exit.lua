-- Run by tools/psx_boot.sh --until-exit through -dofile. Reads
-- PSX_CEILING_FRAMES, PSX_RESULT_FILE and PSX_PROGRESS_FILE from the
-- environment, plays PSX_PAD_SCRIPT through PSX_PAD_PLAYER, and drives
-- PSX_CAPTURE_SCRIPT through PSX_CAPTURE_PLAYER, each when both are
-- non-empty. The program quits through the -testmode exit register; at the
-- ceiling vsync, on a bad script or a capture out of step, this script quits
-- instead. On quitting it writes "exit <vsyncs>", "ceiling <vsyncs>" or
-- "error <message>" to the result file: the exit status alone cannot tell
-- them apart. Every 60 vsyncs it overwrites the progress file with
-- "vsync <vsyncs>", closed each time so that it survives an emulator abort.

local frames = tonumber(os.getenv('PSX_CEILING_FRAMES'))
local path = os.getenv('PSX_RESULT_FILE')
local progress_path = os.getenv('PSX_PROGRESS_FILE')
if frames == nil or path == nil or progress_path == nil then
    error('psx_until_exit.lua: PSX_CEILING_FRAMES, PSX_RESULT_FILE and PSX_PROGRESS_FILE must be set')
end

local vsyncs = 0
local result = nil

-- Listeners the garbage collector reclaims stop firing: keep them global.
psxResultListener = PCSX.Events.createEventListener('Quitting', function()
    local file = assert(io.open(path, 'w'))
    file:write(result or string.format('exit %d', vsyncs), '\n')
    file:close()
end)

-- Global: the capture player calls it from its vsync listener.
function psxBootFail(message)
    result = 'error ' .. tostring(message):gsub('\n', ' ')
    PCSX.quit(1)
end

for _, pair in ipairs({ { 'PSX_PAD_PLAYER', 'PSX_PAD_SCRIPT' }, { 'PSX_CAPTURE_PLAYER', 'PSX_CAPTURE_SCRIPT' } }) do
    local player = os.getenv(pair[1])
    local script = os.getenv(pair[2])
    if player ~= nil and player ~= '' and script ~= nil and script ~= '' then
        local ok, message = pcall(dofile, player)
        if not ok then
            psxBootFail(message)
            return
        end
    end
end

psxCeilingListener = PCSX.Events.createEventListener('GPU::Vsync', function()
    vsyncs = vsyncs + 1
    if vsyncs % 60 == 0 then
        local file = assert(io.open(progress_path, 'w'))
        file:write(string.format('vsync %d', vsyncs), '\n')
        file:close()
    end
    if vsyncs ~= frames then return end
    result = string.format('ceiling %d', vsyncs)
    PCSX.quit(1)
end)
