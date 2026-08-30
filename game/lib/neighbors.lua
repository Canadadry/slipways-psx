local DIRS = { "e", "se", "s", "sw", "w", "nw", "n", "ne" }

local DIR_ANGLE = {
    e = 0, se = 45, s = 90, sw = 135, w = 180, nw = 225, n = 270, ne = 315,
}

local ANGLE_PENALTY_K = 2
local MAX_ANGLE_DIFF = 90

local function angle_diff(a, b)
    local d = math.abs(a - b) % 360
    if d > 180 then d = 360 - d end
    return d
end

local function score(dist, diff)
    return dist * (1 + ANGLE_PENALTY_K * (diff / 180))
end

local function find_nearest(spot, s)
    local best_score = {}
    local best = {}
    for _, dir in ipairs(DIRS) do
        best[dir] = nil
    end

    for _, other in ipairs(spot) do
        if other ~= s then
            local dx = other.x - s.x
            local dy = other.y - s.y
            local dist = math.sqrt(dx * dx + dy * dy)
            local angle = math.deg(math.atan2(dy, dx)) % 360

            for _, dir in ipairs(DIRS) do
                local diff = angle_diff(angle, DIR_ANGLE[dir])
                if diff < MAX_ANGLE_DIFF then
                    local candidate_score = score(dist, diff)
                    if best_score[dir] == nil or candidate_score < best_score[dir] then
                        best_score[dir] = candidate_score
                        best[dir] = other
                    end
                end
            end
        end
    end

    s.neighbors = best
end

return {
    find_nearest = find_nearest,
}
