local T = require("lib.t")
local Neighbors = require("lib.neighbors")

local DIRS = { "n", "ne", "e", "se", "s", "sw", "w", "nw" }

local cases = {
    {
        name = "sole candidate also reaches its two adjacent diagonal directions",
        others = {
            { x = 10, y = 0 },
        },
        expected = { e = 1, se = 1, ne = 1 },
    },
    {
        name = "no candidate in a direction is nil",
        others = {},
        expected = {},
    },
    {
        name = "diagonal sole candidate also reaches its two adjacent cardinal directions",
        others = {
            { x = 10, y = 10 },
        },
        expected = { se = 1, e = 1, s = 1 },
    },
    {
        name = "multiple candidates in one direction: nearest wins, including for shared adjacent directions",
        others = {
            { x = 5, y = 0 },
            { x = 20, y = 0 },
        },
        expected = { e = 1, se = 1, ne = 1 },
    },
    {
        name = "all eight directions populated",
        others = {
            { x = 10, y = 0 },   -- e
            { x = 10, y = 10 },  -- se
            { x = 0, y = 10 },   -- s
            { x = -10, y = 10 }, -- sw
            { x = -10, y = 0 },  -- w
            { x = -10, y = -10 },-- nw
            { x = 0, y = -10 },  -- n
            { x = 10, y = -10 }, -- ne
        },
        expected = { e = 1, se = 2, s = 3, sw = 4, w = 5, nw = 6, n = 7, ne = 8 },
    },
    {
        name = "close off-angle spot beats far well-aligned spot",
        others = {
            { x = 2, y = 2.1 },  -- ~46 deg off east axis, distance ~2.9, just outside the old 45deg "e" cone
            { x = 30, y = 0 },   -- dead-on east, distance 30
        },
        -- the close spot is near enough to also win s/sw/se; the far spot is
        -- only ever a fallback where the close spot is too far off-angle (ne)
        expected = { e = 1, se = 1, s = 1, sw = 1, ne = 2 },
    },
}

for _, case in ipairs(cases) do
    T.test("find_nearest: " .. case.name, function()
        local subject = { x = 0, y = 0 }
        local spot = { subject }
        for _, o in ipairs(case.others) do
            table.insert(spot, o)
        end

        Neighbors.find_nearest(spot, subject)

        for _, dir in ipairs(DIRS) do
            local want_index = case.expected[dir]
            local want = want_index and case.others[want_index] or nil
            T.eq(subject.neighbors[dir], want, "direction " .. dir)
        end
    end)
end

T.report()
