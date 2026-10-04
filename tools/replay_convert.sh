#!/bin/sh
# usage: replay_convert.sh convert <events> <labels> <table> <out> <report>
#        replay_convert.sh first-divergence <labels> <labels>
#        replay_convert.sh stale-pairs <txt> <events 128> <events converted>
# Temporary (plan 12), outside the suite.
# convert: frame n of the 128x128 events takes the offset of the anchor of
#   line n + 1 of the labels; only the two numbers after "mouse_x": and
#   "mouse_y": change. Exit 1, writing neither out nor report, when the frame
#   and label counts differ, a label is not in the table or a frame line does
#   not read.

set -u

convert() {
    events=$1 labels=$2 table=$3 out=$4 report=$5
    for f in "$events" "$labels" "$table"; do
        [ -f "$f" ] || { echo "missing: $f" >&2; exit 1; }
    done
    work=$(mktemp -d "${TMPDIR:-/tmp}/replay_convert.XXXXXX")
    trap 'rm -rf "$work"' EXIT
    : > "$work/report"
    # The offsets are those of the header of tools/cursor_anchors.txt.
    awk -v table="$table" -v labels="$labels" -v report="$work/report" '
        BEGIN {
            ox["gauche"] = 0; ox["centre"] = 16; ox["droite"] = 32
            oy["haut"] = 0; oy["centre"] = -4; oy["bas"] = -8
            while ((getline line < table) > 0) {
                if (line ~ /^#/ || line ~ /^[ \t]*$/) continue
                n = split(line, f, " ")
                if (n == 2 && f[2] == "carte") { dx[f[1]] = 16; dy[f[1]] = -4 }
                else if (n == 3 && (f[2] in ox) && (f[3] in oy)) { dx[f[1]] = ox[f[2]]; dy[f[1]] = oy[f[3]] }
                else fail("bad table line: " line)
            }
            nlab = 0
            while ((getline line < labels) > 0) lab[nlab++] = line
            frame = 0
        }
        # Sets num_at and num_len to the integer after "key": (spaces
        # skipped); 0 when it does not read.
        function number(key,    p, rest) {
            p = index($0, "\"" key "\":")
            if (p == 0) return 0
            p += length(key) + 3
            rest = substr($0, p)
            if (!match(rest, /^ *-?[0-9]+/)) return 0
            match(rest, /-?[0-9]+/)
            num_at = p + RSTART - 1
            num_len = RLENGTH
            return 1
        }
        function fail(message) {
            print message > "/dev/stderr"
            bad = 1
            exit 1
        }
        /"mouse_x"|"mouse_y"/ {
            if (gsub(/"mouse_x"/, "&") != 1 || gsub(/"mouse_y"/, "&") != 1)
                fail("unreadable frame " frame ": " $0)
            if (!number("mouse_x")) fail("unreadable frame " frame ": " $0)
            xs = num_at; xl = num_len
            if (!number("mouse_y") || num_at < xs) fail("unreadable frame " frame ": " $0)
            ys = num_at; yl = num_len
            x = substr($0, xs, xl) + 0; y = substr($0, ys, yl) + 0
            if (frame >= nlab) fail("more frames than the " nlab " label(s)")
            l = lab[frame]
            if (!(l in dx)) fail("frame " frame ": label not in the table: " l)
            nx = x + dx[l]; ny = y + dy[l]
            cx = nx < 0 ? 0 : nx > 159 ? 159 : nx
            cy = ny < 0 ? 0 : ny > 119 ? 119 : ny
            if (cx != nx || cy != ny)
                printf "recadre %d %d %d -> %d %d %s\n", frame, x, y, cx, cy, l > report
            nx = cx; ny = cy
            key = x "," y " -> " nx "," ny
            if (!(key in frames)) { order[npos++] = key; frames[key] = frame; last[key] = frame; open_[key] = 0 }
            else if (frame == last[key] + 1) { last[key] = frame; open_[key] = 1 }
            else { frames[key] = frames[key] (open_[key] ? "-" last[key] : "") "," frame; last[key] = frame; open_[key] = 0 }
            $0 = substr($0, 1, xs - 1) nx substr($0, xs + xl, ys - xs - xl) ny substr($0, ys + yl)
            frame++
        }
        { print }
        END {
            if (bad) exit 1
            if (frame != nlab) fail(frame " frame(s) but " nlab " label(s)")
            for (i = 0; i < npos; i++) {
                key = order[i]
                printf "position %s %s\n", key, frames[key] (open_[key] ? "-" last[key] : "") > report
            }
        }
    ' "$events" > "$work/out" || exit 1
    mv "$work/out" "$out" && mv "$work/report" "$report"
}

first_divergence() {
    for f in "$1" "$2"; do
        [ -f "$f" ] || { echo "missing: $f" >&2; exit 1; }
    done
    awk -v old="$1" -v new="$2" '
        BEGIN {
            for (n = 0; ; n++) {
                a = (getline la < old) > 0
                b = (getline lb < new) > 0
                if (!a && !b) { print "identique"; exit }
                if (!a) la = "absente"
                if (!b) lb = "absente"
                if (!a || !b || la != lb) { print "frame " n " " la " " lb; exit }
            }
        }'
}

stale_pairs() {
    for f in "$1" "$2" "$3"; do
        [ -f "$f" ] || { echo "missing: $f" >&2; exit 1; }
    done
    awk -v old="$2" -v new="$3" '
        function point(line,    x, y) {
            if (!match(line, /"mouse_x": *-?[0-9]+/)) return ""
            x = substr(line, RSTART, RLENGTH); sub(/.*: */, "", x)
            if (!match(line, /"mouse_y": *-?[0-9]+/)) return ""
            y = substr(line, RSTART, RLENGTH); sub(/.*: */, "", y)
            return x "," y
        }
        BEGIN {
            while ((getline lo < old) > 0) {
                po = point(lo)
                if (po == "") continue
                if ((getline ln < new) <= 0) break
                while (point(ln) == "" && (getline ln < new) > 0) ;
                pn = point(ln)
                converted[pn] = 1
                if (po != pn) moved[po] = 1
            }
        }
        {
            rest = $0
            while (match(rest, /\(-?[0-9]+, *-?[0-9]+\)/)) {
                pair = substr(rest, RSTART, RLENGTH)
                key = pair; gsub(/[() ]/, "", key)
                if ((key in moved) && !(key in converted)) print "ligne " FNR " " pair
                rest = substr(rest, RSTART + RLENGTH)
            }
        }' "$1"
}

case "${1:-}" in
    convert)
        shift
        [ $# -eq 5 ] || { echo "usage: $0 convert <events> <labels> <table> <out> <report>" >&2; exit 1; }
        convert "$@"
        ;;
    first-divergence)
        shift
        [ $# -eq 2 ] || { echo "usage: $0 first-divergence <labels> <labels>" >&2; exit 1; }
        first_divergence "$@"
        ;;
    stale-pairs)
        shift
        [ $# -eq 3 ] || { echo "usage: $0 stale-pairs <txt> <events 128> <events converted>" >&2; exit 1; }
        stale_pairs "$@"
        ;;
    *)
        sed -n '2,4s/^# //p' "$0" >&2
        exit 1
        ;;
esac
