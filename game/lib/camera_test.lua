local T = require("lib.t")
local Camera = require("lib.camera")

local cases = {
    {
        name = "target outside margin: camera eases toward the margin-corrected target",
        view_w = 100,
        view_h = 100,
        margin = 10,
        start = { x = 0, y = 0 },
        spot = { x = 95, y = 50 },
        want_target = { x = 5, y = 0 },
    },
    {
        name = "target already within margin: camera stays put",
        view_w = 100,
        view_h = 100,
        margin = 10,
        start = { x = 0, y = 0 },
        spot = { x = 50, y = 50 },
        want_target = { x = 0, y = 0 },
    },
}

local STEPS = 60
local DT = 0.1
local EPSILON = 0.01

for _, case in ipairs(cases) do
    T.test("camera: " .. case.name, function()
        local camera = Camera.new(case.view_w, case.view_h, case.margin, case.start.x, case.start.y)
        Camera.set_target(camera, case.spot.x, case.spot.y)

        local prev_dist_x = math.abs(camera.target_x - camera.x)
        local prev_dist_y = math.abs(camera.target_y - camera.y)

        for _ = 1, STEPS do
            Camera.update(camera, DT)

            local dist_x = math.abs(camera.target_x - camera.x)
            local dist_y = math.abs(camera.target_y - camera.y)
            T.eq(dist_x <= prev_dist_x, true, "x should not overshoot the target")
            T.eq(dist_y <= prev_dist_y, true, "y should not overshoot the target")
            prev_dist_x, prev_dist_y = dist_x, dist_y
        end

        T.eq(math.abs(camera.x - case.want_target.x) < EPSILON, true, "x converged to target")
        T.eq(math.abs(camera.y - case.want_target.y) < EPSILON, true, "y converged to target")
    end)
end


T.test("camera: in center mode, set_target eases toward centering x in the current view_w", function()
    local camera = Camera.new(240, 160, 10, 0, 0)
    camera.view_w= 140
    Camera.set_mode(camera, "center")
    Camera.set_target(camera, 300, 80)

    for _ = 1, STEPS do
        Camera.update(camera, DT)
    end

    -- centering spot_x=300 in a 140-wide viewport: camera.x = 300 - 140/2 = 230
    T.eq(math.abs(camera.x - 230) < EPSILON, true, "x converged to centered target")
end)

T.test("camera: in center mode, y still uses margin-based scrolling (x-only centering)", function()
    local camera = Camera.new(240, 160, 10, 0, 0)
    camera.view_w= 140
    Camera.set_mode(camera, "center")
    Camera.set_target(camera, 300, 200)

    for _ = 1, STEPS do
        Camera.update(camera, DT)
    end

    -- spot_y=200 is beyond bottom margin (view_h - margin = 150), so target_y = 200 - 160 + 10 = 50
    T.eq(math.abs(camera.y - 50) < EPSILON, true, "y follows the spot via margin scrolling, not centering")
end)

T.test("camera: set_mode freezes any in-flight animation, no drift on subsequent update", function()
    local camera = Camera.new(240, 160, 10, 0, 0)
    camera.view_w= 140
    Camera.set_mode(camera, "center")
    Camera.set_target(camera, 300, 80)
    Camera.update(camera, DT) -- animation in flight, camera.x has not reached target_x yet

    camera.view_w= 240
    Camera.set_mode(camera, "bound")
    local x_at_switch = camera.x

    for _ = 1, STEPS do Camera.update(camera, DT) end

    T.eq(camera.x, x_at_switch, "x does not drift after switching mode")
end)

T.test("camera: after round-tripping to center and back to bound, bound uses the restored view_w", function()
    local camera = Camera.new(240, 160, 10, 0, 0)
    camera.view_w= 140
    Camera.set_mode(camera, "center")
    Camera.set_target(camera, 300, 80)
    for _ = 1, STEPS do Camera.update(camera, DT) end

    camera.view_w= 240
    Camera.set_mode(camera, "bound")

    -- camera.x is now 230 (left over from centering); right bound margin is camera.x + 240 - 10 = 460
    Camera.set_target(camera, 455, 80)
    for _ = 1, STEPS do Camera.update(camera, DT) end

    -- 455 is within the 240-wide bound box, so camera.x should not have moved from 230
    T.eq(math.abs(camera.x - 230) < EPSILON, true, "bound mode respects the restored full view_w")
end)

T.report()
