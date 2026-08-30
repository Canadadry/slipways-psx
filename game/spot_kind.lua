local function new(name,color)
    return {
        name=name,
        color=color,
        draw = function(self, selected, x, y)
            if selected then
                love.graphics.setColor(255, 255, 255)
                love.graphics.setLineWidth(1)
                love.graphics.circle("line",x,y,8)
            end
            love.graphics.setColor(self.color.r,self.color.g,self.color.b)
            love.graphics.circle("fill",x,y,5)
        end,
    }
end

return {
    new("red"  ,{ r= 255 , g =   0 , b = 0   } ),
    new("green",{ r=   0 , g = 255 , b = 0   } ),
    new("blue" ,{ r=   0 , g =   0 , b = 255 } ),
}
