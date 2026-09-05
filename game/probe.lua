local prode_source_speed = 1
return {
    move_source_px = function(self, key)
        if key.up then
            self.source.y = self.source.y - prode_source_speed
        end
        if key.down then
            self.source.y = self.source.y + prode_source_speed
        end
        if key.left then
            self.source.x = self.source.x - prode_source_speed
        end
        if key.right then
            self.source.x = self.source.x + prode_source_speed
        end
    end,
    move_target_px = function(self, key)
        if self.target == nil then
            self.target = { x = self.source.x, y = self.source.y }
        end
        if key.up then
            self.target.y = self.target.y - prode_source_speed
        end
        if key.down then
            self.target.y = self.target.y + prode_source_speed
        end
        if key.left then
            self.target.x = self.target.x - prode_source_speed
        end
        if key.right then
            self.target.x = self.target.x + prode_source_speed
        end
    end,
    set_source = function(self, target)
        self.source.x = target.x
        self.source.y = target.y
    end,
    new=function(view_size)
        return {
            target = nil,
            source= {x=view_size/2,y=view_size/2},
        }
    end,
    draw = function(camera,self)
        love.graphics.setColor(255, 255, 0)
        love.graphics.circle("fill", self.source.x-camera.x, self.source.y-camera.y, 3)
        if self.target ~= nil then
            love.graphics.line(
                self.source.x-camera.x, self.source.y-camera.y,
                self.target.x-camera.x, self.target.y-camera.y
            )
        end
    end,
}
