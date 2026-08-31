local function new(actions)
    return {
        target = nil,
        open = false,
        actions = actions,
        selected = 0,
        toogle = function(self)
            self.open = not self.open
            if not self.open then
                self.selected = 0
            end
            return self.open
        end
    }
end

local function set_target(panel, s)
    if panel.open then
        panel.selected = 0
        panel.target = s
    end
end

local function next_item(panel)
    if panel.open and panel.target ~= nil then
        panel.selected = panel.selected + 1
        if panel.selected > #panel.target.kind.actions then
            panel.selected=1
        end
    end
end

local function prev_item(panel)
    if panel.open and panel.target ~= nil then
        panel.selected = panel.selected - 1
        if panel.selected < 1 then
            panel.selected = #panel.target.kind.actions
        end
    end
end

local function run(panel)
    panel.target.kind:run(panel.selected)
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
    love.graphics.print(panel.target.kind.name, x + 8, 8)
    for index, action in ipairs(panel.target.kind.actions) do
        local offset_y = index * 8 + 8
        if index == panel.selected then
            love.graphics.setColor(255,0,0)
        else
            love.graphics.setColor(255,255,255)
        end
        love.graphics.print(action, x+16, offset_y)
    end
end

return {
    new = new,
    set_target = set_target,
    draw = draw,
    next_item = next_item,
    prev_item = prev_item,
    run=run,
}
