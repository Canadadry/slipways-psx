#!/bin/sh
# usage: psx_capture_record.sh ceilings|references <run-dir> <label> <file>
# Rewrites <label>'s lines of <file> from the last make psx-check run in
# <run-dir> and leaves every other line as it was, sorted by frame:
#   ceilings   "<label> <frame> <differ>" from gpu-captures/report.txt; refused
#              unless every condition but gpu-capture-ceilings passed;
#   references "<label> <frame> <sha256 of gpu.f<frame>.vram>"; refused unless
#              every condition passed. The human's command: no agent runs it.
# Refused too when the run is missing or has no capture of <label>.

set -u

fail() {
    echo "psx-capture-record: $*" >&2
    exit 1
}

[ $# -eq 4 ] || fail "usage: psx_capture_record.sh ceilings|references <run-dir> <label> <file>"
kind=$1
run_dir=$2
label=$3
file=$4
case $kind in
    ceilings) allowed=gpu-capture-ceilings ;;
    references) allowed= ;;
    *) fail "unknown kind $kind (ceilings or references)" ;;
esac
[ -n "$label" ] || fail "no label"

conditions=$run_dir/conditions.txt
report=$run_dir/gpu-captures/report.txt
[ -f "$conditions" ] && [ -f "$report" ] || fail "no make psx-check run in $run_dir"
failed=$(awk -v allowed="$allowed" '$2 != "ok" && $1 != allowed { printf "%s ", $1 }' "$conditions")
[ -z "$failed" ] || fail "the run in $run_dir failed: $failed"

lines=$(awk -v label="$label" '$1 == label { sub(/^f/, "", $2); print $2 }' "$report" | while read -r frame; do
    if [ "$kind" = ceilings ]; then
        value=$(awk -v head="$label f$frame " 'index($0, head) == 1 {
            for (i = 1; i < NF; i++) if ($i == "differ") { print $(i + 1); exit } }' "$report")
    else
        capture=$run_dir/gpu.f$frame.vram
        [ -f "$capture" ] || { echo "missing $capture" >&2; exit 1; }
        if command -v sha256sum > /dev/null 2>&1; then
            value=$(sha256sum "$capture" | cut -d ' ' -f 1)
        else
            value=$(shasum -a 256 "$capture" | cut -d ' ' -f 1)
        fi
    fi
    echo "$label $frame $value"
done) || fail "a capture of $label is missing in $run_dir"
[ -n "$lines" ] || fail "no capture of $label in $report"

kept=$(awk -v label="$label" 'NF && $1 != label' "$file" 2>/dev/null)
printf '%s\n%s\n' "$kept" "$lines" | awk 'NF' | sort -k 2,2n -k 1,1 > "$file.tmp" && mv "$file.tmp" "$file"
echo "psx-capture-record: $kind of $label written to $file:"
printf '%s\n' "$lines"
