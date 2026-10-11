#!/bin/sh
# usage: psx_capture_check.sh <list> <pc-dir> <gpu-tty> <run-prefix> <repeat-prefix> <report-dir>
#                             <ceilings> <references> <conditions>
# The seven GPU capture conditions of make psx-check, from the files of one
# run: gpu-capture-list, gpu-captures, gpu-capture-deterministic,
# gpu-capture-clean, gpu-capture-report, gpu-capture-ceilings,
# gpu-capture-references. One line per condition (ok on stdout, failure on
# stderr), and "<condition> ok|failed" appended to <conditions>. Exit 1 if
# any failed.

set -u

usage="usage: psx_capture_check.sh <list> <pc-dir> <gpu-tty> <run-prefix> <repeat-prefix> <report-dir> <ceilings> <references> <conditions>"
[ $# -eq 9 ] || { echo "psx-check: $usage" >&2; exit 1; }
list=$1
pc_dir=$2
gpu_tty=$3
prefix=$4
repeat_prefix=$5
report_dir=$6
ceilings=$7
references=$8
conditions=$9
report=$report_dir/report.txt

failed=0
pass() {
    echo "psx-check: $1: ok${2:+, $2}"
    echo "$1 ok" >> "$conditions"
}
fail() {
    echo "psx-check: $1: $2" >&2
    echo "$1 failed" >> "$conditions"
    failed=1
}

# sha256 <file>... -> "<hash>  <file>" a file
sha256() {
    if command -v sha256sum > /dev/null 2>&1; then sha256sum "$@"; else shasum -a 256 "$@"; fi
}

# "<label> <frame>" a capture, list order.
captures=$(awk 'NF && $1 !~ /^#/ && $1 != "replay" { print $1, $2 }' "$list" 2>/dev/null)
frames=$(printf '%s\n' "$captures" | awk 'NF { print $2 }')
list_replay=$(awk 'NF && $1 !~ /^#/ { if ($1 == "replay") print $2; exit }' "$list" 2>/dev/null)
tty_lines=$(tr -d '\r' < "$gpu_tty" 2>/dev/null)
exe_replay=$(printf '%s\n' "$tty_lines" | sed -n 's/^psx-replay: done \([^ ]*\) .*/\1/p' | tail -n 1)

# gpu-capture-list
problem=
if [ ! -f "$list" ]; then
    problem="missing list $list"
elif [ -z "$list_replay" ]; then
    problem="$list has no replay line"
else
    [ "$list_replay" = "$exe_replay" ] \
        || problem="$list is for replay $list_replay, the PS-EXE replays ${exe_replay:-(no done line)}; "
    [ "$(awk '{ print $1 }' "$pc_dir/capture_script.txt" 2>/dev/null)" = "$frames" ] \
        || problem="$problem$pc_dir/capture_script.txt does not hold the list's frames (did the PC run parse it?); "
    problem=$problem$(printf '%s\n' "$captures" | awk -v counts="$pc_dir/counts.txt" '
        BEGIN { while ((getline line < counts) > 0) { split(line, w, " "); calls[w[1] " " w[2]] = w[3] } }
        NF == 2 && $1 != "all" && calls[$2 " " $1] + 0 < 1 { printf "%s f%s draws no %s call on PC; ", $1, $2, $1 }')
fi
if [ -z "$problem" ]; then pass gpu-capture-list; else fail gpu-capture-list "$problem"; fi

# gpu-captures
problem=
for frame in $frames; do
    file=$prefix.f$frame.vram
    if [ ! -f "$file" ]; then
        problem="${problem}missing $file; "
    else
        read -r header < "$file" || header=
        size=$(wc -c < "$file" | tr -d " ")
        [ "$header" = "psx-vram 320 240 16" ] && [ "$size" -eq 153620 ] \
            || problem="${problem}malformed $file ('$header', $size bytes); "
    fi
done
tty_frames=$(printf '%s\n' "$tty_lines" | sed -n 's/^psx-replay: capture \([0-9]*\) .*/\1/p')
if [ "$tty_frames" != "$frames" ]; then
    problem="${problem}TTY capture frames '$(echo $tty_frames)', expected '$(echo $frames)'"
fi
if [ -z "$problem" ]; then pass gpu-captures "$(printf '%s\n' "$frames" | grep -c .) capture(s)"; else fail gpu-captures "$problem"; fi

# gpu-capture-deterministic
problem=
for frame in $frames; do
    if ! cmp -s "$prefix.f$frame.vram" "$repeat_prefix.f$frame.vram"; then
        problem="the second boot's capture of frame $frame differs ($repeat_prefix.f$frame.vram)"
        break
    fi
done
if [ -z "$problem" ]; then pass gpu-capture-deterministic; else fail gpu-capture-deterministic "$problem"; fi

# One awk pass joins the list with the report, the ceilings, the references
# and the captures' hashes (only hashed when a reference exists):
#   capture <label> <frame> <differ> <offpalette> <nonuniform> <ceiling> <reference> <hash>
#   stale-ceiling <label> <frame>
#   stale-reference <label> <frame>
# "-" where a value is missing.
hashes=
if [ -s "$references" ]; then
    files=
    for frame in $frames; do
        [ ! -f "$prefix.f$frame.vram" ] || files="$files $prefix.f$frame.vram"
    done
    [ -z "$files" ] || hashes=$(sha256 $files)
fi
table=$(printf '%s\n' "$hashes" | captures=$captures awk -v report="$report" -v ceilings="$ceilings" \
    -v references="$references" -v prefix="$prefix" '
    function value(map, key) { return (key in map) ? map[key] : "-" }
    BEGIN {
        while ((getline line < report) > 0) {
            n = split(line, w, " ")
            key = w[1] " " substr(w[2], 2)
            if (key in reported) continue
            reported[key] = 1
            for (i = 3; i < n; i++) {
                if (w[i] == "differ") differ[key] = w[i + 1]
                if (w[i] == "offpalette") offpalette[key] = w[i + 1]
                if (w[i] == "nonuniform") nonuniform[key] = w[i + 1]
            }
        }
        while ((getline line < ceilings) > 0) {
            if (split(line, w, " ") >= 3 && !((w[1] " " w[2]) in ceiling)) ceiling[w[1] " " w[2]] = w[3]
        }
        while ((getline line < references) > 0) {
            if (split(line, w, " ") >= 3 && !((w[1] " " w[2]) in reference)) reference[w[1] " " w[2]] = w[3]
        }
    }
    NF == 2 { hash[$2] = $1 }
    END {
        count = split(ENVIRON["captures"], lines, "\n")
        for (c = 1; c <= count; c++) {
            if (split(lines[c], w, " ") < 2) continue
            key = w[1] " " w[2]
            listed[key] = 1
            print "capture", key, value(differ, key), value(offpalette, key), value(nonuniform, key),
                value(ceiling, key), value(reference, key), value(hash, prefix ".f" w[2] ".vram")
        }
        for (key in ceiling) if (!(key in listed)) print "stale-ceiling", key
        for (key in reference) if (!(key in listed)) print "stale-reference", key
    }')

# gpu-capture-clean and gpu-capture-report
clean=
reported=
while read -r kind label frame differ offpalette nonuniform ceiling reference hash; do
    [ "$kind" = capture ] || continue
    if [ "$differ" = - ]; then
        clean="${clean}$label f$frame has no report line; "
        reported="${reported}no report line for $label f$frame; "
        continue
    fi
    if [ "$nonuniform" != 0 ] || [ "$offpalette" != 0 ]; then
        clean="${clean}$label f$frame: nonuniform $nonuniform offpalette $offpalette; "
    fi
    for file in "$report_dir/$label-f$frame.diff.txt" "$report_dir/$label-f$frame.ppm"; do
        [ -f "$file" ] || reported="${reported}missing $file; "
    done
done <<END
$table
END
if [ -z "$clean" ]; then pass gpu-capture-clean; else fail gpu-capture-clean "$clean"; fi
if [ -z "$reported" ]; then pass gpu-capture-report "$report"; else fail gpu-capture-report "$reported"; fi

# gpu-capture-ceilings and gpu-capture-references
ceiling_problem=
reference_problem=
notes=
pending=
stale_ceilings=
stale_references=
while read -r kind label frame differ offpalette nonuniform ceiling reference hash; do
    case $kind in
        stale-ceiling) stale_ceilings="$stale_ceilings$label f$frame; "; continue ;;
        stale-reference) stale_references="$stale_references$label f$frame; "; continue ;;
        capture) ;;
        *) continue ;;
    esac
    if [ "$reference" = - ]; then
        pending="${pending}psx-check: pending human acceptance: $label f$frame
