local Spot = require("spot")
local Camera = require("lib.camera")

local ARROW_KEYS = { up = true, down = true, left = true, right = true }

local win_w = nil
local win_h = nil
local view_size = 1024
local spot_count = 90
local spot = nil
local camera = nil
local selected = nil

function love.load()
    love.graphics.setBackgroundColor(0.1, 0.1, 0.1)
    spot = Spot.new_spot(spot_count, view_size)
    win_w = love.graphics.getWidth()
    win_h = love.graphics.getHeight()

    selected = Spot.nearest(spot, view_size / 2, view_size / 2)
    selected.selected = true

    local margin = 0.1 * math.min(win_w, win_h)
    camera = Camera.new(win_w, win_h, margin, selected.x - win_w / 2, selected.y - win_h / 2)
end

function love.update(dt)
    Camera.update(camera, dt)
end

function love.keypressed(key)
    if not ARROW_KEYS[key] then return end

    local direction = Camera.resolve_direction()
    if not direction then return end

    local neighbor = selected.neighbors[direction]
    if not neighbor then return end

    selected.selected=false
    selected = neighbor
    selected.selected=true
    Camera.set_target(camera, selected.x, selected.y)
end

function love.draw()
    for index, s in ipairs(spot) do
        if s.selected then
            love.graphics.setColor(255,0,0)
        else
            love.graphics.setColor(255,255,255)
        end
        love.graphics.circle("fill", s.x - camera.x, s.y - camera.y, 10)
    end
end
