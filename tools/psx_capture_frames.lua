-- Loaded by tools/psx_until_exit.lua when PSX_CAPTURE_SCRIPT is set. Drives
-- the GPU replay's frame-locked capture on the scratchpad words of
-- src/platform/psx_capture_protocol.h: hands it the next "<frame> <mask hex>"
-- of the script, and on FRAME_SHOWN waits one vsync, writes
-- <PSX_CAPTURE_PREFIX>.f<frame>.vram (psx_capture.lua's format), then hands
-- the next one. Raises an error naming the line on a bad script; calls
-- psxBootFail on a frame other than the expected one.

local STATE_OFFSET = 0x3f4
local FRAME_OFFSET = 0x3f0
local MASK_OFFSET = 0x3ec
local PROGRAM_READY = 0x43505244
local NEXT_READY = 0x434e5854
local FRAME_SHOWN = 0x43534857
local NO_FRAME = 0xffffffff

local path = os.getenv('PSX_CAPTURE_SCRIPT')
local prefix = os.getenv('PSX_CAPTURE_PREFIX')
if prefix == nil or prefix == '' then error('psx_capture_frames.lua: PSX_CAPTURE_PREFIX must be set') end

local entries = {}
local previous_frame = -1
local line_number = 0
for line in io.lines(path) do
    line_number = line_number + 1
    line = line:gsub('\r$', '')
    if not line:match('^%s*#') and not line:match('^%s*$') then
        local where = path .. ' line ' .. line_number .. ': '
        local frame, mask = line:match('^(%d+) (%x%x%x%x)$')
        if frame == nil then error(where .. 'expected <frame> <mask as 4 hex digits>: ' .. line) end
        frame = tonumber(frame)
        if frame <= previous_frame then error(where .. 'frame ' .. frame .. ' does not increase') end
        previous_frame = frame
        entries[#entries + 1] = { frame = frame, mask = tonumber(mask, 16) }
    end
end

local scratch = PCSX.getScratchPtr()
local state_word = ffi.cast('uint32_t *', scratch + STATE_OFFSET)
local frame_word = ffi.cast('uint32_t *', scratch + FRAME_OFFSET)
local mask_word = ffi.cast('uint32_t *', scratch + MASK_OFFSET)
local next_entry = 1
local shown_at = nil
local vsyncs = 0

local function hand_next()
    local entry = entries[next_entry]
    if entry == nil then
        frame_word[0] = NO_FRAME
        mask_word[0] = 0
    else
        frame_word[0] = entry.frame
        mask_word[0] = entry.mask
    end
    state_word[0] = NEXT_READY
end

local function write_capture(frame)
    local shot = PCSX.GPU.takeScreenShot()
    local depth = tonumber(shot.bpp) == 0 and 16 or 24
    local file = assert(io.open(string.format('%s.f%d.vram', prefix, frame), 'wb'))
    file:write(string.format('psx-vram %d %d %d\n', shot.width, shot.height, depth))
    file:write(tostring(shot.data))
    file:close()
end

-- A listener the garbage collector reclaims stops firing: keep it global.
psxCaptureFramesListener = PCSX.Events.createEventListener('GPU::Vsync', function()
    vsyncs = vsyncs + 1
    local state = state_word[0]
    if state == PROGRAM_READY then
        hand_next()
    elseif state == FRAME_SHOWN then
        if shown_at == nil then
            local expected = entries[next_entry]
            local frame = tonumber(frame_word[0])
            if expected == nil or expected.frame ~= frame then
                psxBootFail(string.format('capture: frame %d shown, expected %s', frame,
                    expected and tostring(expected.frame) or 'none'))
                return
            end
            shown_at = vsyncs
        elseif vsyncs == shown_at + 1 then
            write_capture(entries[next_entry].frame)
            next_entry = next_entry + 1
            shown_at = nil
            hand_next()
        end
    end
end)
