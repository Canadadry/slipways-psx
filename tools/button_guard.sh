#!/bin/sh
# usage: button_guard.sh <allowance> <directory>
# Counts, per `.c` and `.h` file of <directory>, the names starting with
# P8_PAD_BUTTON_ outside comments and string literals, and requires strict
# equality with the allowance (lines `<path> <owner> <count> | <reason>`,
# owner step-9 … step-13 or fixed, summed per path, a leading `#` for a
# comment). Prints the differences, then `button-guard files N reads M`.
# Exit 1 on any difference, 2 on an invalid or empty allowance or a missing
# directory: a guard over nothing would pass for green.

set -u

list=$1
dir=$2

fail() {
    echo "button_guard: $1" >&2
    exit 2
}

[ -d "$dir" ] || fail "directory not found: $dir"
[ -f "$list" ] || fail "allowance not found: $list"

# Word splitting below is deliberate: one argument per file, no path of src/
# has a space.
files=$(grep -rl --include='*.c' --include='*.h' 'P8_PAD_BUTTON_' "$dir")

exec awk -v list="$list" -v sq="'" '
    function sorted_print(lines, n,    i, j, t) {
        for (i = 2; i <= n; i++) {
            t = lines[i]
            for (j = i - 1; j >= 1 && lines[j] > t; j--) lines[j + 1] = lines[j]
            lines[j + 1] = t
        }
        for (i = 1; i <= n; i++) print lines[i]
    }

    function invalid(message) {
        print "button_guard: " message > "/dev/stderr"
        bad_list = 1
    }

    function list_line(    bar, why, n, field, path, owner, count, probe) {
        if ($0 ~ /^[[:space:]]*(#|$)/) return
        bar = index($0, "|")
        if (bar == 0) { invalid("line without a reason: " $0); return }
        why = substr($0, bar + 1)
        gsub(/^[[:space:]]+|[[:space:]]+$/, "", why)
        n = split(substr($0, 1, bar - 1), field, " ")
        if (n != 3) { invalid("not `<path> <owner> <count> | <reason>`: " $0); return }
        path = field[1]; owner = field[2]; count = field[3]
        if (why == "") invalid("empty reason: " path)
        if (owner !~ /^(step-(9|1[0-3])|fixed)$/) invalid("unknown owner " owner ": " path)
        if (count !~ /^[1-9][0-9]*$/) invalid("count not a positive integer: " $0)
        if ((getline probe < path) < 0) invalid("listed file missing: " path)
        close(path)
        if ((path " " owner) in seen) invalid("duplicate line: " path " " owner)
        seen[path " " owner] = 1
        want[path] += count
        entries++
    }

    # Comments become a space and string or character literals empty quotes,
    # so a name in a comment or an assert message is not a read.
    function source_line(    line, code, i, token, word) {
        line = $0
        code = ""
        while (line != "") {
            if (block) {
                i = index(line, "*/")
                if (i == 0) break
                line = substr(line, i + 2)
                block = 0
                code = code " "
                continue
            }
            if (!match(line, "/\\*|//|\"|" sq)) { code = code line; break }
            code = code substr(line, 1, RSTART - 1)
            token = substr(line, RSTART, RLENGTH)
            line = substr(line, RSTART + RLENGTH)
            if (token == "//") break
            if (token == "/*") { block = 1; code = code " "; continue }
            if (match(line, "^([^" token "\\\\]|\\\\.)*" token)) line = substr(line, RLENGTH + 1)
            else line = ""
            code = code token token
        }
        while (match(code, /[A-Za-z0-9_]+/)) {
            word = substr(code, RSTART, RLENGTH)
            code = substr(code, RSTART + RLENGTH)
            if (word !~ /^P8_PAD_BUTTON_/) continue
            got[FILENAME]++
            reads++
        }
    }

    FILENAME == list { list_line(); next }
    { source_line() }

    END {
        if (entries == 0 && !bad_list) invalid("empty list")
        if (bad_list) exit 2
        n = 0
        used_files = 0
        for (path in got) {
            used_files++
            if (!(path in want)) out[++n] = sprintf("button_guard: %s names %d button(s), not listed", path, got[path])
            else if (got[path] != want[path]) out[++n] = sprintf("button_guard: %s names %d button(s), listed %d", path, got[path], want[path])
        }
        for (path in want) {
            if (!(path in got)) out[++n] = sprintf("button_guard: %s names no button, listed %d", path, want[path])
        }
        sorted_print(out, n)
        printf "button-guard files %d reads %d\n", used_files, reads + 0
        exit (n > 0)
    }' "$list" $files
