local DIRS = { "e", "se", "s", "sw", "w", "nw", "n", "ne" }

local function direction_of(dx, dy)
    local angle_deg = math.deg(math.atan2(dy, dx))
    local index = math.floor(((angle_deg + 22.5) % 360) / 45) + 1
    return DIRS[index]
end

local function find_nearest(spot, s)
    local best_dist = {}
    local best = {}
    for _, dir in ipairs(DIRS) do
        best[dir] = nil
    end

    for _, other in ipairs(spot) do
        if other ~= s then
            local dx = other.x - s.x
            local dy = other.y - s.y
            local dist2 = dx * dx + dy * dy
            local dir = direction_of(dx, dy)
            if best_dist[dir] == nil or dist2 < best_dist[dir] then
                best_dist[dir] = dist2
                best[dir] = other
            end
        end
    end

    s.neighbors = best
end

return {
    find_nearest = find_nearest,
}
