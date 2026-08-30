local Spot = require("spot")
local Camera = require("lib.camera")
local Panel = require("panel")

local ARROW_KEYS = { up = true, down = true, left = true, right = true }

local NATIVE_W = 240
local NATIVE_H = 160

local view_size = 1024
local spot_count = 90
local spot = nil
local camera = nil
local selected = nil
local panel = nil
local canvas = nil

function love.load()
    love.graphics.setBackgroundColor(0.1, 0.1, 0.1)
    spot = Spot.new_spot(spot_count, view_size)

    canvas = love.graphics.newCanvas(NATIVE_W, NATIVE_H)
    canvas:setFilter("nearest", "nearest")

    selected = Spot.nearest(spot, view_size / 2, view_size / 2)
    selected.selected = true

    local margin = 0.1 * math.min(NATIVE_W, NATIVE_H)
    camera = Camera.new(NATIVE_W, NATIVE_H, margin, selected.x - NATIVE_W / 2, selected.y - NATIVE_H / 2)
    panel = Panel.new()
end

function love.update(dt)
    Camera.update(camera, dt)
end

function love.keypressed(key)
    if key == "escape" then love.event.quit() end
    if key == "space" then Panel.toogle(panel,selected) end

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
    love.graphics.setCanvas(canvas)
    love.graphics.clear(love.graphics.getBackgroundColor())
    Spot.draw(camera,spot)
    Panel.draw(panel)
    love.graphics.setCanvas()

    local win_w, win_h = love.graphics.getDimensions()
    local scale = math.max(1, math.floor(math.min(win_w / NATIVE_W, win_h / NATIVE_H)))
    local offset_x = (win_w - NATIVE_W * scale) / 2
    local offset_y = (win_h - NATIVE_H * scale) / 2

    love.graphics.setColor(1, 1, 1)
    love.graphics.draw(canvas, offset_x, offset_y, 0, scale, scale)
end
