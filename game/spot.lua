local Neighbors = require("lib.neighbors")

local function len(p)
    return math.sqrt(p.x*p.x+p.y*p.y)
end

local function dist(p1,p2)
    return len({x=p1.x-p2.x,y=p1.y-p2.y})
end

local function check_min_dist(spot, s1)
    for index, s2 in ipairs(spot) do
        if dist(s1, s2) < spot.min_dist then
            return false
        end
    end
    return true
end

local function add_spot(spot,kinds)
    local rand  = function() return math.random(spot.view_size-2*spot.border)+spot.border end
    local p = { x = rand(), y = rand(), selected = false,hidden = true }
    while check_min_dist(spot, p) == false do
        p = { x = rand(), y = rand(), selected = false,hidden = true }
    end
    p.kind = kinds[math.random(#kinds)]
    table.insert(spot,p)
end

local function visible_spot(spot)
    local visible = {}
    for _, s in ipairs(spot) do
        if s.hidden == false then
            table.insert(visible, s)
        end
    end
    return visible
end

local function nearest(spot, x, y)
    local best, best_dist2
    for _, s in ipairs(spot) do
        local dx, dy = s.x - x, s.y - y
        local dist2 = dx * dx + dy * dy
        if best_dist2 == nil or dist2 < best_dist2 then
            best_dist2 = dist2
            best = s
        end
    end
    return best
end

local function new_spot(spot_count, view_size,kinds)
    local spot = {
        count = 90,
        min_dist = 0.05 * view_size,
        border = 0.05 * view_size,
        view_size = view_size,
    }
    for i = 0, spot_count, 1 do
        add_spot(spot,kinds)
    end
    for _, point in ipairs(spot) do
        Neighbors.find_nearest(spot, point)
    end
    return spot
end

local function draw(camera,spot)
    for index, s in ipairs(spot) do
        s.kind:draw(s.selected,s.hidden, s.x - camera.x, s.y - camera.y)
    end
end

return {
    new_spot=new_spot,
    nearest = nearest,
    draw=draw,
}
