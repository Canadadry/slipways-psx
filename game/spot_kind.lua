local function new(name,color,actions)
    return {
        name=name,
        color = color,
        actions = actions,
        run = function(self, selected_action)
            if self.actions[selected_action] == nil then return end
            self.name = self.name .. " " .. self.actions[selected_action]
            self.actions={}
        end,
        draw = function(self, selected,hidden, x, y)
            if selected then
                love.graphics.setColor(255, 255, 255)
                love.graphics.setLineWidth(1)
                love.graphics.circle("line", x, y, 8)
            end
            if hidden then
                love.graphics.setColor(255, 255, 255)
            else
                love.graphics.setColor(self.color.r,self.color.g,self.color.b)
            end
            love.graphics.circle("fill",x,y,5)
        end,
    }
end

return {
    new("red"  ,{ r= 255 , g =   0 , b = 0   },{"a to b","a to c","b to c"} ),
    new("green",{ r=   0 , g = 255 , b = 0   },{"b to c","c to d"} ),
    new("blue" ,{ r=   0 , g =   0 , b = 255 },{"d to a"} ),
}
