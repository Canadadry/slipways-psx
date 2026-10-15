#!/bin/sh
# usage: lever_estimate_check.sh <judge-frames.txt> <spike-frames.txt> <fixtures dir> <output dir>
# Known-answer check of tools/lever_estimate.sh against step 9's "Spreading"
# results (docs/psx-perf-levers.md), independent allocation and step 9's
# spike rule (spike-rule=step9: placing the work is enough), on the
# fixtures uniform-88, uniform-87 and step78-low: a uniform 88% draw cut
# leaves 0 of 166 spike frames does-not-fit; 87% leaves frame 6043 among
# them; 80.7% in game with 74.8% on frame 12 leaves exactly frame 6043 (C15)
# with every class's work and frames equal to step 9's `step7+8, gain
# 80.7%` column (C13 its PC row); and on each fixture the spike lines equal
# frame_profile_check.sh spread's token for token. Outputs in <output dir>.
# Exit 0 when every result is reproduced, 1 naming each one that is not, or
# on a usage error.

set -u
LC_ALL=C
export LC_ALL

[ $# -eq 4 ] || { echo "usage: lever_estimate_check.sh <judge-frames.txt> <spike-frames.txt> <fixtures dir> <output dir>" >&2; exit 1; }
judge=$1
spikes=$2
fixtures=$3
out=$4
for file in "$judge" "$spikes" "$fixtures/uniform-88.txt" "$fixtures/uniform-87.txt" "$fixtures/step78-low.txt"; do
    [ -f "$file" ] || { echo "lever-estimate-check: missing $file" >&2; exit 1; }
done
mkdir -p "$out" || exit 1
tools=$(dirname "$0")

failed=0
not_reproduced() {
    echo "lever-estimate-check: not reproduced: $1" >&2
    failed=1
}

# The summary line's value after <key>.
summary_value() {
    awk -v key="$2" '$1 == "summary" { for (i = 2; i < NF; i++) if ($i == key) print $(i + 1) }' "$1"
}

ratios=ui_update=0.96,save_at_frame_end=1.11
for pair in uniform-88:default=88 uniform-87:default=87 step78-low:default=80.7,12-12=74.8; do
    name=${pair%%:*}
    gains=${pair#*:}
    table=$fixtures/$name.txt
    if ! sh "$tools/lever_estimate.sh" "$judge" "$spikes" "$table" measured allocation=independent spike-rule=step9 > "$out/$name.txt"; then
        not_reproduced "$name: lever_estimate.sh failed"
        continue
    fi
    sh "$tools/frame_profile_check.sh" spread 263 "$judge" "$spikes" "$gains" "$ratios" > "$out/$name.spread.txt" \
        || { not_reproduced "$name: frame_profile_check.sh spread failed"; continue; }
    awk 'FNR == NR { sub(/(^|[ \t])#.*/, ""); if ($1 == "class") path[$2] = $3; next }
        $1 == "frame" && $5 == "class" {
            line = "frame " $2 " class " path[$6]
            for (i = 7; i <= NF; i++) if ($i == "spread") i++; else line = line " " $i
            print line
        }
        $1 == "class" { $2 = path[$2]; print }' "$table" "$out/$name.txt" > "$out/$name.as-spread.txt"
    cmp -s "$out/$name.as-spread.txt" "$out/$name.spread.txt" \
        || not_reproduced "$name: spike lines differ from spread $gains ($out/$name.as-spread.txt, $out/$name.spread.txt)"
    [ "$(summary_value "$out/$name.txt" spikes)" = 166 ] || not_reproduced "$name: not 166 spike frames"
done

[ "$(summary_value "$out/uniform-88.txt" does-not-fit)" = 0 ] || not_reproduced "uniform 88%: does-not-fit is not 0"

misses_87=$(summary_value "$out/uniform-87.txt" does-not-fit)
grep -q '^frame 6043 .* does-not-fit$' "$out/uniform-87.txt" && [ "${misses_87:-0}" -ge 1 ] \
    || not_reproduced "uniform 87%: frame 6043 is not among the does-not-fit"

[ "$(summary_value "$out/step78-low.txt" does-not-fit)" = 1 ] \
    && grep -q '^frame 6043 before [0-9]* class C15 .* does-not-fit$' "$out/step78-low.txt" \
    || not_reproduced "step7+8 at 80.7%: does-not-fit is not exactly frame 6043 (C15)"

sort > "$out/step78-low.expected.txt" <<'EOF'
class C1 work 474-722.5-1195 frames 10-24.5-67 does-not-fit 0
class C2 work 321-392-518 frames 6-18-108 does-not-fit 0
class C3 work 355-445-532 frames 6-11.5-55 does-not-fit 0
class C4 work 189-245-377 frames 11-30-33 does-not-fit 0
class C5 work 397-498.5-535 frames 5-8.5-22 does-not-fit 0
class C6 work 8980-8980-8980 frames 162-162-162 does-not-fit 0
class C7 work 291-316-351 frames 5-6-7 does-not-fit 0
class C8 work 349-417-522 frames 11-26.5-54 does-not-fit 0
class C9 work 415-537-795 frames 13-19.5-53 does-not-fit 0
class C10 work 613-642-660 frames 25-36-46 does-not-fit 0
class C11 work 434-475-508 frames 10-13-13 does-not-fit 0
class C12 work 273-343-418 frames 10-15-19 does-not-fit 0
class C13 work 623-639-744 frames 8-16-22 does-not-fit 0
class C14 work 376-414-452 frames 8-9.5-11 does-not-fit 0
class C15 work 405-446.5-488 frames 134-134-134 does-not-fit 1
class C16 work 347-360.5-374 frames 13-16.5-20 does-not-fit 0
class C17 work 631-631-631 frames 20-20-20 does-not-fit 0
class C18 work 424-424-424 frames 21-21-21 does-not-fit 0
class C19 work 194-194-194 frames 28-28-28 does-not-fit 0
EOF
grep '^class ' "$out/step78-low.txt" | sort > "$out/step78-low.classes.txt"
cmp -s "$out/step78-low.classes.txt" "$out/step78-low.expected.txt" \
    || not_reproduced "step7+8 at 80.7%: class figures differ from step 9's ($out/step78-low.classes.txt, $out/step78-low.expected.txt)"

[ $failed -eq 0 ] || exit 1
echo "lever-estimate-check ok: uniform 88% does-not-fit 0 of 166; uniform 87% does-not-fit $misses_87, frame 6043 among them;" \
    "80.7% in game, 74.8% on frame 12: does-not-fit 1 (frame 6043, C15), 19 classes equal step 9's;" \
    "spike lines equal spread's on the 3 tables"
