-- Loaded by tools/psx_until_exit.lua when PSX_PAD_SCRIPT is set. Plays the
-- pad script (format in tools/psx_pad_compare.sh) on port 1 through the pad
-- overrides, offsets counted from the vsync on which the probe's handshake
-- word appears in the last scratchpad word. END_DELAY vsyncs after the last
-- step it writes the end word just below it, telling the probe the script is
-- over. Raises an error naming the line on a bad script.

local HANDSHAKE_OFFSET = 0x3fc
local END_OFFSET = 0x3f8
local HANDSHAKE_MAGIC = 0x50414421
local END_MAGIC = 0x454e4421
local END_DELAY = 10

local path = os.getenv('PSX_PAD_SCRIPT')
local buttons = PCSX.CONSTS.PAD.BUTTON
local steps = {}
local previous_offset = -1
local line_number = 0
for line in io.lines(path) do
    line_number = line_number + 1
    line = line:gsub('\r$', '')
    if not line:match('^%s*#') and not line:match('^%s*$') then
        local where = path .. ' line ' .. line_number .. ': '
        local words = {}
        for word in line:gmatch('%S+') do words[#words + 1] = word end
        local offset = words[1]:match('^%d+$') and tonumber(words[1])
        if not offset then error(where .. 'vsync offset is not an integer: ' .. words[1]) end
        if offset <= previous_offset then error(where .. 'vsync offset ' .. offset .. ' does not increase') end
        if #words < 2 then error(where .. 'no button set (use - for none)') end
        previous_offset = offset
        local held = {}
        if not (#words == 2 and words[2] == '-') then
            for i = 2, #words do
                local index = buttons[words[i]]
                if index == nil then error(where .. 'unknown button ' .. words[i]) end
                held[index] = true
            end
        end
        steps[#steps + 1] = { offset = offset, held = held }
    end
end
if #steps == 0 then error(path .. ': no step') end
if next(steps[#steps].held) ~= nil then error(path .. ': last step must be - (no button held)') end

local pad = PCSX.SIO0.slots[1].pads[1]
local scratch = PCSX.getScratchPtr()
local handshake_word = ffi.cast('uint32_t *', scratch + HANDSHAKE_OFFSET)
local end_word = ffi.cast('uint32_t *', scratch + END_OFFSET)
local start = nil
local vsyncs = 0
local next_step = 1

local function apply(held)
    for _, index in pairs(buttons) do
        if held[index] then pad.setOverride(index) else pad.clearOverride(index) end
    end
end

-- A listener the garbage collector reclaims stops firing: keep it global.
psxPadPlayListener = PCSX.Events.createEventListener('GPU::Vsync', function()
    vsyncs = vsyncs + 1
    if start == nil then
        if handshake_word[0] ~= HANDSHAKE_MAGIC then return end
        start = vsyncs
    end
    local offset = vsyncs - start
    local step = steps[next_step]
    if step ~= nil and step.offset == offset then
        apply(step.held)
        next_step = next_step + 1
    elseif step == nil and offset == steps[#steps].offset + END_DELAY then
        end_word[0] = END_MAGIC
    end
end)
