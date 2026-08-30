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

local function add_spot(spot)
    local rand  = function() return math.random(spot.view_size-2*spot.border)+spot.border end
    local p = { x = rand(), y = rand() }
    while check_min_dist(spot,p)==false do
        p = { x = rand(), y = rand() }
    end
    table.insert(spot,p)
end


local function new_spot(spot_count,view_size)
    local spot = {
        count = 90,
        min_dist =  0.05 * view_size,
        border =  0.05 * view_size,
        view_size =  view_size,
    }
    for i = 0, spot_count, 1 do
        add_spot(spot)
    end
    return spot
end

return {
    new_spot=new_spot
}
