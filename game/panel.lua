return {
    new = function(s)
        return {
            target=s
        }
    end,
    toogle = function(panel, s)
        if panel.target ~= nil then
            panel.target=nil
        else
            panel.target=s
        end
    end,
    draw = function(camera,panel)
        local w = 100
        local h = 160
        if panel.target == nil then
            return
        end
        local x = 0
        if (panel.target.x-camera.x) < camera.view_w/2 then
            x = camera.view_w -w
        end
        love.graphics.rectangle("fill", x, 0, w, h)
        love.graphics.print(panel.target.title, x+8, 8)
    end
}
