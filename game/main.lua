local Spot = require("spot")
local Camera = require("lib.camera")
local Panel = require("panel")
local SpotKinds = require("spot_kind")
local Probe = require("probe")

local ARROW_KEYS = { up = true, down = true, left = true, right = true }

local NATIVE_W = 240
local NATIVE_H = 160

local PANEL_WIDTH = 100

local view_size = 1024
local spot_count = 90
local spot = nil
local camera = nil
local selected = nil
local panel = nil
local canvas = nil
local font = nil
local probe = nil
local worm_hole=nil

function love.load()
    font = love.graphics.newImageFont("assets/font.png", " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")
    love.graphics.setBackgroundColor(0.1, 0.1, 0.1)
    spot = Spot.new_spot(spot_count, view_size,SpotKinds)

    canvas = love.graphics.newCanvas(NATIVE_W, NATIVE_H)
    canvas:setFilter("nearest", "nearest")

    local margin = 0.1 * math.min(NATIVE_W, NATIVE_H)
    camera = Camera.new(NATIVE_W, NATIVE_H, margin, (view_size-NATIVE_W) / 2, (view_size-NATIVE_H) / 2)
    panel = Panel.new()
    probe = Probe.new(view_size)
end

function love.update(dt)
    Camera.update(camera, dt)
    if worm_hole == nil then
        Probe.move_source_px(probe, {
            left = love.keyboard.isDown("left"),
            right = love.keyboard.isDown("right"),
            up = love.keyboard.isDown("up"),
            down = love.keyboard.isDown("down"),
        })
    end
    if worm_hole ~= nil and (selected==nil or love.keyboard.isDown("x")) then
        Probe.move_target_px(probe, {
            left=love.keyboard.isDown("left"),
            right=love.keyboard.isDown("right"),
            up=love.keyboard.isDown("up"),
            down=love.keyboard.isDown("down"),
        })
    end

end

function love.keypressed(key)
    if key == "escape" then love.event.quit() end
    if worm_hole == nil then
        if key == "space" then
            worm_hole = { x = probe.source.x, y = probe.source.y }
        end
        return
    end

    if key == "x" then
        local probe_target = Probe.get_valid_target(probe)
        if probe_target ~= nil then
            Spot.reveal_at(probe_target)
        end
        return
    end

    if selected == nil then return end
    if key == "space" then
        if panel:toogle() then
            camera.view_w = NATIVE_W - PANEL_WIDTH
            Camera.set_mode(camera, "center")
            Camera.set_target(camera, selected.x, selected.y)
        else
            camera.view_w = NATIVE_W
            Camera.set_mode(camera, "bound")
        end
        Panel.set_target(panel, selected)
    end
    if love.keyboard.isDown("x") then
        if panel.open then
            if key == "up" then
                Panel.prev_item(panel)
            end
            if key == "down" then
                Panel.next_item(panel)
            end
            if key == "right" then
                Panel.run(panel)
            end
            return
        end
    end

    if not ARROW_KEYS[key] then return end

    local direction = Camera.resolve_direction()
    if not direction then return end

    local neighbor = selected.neighbors[direction]
    if not neighbor then return end

    selected.selected=false
    selected = neighbor
    selected.selected = true

    Panel.set_target(panel, selected)
    Probe.set_source(probe,selected)
    Camera.set_target(camera, selected.x, selected.y)
end

function love.draw()
    love.graphics.setFont(font)
    love.graphics.setCanvas(canvas)
    love.graphics.clear(love.graphics.getBackgroundColor())
    Spot.draw(camera,spot)
    Panel.draw(camera, panel, NATIVE_W)
    Probe.draw(camera,probe)
    love.graphics.setCanvas()

    local win_w, win_h = love.graphics.getDimensions()
    local scale = math.max(1, math.floor(math.min(win_w / NATIVE_W, win_h / NATIVE_H)))
    local offset_x = (win_w - NATIVE_W * scale) / 2
    local offset_y = (win_h - NATIVE_H * scale) / 2

    love.graphics.setColor(1, 1, 1)
    love.graphics.draw(canvas, offset_x, offset_y, 0, scale, scale)
end
