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

T.report()
