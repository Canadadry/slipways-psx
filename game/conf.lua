local native_w = 240
local native_h = 160
local scale = 3

function love.conf(t)
    t.window.title = "Slipways"
    t.window.width = native_w * scale
    t.window.height = native_h * scale
    t.window.resizable = true
end