"
    elif [ "$hash" = "$reference" ]; then
        continue
    else
        reference_problem="${reference_problem}$label f$frame differs from its reference $reference; "
    fi
    if [ "$ceiling" = - ]; then
        ceiling_problem="${ceiling_problem}no ceiling for $label f$frame (differ $differ); "
    elif [ "$differ" = - ]; then
        ceiling_problem="${ceiling_problem}no differ count for $label f$frame; "
    elif [ "$differ" -gt "$ceiling" ]; then
        ceiling_problem="${ceiling_problem}$label f$frame differ $differ above its ceiling $ceiling; "
    elif [ "$differ" -lt "$ceiling" ]; then
        notes="${notes}psx-check: below ceiling: $label f$frame $ceiling -> $differ
"
    fi
done <<END
$table
END
[ -z "$stale_ceilings" ] || ceiling_problem="${ceiling_problem}ceiling for a capture no longer listed: $stale_ceilings"
[ -z "$stale_references" ] || reference_problem="${reference_problem}reference for a capture no longer listed: $stale_references"
printf '%s' "$notes"
if [ -z "$ceiling_problem" ]; then pass gpu-capture-ceilings; else fail gpu-capture-ceilings "$ceiling_problem"; fi
printf '%s' "$pending"
if [ -z "$reference_problem" ]; then pass gpu-capture-references; else fail gpu-capture-references "$reference_problem"; fi

[ "$failed" -eq 0 ]
