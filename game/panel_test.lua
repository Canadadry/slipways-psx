local T = require("lib.t")
local Panel = require("panel")

T.test("panel: set_target is a no-op while the panel is closed", function()
    local panel = Panel.new()
    Panel.set_target(panel, { title = "beta" })
    T.eq(panel.target, nil, "target stays nil while panel is closed")
end)

T.test("panel: closing the panel keeps the last target, only the open flag changes", function()
    local panel = Panel.new()
    panel.open = true
    Panel.set_target(panel, { title = "alpha" })

    panel.open = false

    T.eq(panel.open, false, "panel is closed")
    T.eq(panel.target.title, "alpha", "target is preserved after closing")
end)

T.report()
