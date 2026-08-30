local EASE_RATE = 8

local function new(view_w, view_h, margin, x, y)
    x = x or 0
    y = y or 0
    return {
        x = x,
        y = y,
        target_x = x,
        target_y = y,
        view_w = view_w,
        view_h = view_h,
        margin = margin,
        mode = "bound",
    }
end

local function set_target(camera, x, y)
    if camera.mode == "center" then
        camera.target_x = x - camera.view_w / 2
        camera.target_y = y - camera.view_h / 2
        return
    end

    local left = camera.x + camera.margin
    local right = camera.x + camera.view_w - camera.margin
    if x < left then
        camera.target_x = x - camera.margin
    elseif x > right then
        camera.target_x = x - camera.view_w + camera.margin
    end

    local top = camera.y + camera.margin
    local bottom = camera.y + camera.view_h - camera.margin
    if y < top then
        camera.target_y = y - camera.margin
    elseif y > bottom then
        camera.target_y = y - camera.view_h + camera.margin
    end
end


local function set_mode(camera, mode)
    assert(mode == "bound" or mode == "center", "invalid camera mode: " .. tostring(mode))
    camera.mode = mode
    camera.target_x = camera.x
    camera.target_y = camera.y
end

local function update(camera, dt)
    local factor = 1 - math.exp(-EASE_RATE * dt)
    camera.x = camera.x + (camera.target_x - camera.x) * factor
    camera.y = camera.y + (camera.target_y - camera.y) * factor
end

local function resolve_direction()
    local up = love.keyboard.isDown("up")
    local down = love.keyboard.isDown("down")
    local left = love.keyboard.isDown("left")
    local right = love.keyboard.isDown("right")

    if up and right then return "ne" end
    if up and left then return "nw" end
    if down and right then return "se" end
    if down and left then return "sw" end
    if up then return "n" end
    if down then return "s" end
    if left then return "w" end
    if right then return "e" end
    return nil
end

return {
    new = new,
    set_target = set_target,
    set_mode = set_mode,
    update = update,
    resolve_direction = resolve_direction,
}
