local Spot =require("spot")

local win_w=nil
local win_h=nil
local view_size = 1024
local spot_count = 90
local spot = nil
local camera_x = 0
local camera_y = 0

function love.load()
    love.graphics.setBackgroundColor(0.1, 0.1, 0.1)
    spot = Spot.new_spot(spot_count, view_size)
    win_w = love.graphics.getWidth()
    win_h = love.graphics.getHeight()
    camera_x = ( view_size - win_w ) / 2
    camera_y = ( view_size - win_h ) / 2
end

function love.update(dt)
    local camera_speed = 1
    if love.keyboard.isDown("left")  then camera_x = camera_x - camera_speed end
    if love.keyboard.isDown("right") then camera_x = camera_x + camera_speed end
    if love.keyboard.isDown("up")    then camera_y = camera_y - camera_speed end
    if love.keyboard.isDown("down")  then camera_y = camera_y + camera_speed end
end


function love.draw()
    for index, s in ipairs(spot) do
        love.graphics.circle("fill",s.x-camera_x,s.y-camera_y,10)
    end
end
