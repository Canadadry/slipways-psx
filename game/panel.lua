local function new()
    return {
        target = nil,
        open = false,
        toogle = function(self)
            self.open = not self.open
            return self.open
        end
    }
end

local function set_target(panel, s)
    if panel.open then
        panel.target = s
    end
end

local function draw(camera, panel, screen_w)
    local w = 100
    local h = 160
    if not panel.open then
        return
    end
    local x = screen_w - w
    love.graphics.setColor(255,255,255)
    love.graphics.rectangle("fill", x, 0, w, h)
    love.graphics.print(panel.target.kind.name, x+8, 8)
end

return {
    new = new,
    set_target = set_target,
    draw = draw,
}
