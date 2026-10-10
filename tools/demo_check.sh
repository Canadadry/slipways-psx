#!/bin/sh
# usage: demo_check.sh <demo.json> <record state> <end state>...
# Prints `demo-check: <file>: <reason>` for each failure, exit 1 if any.

set -u
LC_ALL=C
export LC_ALL

if [ $# -lt 3 ]; then
    echo "usage: demo_check.sh <demo.json> <record state> <end state>..." >&2
    exit 2
fi

exec awk '
    function fail(file, reason) {
        print "demo-check: " file ": " reason
        failed = 1
    }

    # Reads the counted lines of a state file into values[] and reports a cheat;
    # returns 0 after reporting a missing file, a `stopped` state or a missing
    # line, 1 otherwise.
    function read_state(path, values,    line, f, n, status, stopped, key, read) {
        split("", values)
        stopped = 0
        while ((status = (getline line < path)) > 0) {
            n = split(line, f, " ")
            if (f[1] == "stopped") stopped = 1
            if (f[1] == "clock" && n == 5 && f[2] == "year") values["year"] = f[3]
            if (f[1] == "count" && n == 3 && (f[2] == "colonies" || f[2] == "slipways" || f[2] == "techs")) values[f[2]] = f[3]
            if (f[1] == "cheat" && n == 3 && f[2] == "activations") values["cheat activations"] = f[3]
        }
        close(path)
        if (status < 0) { fail(path, "no state"); return 0 }
        if (stopped) { fail(path, "stopped"); return 0 }
        read = 1
        for (key in line_of)
            if (!(key in values)) { fail(path, "no " line_of[key] " line"); read = 0 }
        if (read && values["cheat activations"] != 0)
            fail(path, "cheat activations " values["cheat activations"] ", expected 0")
        return read
    }

    BEGIN {
        line_of["year"] = "clock year"
        line_of["colonies"] = "count colonies"
        line_of["slipways"] = "count slipways"
        line_of["techs"] = "count techs"
        line_of["cheat activations"] = "cheat activations"
        split("year colonies slipways techs", counted, " ")
        # Section 2 of docs/plan-18.md, pinned on purpose: the spec the script is checked against.
        expected_n = split("build a trading empire\\nacross the stars|" \
                           "probe the dark\\nto find new planets|" \
                           "colonize planets:\\neach one needs and makes goods|" \
                           "link colonies with slipways\\nso goods can flow|" \
                           "end the year:\\ntrade earns money and science|" \
                           "spend science on new tech|" \
                           "slipways\\npress any button", expected, "|")
        demo = ARGV[1]
        failed = 0
        frames = 0
        captions = 0
        section = ""
        while ((status = (getline line < demo)) > 0) {
            if (line ~ /^  "captions": \[/) { section = "captions"; continue }
            if (line ~ /^  "frames": \[/) { section = "frames"; continue }
            if (line ~ /^  \]/) { section = ""; continue }
            if (section == "frames" && line ~ /^    \{/) frames++
            if (section == "captions") {
                if (line !~ /^    \{ "from": [0-9]+, "to": [0-9]+, "text": ".*" \},?$/) {
                    fail(demo, "caption line not understood: " line)
                    continue
                }
                captions++
                rest = line
                sub(/^    \{ "from": /, "", rest)
                caption_from[captions] = rest + 0
                sub(/^[0-9]+, "to": /, "", rest)
                caption_to[captions] = rest + 0
                sub(/^[0-9]+, "text": "/, "", rest)
                sub(/" \},?$/, "", rest)
                caption_text[captions] = rest
            }
        }
        close(demo)
        if (status < 0) fail(demo, "no file")
        if (status >= 0 && frames == 0) fail(demo, "no frame")
        if (frames > 3600) fail(demo, frames " frames, more than 3600")
        if (captions != expected_n) fail(demo, captions " captions, expected " expected_n)
        for (i = 1; i <= captions && i <= expected_n; i++)
            if (caption_text[i] != expected[i])
                fail(demo, "caption " i " reads \"" caption_text[i] "\", expected \"" expected[i] "\"")
        for (i = 1; i <= captions; i++) {
            start = i == 1 ? 0 : caption_to[i - 1]
            if (caption_from[i] != start) fail(demo, "caption " i " starts at " caption_from[i] ", expected " start)
        }
        if (captions > 0) {
            last_from = caption_from[captions]
            last_to = caption_to[captions]
            if (last_to != frames) fail(demo, "the closing caption ends at " last_to ", expected " frames " (the frame count)")
            if (last_to - last_from != 180)
                fail(demo, "the closing caption lasts " last_to - last_from " frames, expected 180")
        }
        record_read = read_state(ARGV[2], record)
        for (arg = 3; arg < ARGC; arg++) {
            if (!read_state(ARGV[arg], end) || !record_read) continue
            for (i = 1; i <= 4; i++) {
                key = counted[i]
                if (end[key] != record[key] + 1)
                    fail(ARGV[arg], key " " end[key] ", expected " record[key] + 1 " (the record'"'"'s " record[key] " + 1)")
            }
        }
        if (!failed)
            print "demo-check: " demo ": " frames " frames, " captions " captions, " ARGC - 3 " end state" (ARGC == 4 ? "" : "s") \
                " one year, colony, slipway and tech past the record, no cheat"
        exit failed
    }
' "$@"
