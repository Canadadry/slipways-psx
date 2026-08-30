local T = { pass = 0, fail = 0, _suite_active = false, _filter_test = nil }

function T.test(name, fn)
    if not T._suite_active then
        error("T.test called outside T.run_test_suite — run tests via make test-game", 2)
    end
    if T._filter_test and not name:find(T._filter_test, 1, true) then
        return
    end
    local ok, err = pcall(fn)
    if ok then
        print("PASS " .. name)
        T.pass = T.pass + 1
    else
        print("FAIL " .. name)
        print("     " .. tostring(err))
        T.fail = T.fail + 1
    end
end

function T.eq(a, b, msg)
    if a ~= b then
        error((msg or "eq") .. ": expected " .. tostring(b) .. ", got " .. tostring(a), 2)
    end
end

function T.report()
    local p, f = T.pass, T.fail
    T.pass, T.fail = 0, 0
    print(string.format("%d passed, %d failed\n", p, f))
    if f > 0 then os.exit(1) end
end

-- opts.exclude     — optional -not -path pattern string
-- opts.before_each — optional function called before each suite file
-- CLI: -f <str>   — only run suites whose path contains <str>
--      -t <str>   — only run tests whose name contains <str>
function T.run_test_suite(opts)
    opts = opts or {}

    local filter_file, filter_test
    if arg then
        local i = 1
        while arg[i] do
            if arg[i] == "-f" and arg[i + 1] then
                filter_file = arg[i + 1]; i = i + 2
            elseif arg[i] == "-t" and arg[i + 1] then
                filter_test = arg[i + 1]; i = i + 2
            else
                i = i + 1
            end
        end
    end

    local cmd = "find . -name '*_test.lua'"
    if opts.exclude then
        cmd = cmd .. " -not -path '" .. opts.exclude .. "'"
    end
    cmd = cmd .. " | sort"

    local handle = io.popen(cmd)
    local suites = {}
    for path in handle:lines() do
        local p = path:gsub("^%./", "")
        if not filter_file or p:find(filter_file, 1, true) then
            suites[#suites + 1] = p
        end
    end
    handle:close()

    T._suite_active = true
    T._filter_test  = filter_test
    local real_exit = os.exit
    local failed    = 0

    os.exit = function(code)
        if (code or 0) ~= 0 then error("__suite_failed__") end
    end

    for _, path in ipairs(suites) do
        if opts.before_each then opts.before_each() end
        print(path)
        local ok, err = pcall(dofile, path)
        if not ok then
            if type(err) ~= "string" or not err:find("__suite_failed__") then
                print("ERROR in " .. path .. ": " .. tostring(err))
            end
            failed = failed + 1
        end
    end

    local total = #suites
    print(string.format("\n=== %d/%d suites passed ===", total - failed, total))
    real_exit(failed > 0 and 1 or 0)
end

return T
