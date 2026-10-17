#!/bin/sh
# usage: psx_capture_report.sh <compare-tool> <list> <pc-dir> <run-prefix> <report-dir>
# Runs the comparator (tmp/psx_capture_compare) on each capture of the list:
# <run-prefix>.f<frame>.vram against frame <n> of <pc-dir>/pc_frames.bin, with
# its mask from <pc-dir>/capture_script.txt and its calls from
# <pc-dir>/counts.txt (the label's family; the sum of every family but pattern
# for "all" and "display"). Writes <report-dir>/report.txt (one line a capture, list order)
# and the comparator's diff map and image. A capture the comparator refuses
# gets no line; its error goes to <report-dir>/errors.txt. Exit 0.

set -u

[ $# -eq 5 ] || { echo "usage: psx_capture_report.sh <compare-tool> <list> <pc-dir> <run-prefix> <report-dir>" >&2; exit 1; }
tool=$1
list=$2
pc_dir=$3
prefix=$4
report_dir=$5

rm -rf "$report_dir"
mkdir -p "$report_dir"
: > "$report_dir/report.txt"
: > "$report_dir/errors.txt"

index=0
awk 'NF && $1 !~ /^#/ && $1 != "replay" { print $1, $2 }' "$list" | while read -r label frame; do
    mask=$(awk -v frame="$frame" '$1 == frame { print $2 }' "$pc_dir/capture_script.txt" 2>/dev/null)
    calls=$(awk -v frame="$frame" -v label="$label" '
        $1 == frame && (label == "all" || label == "display" ? $2 != "pattern" : $2 == label) { sum += $3; found = 1 }
        END { if (found) print sum }' "$pc_dir/counts.txt" 2>/dev/null)
    if "$tool" "$prefix.f$frame.vram" "$pc_dir/pc_frames.bin" "$index" "$label" "$frame" "${mask:-x}" "${calls:-x}" \
        "$report_dir" >> "$report_dir/report.txt" 2>> "$report_dir/errors.txt"; then
        :
    fi
    index=$((index + 1))
done
exit 0
