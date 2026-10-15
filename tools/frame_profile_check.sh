#!/bin/sh
# usage: frame_profile_check.sh breakdown <frames.txt> <breakdown.tty.txt> <map.txt>
#        frame_profile_check.sh coverage <threshold> <full frames.txt>[,...] <name>=<frames.txt>[,...]:<map.txt>...
#        frame_profile_check.sh subtree <name>=<frames.txt>[,...]:<map.txt>...
#        frame_profile_check.sh unit-cost <frames.txt> <breakdown.tty.txt> <map.txt>
#        frame_profile_check.sh unit-cost-pooled <unit-cost output>...
# breakdown: over the window of <map.txt> (tmp/short_run_cut), the per-family
# calls of tmp/frame_profile's <frames.txt> must equal the psx-draw: lines of
# the run's breakdown boot. Prints "calls-match ok <n> frames" then
# "family <f> pc <c> psx <c>" window totals, or "calls-match differs frame
# <i> ..." for the first mismatch or missing line. Exit 0 match, 2 mismatch,
# 1 usage error.
# coverage: per cart_draw child and per family but pattern (its time is
# inside its shapes), the share of cart_draw's PC time on each full-run frame
# (the minimum over the full profiles, which must have the same frames) and
# over each window's frames (window run frames from its map; the median over
# that window's profiles). Prints one
# line per item: frames above <threshold> %, mean and max share, each
# window's share, then the verdict: covered (above on at least 15 frames and
# in a window), uncovered (above on 15 frames, in no window) with the
# 15-frame span of highest mean share, sporadic (1 to 14 frames, listed) or
# below. Then the fewest "window draw-<a> resume <a> <b>" spans that cover
# every uncovered item, and "coverage ok" or "coverage uncovered ...". Exit 0
# when nothing is uncovered, 3 when something is, 2 on malformed input, 1 on
# a usage error.
# subtree: profiles of one draw root (their "root draw <symbol>" line), per
# run over its window: "root <symbol> run <name> window-frames <n>
# root-ns-per-frame <ns>" (median over the run's profiles of the mean), then
# per child called in the window, by decreasing share, "child <symbol>
# <child> calls <min>-<max> share <pct>" (calls per window frame in the
# first profile, median share of the root's PC time), then "self <symbol>
# share <pct>". Exit 0, 2 on malformed input, 1 on a usage error.
# unit-cost: over the window, per family "family <f> calls <c> units <u>
# switches <s> hblanks <h> per-call <x> per-unit <y>" (PC units, PSX time
# from the breakdown boot's fine ticks and calibration; "-" for a zero
# divisor), then "in-calls <h>" and "window-frames <n>"; a calls mismatch
# prints the first one as breakdown does. unit-cost-pooled sums such outputs.
# Exit 0, 2 on a mismatch or malformed input, 1 on a usage error.

set -u
LC_ALL=C
export LC_ALL

usage="usage: frame_profile_check.sh breakdown <frames.txt> <breakdown.tty.txt> <map.txt>
       frame_profile_check.sh coverage <threshold> <full frames.txt>[,...] <name>=<frames.txt>[,...]:<map.txt>...
       frame_profile_check.sh subtree <name>=<frames.txt>[,...]:<map.txt>...
       frame_profile_check.sh unit-cost <frames.txt> <breakdown.tty.txt> <map.txt>
       frame_profile_check.sh unit-cost-pooled <unit-cost output>..."

fail_usage() {
    echo "frame-profile-check: $1" >&2
    exit 1
}

# usage: window_of <map>; prints "<run-first> <run-last>", or fails the script on a malformed map.
window_of() {
    [ -f "$1" ] || fail_usage "missing map: $1"
    set -- $(head -n 1 "$1") "$1"
    if [ $# -ne 10 ] || [ "$1" != cut ] || [ "$3" != first-game-frame ] || [ "$5" != source-first ] || [ "$7" != window ]; then
        fail_usage "malformed map: ${10}"
    fi
    for value in "$4" "$6" "$8" "$9"; do
        case $value in
            '' | *[!0-9]*) fail_usage "malformed map: ${10}" ;;
        esac
    done
    echo "$(($4 + $8 - $6)) $(($4 + $9 - $6))"
}

breakdown() {
    [ $# -eq 3 ] || fail_usage "$usage"
    [ -f "$1" ] || fail_usage "missing profile: $1"
    [ -f "$2" ] || fail_usage "missing breakdown tty: $2"
    window=$(window_of "$3") || exit 1
    set -- "$1" "$2" $window
    {
        sed -n 's/^frame \([0-9]*\) family /P \1 /p' "$1"
        tr -d '\r' < "$2" | sed -n 's/^psx-draw: frame /D /p'
    } | awk -v first="$3" -v last="$4" '
        BEGIN { family_count = split("cls pset line rect rectfill circ circfill spr print pattern", families, " ") }
        $1 == "P" && (NF == 5 || NF == 9 && $6 == "units" && $8 == "switches") { pc[$2, $3] = $4; next }
        $1 == "D" && NF == 34 && $33 == "wrapped" {
            for (i = 1; i <= family_count; i++) if ($(3 * i) != families[i]) next
            seen[$2] = 1
            for (i = 1; i <= family_count; i++) psx[$2, families[i]] = $(3 * i + 2)
        }
        END {
            for (r = first; r <= last; r++) {
                if (!(r in seen)) { print "calls-match differs frame " r " no psx-draw line"; exit 2 }
                for (i = 1; i <= family_count; i++) {
                    f = families[i]
                    if (!((r, f) in pc)) { print "calls-match differs frame " r " no pc family " f " line"; exit 2 }
                    if (pc[r, f] != psx[r, f]) {
                        print "calls-match differs frame " r " family " f " pc " pc[r, f] " psx " psx[r, f]
                        exit 2
                    }
                    pc_total[f] += pc[r, f]; psx_total[f] += psx[r, f]
                }
            }
            print "calls-match ok " (last - first + 1) " frames"
            for (i = 1; i <= family_count; i++) print "family " families[i] " pc " pc_total[families[i]] " psx " psx_total[families[i]]
        }'
}

coverage() {
    [ $# -ge 3 ] || fail_usage "$usage"
    threshold=$1
    fulls=$(echo "$2" | tr ',' ' ')
    shift 2
    case $threshold in
        '' | *[!0-9.]* | *.*.* | . ) fail_usage "threshold is not a percentage: '$threshold'" ;;
    esac
    [ -n "$fulls" ] || fail_usage "$usage"
    full_count=0
    for full in $fulls; do
        [ -f "$full" ] || fail_usage "missing profile: $full"
        full_count=$((full_count + 1))
    done
    names=
    firsts=
    lasts=
    files=
    owners=
    window=0
    for spec in "$@"; do
        name=${spec%%=*}
        files_part=${spec#*=}
        profiles=${files_part%%:*}
        map=${files_part#*:}
        if [ "$name" = "$spec" ] || [ -z "$name" ] || [ "$profiles" = "$files_part" ] || [ -z "$profiles" ] || [ -z "$map" ]; then
            fail_usage "window is not <name>=<frames.txt>[,<frames.txt>...]:<map.txt>: $spec"
        fi
        window=$((window + 1))
        for profile in $(echo "$profiles" | tr ',' ' '); do
            [ -f "$profile" ] || fail_usage "missing profile: $profile"
            files="$files $profile"
            owners="$owners $window"
        done
        bounds=$(window_of "$map") || exit 1
        set -- $bounds
        names="$names $name"
        firsts="$firsts $1"
        lasts="$lasts $2"
    done
    awk -v threshold="$threshold" -v names="$names" -v firsts="$firsts" -v lasts="$lasts" -v owners="$owners" \
        -v full_count="$full_count" -v span=15 '
        BEGIN {
            window_count = split(names, window_names, " ")
            split(firsts, window_firsts, " ")
            split(lasts, window_lasts, " ")
            split(owners, file_window, " ")
            family_count = split("cls pset line rect rectfill circ circfill spr print", families, " ")
            file = 0
        }
        function malformed(reason) {
            print "frame-profile-check: malformed " FILENAME ": " reason > "/dev/stderr"
            failed = 1
            exit 2
        }
        function item_of(name) {
            if (!(name in item_index)) { item_count++; item_index[name] = item_count; items[item_count] = name }
            return item_index[name]
        }
        FNR == 1 {
            file++
            if ($1 != "profile" || $3 != "frames" || $4 !~ /^[0-9]+$/) malformed("no profile header")
            if (file == 1) frames = $4
            else if (file <= full_count && $4 != frames) malformed("frames " $4 ", not " frames " as the first full profile")
            if (file > full_count) w = file_window[file - full_count]
            next
        }
        $1 != "frame" { next }
        file <= full_count {
            if ($3 == "update" && NF == 6) draw[file, $2] = $6
            else if ($3 == "child" && $4 == "draw" && NF == 9) { item_of("child " $5); value[file, "child " $5, $2] = $7 }
            else if ($3 == "family" && (NF == 6 || NF == 10 && $7 == "units" && $9 == "switches")) value[file, "family " $4, $2] = $6
            next
        }
        $2 >= window_firsts[w] && $2 <= window_lasts[w] {
            if ($3 == "update" && NF == 6) { window_draw[file] += $6; window_seen[file, $2] = 1 }
            else if ($3 == "child" && $4 == "draw" && NF == 9) window_value[file, "child " $5] += $7
            else if ($3 == "family" && (NF == 6 || NF == 10 && $7 == "units" && $9 == "switches")) window_value[file, "family " $4] += $6
        }
        function pct(x) { return sprintf("%.1f%%", x) }
        # Timer noise only adds time: a frame counts above the threshold only if every profile says so.
        function share(it, f,    k, s, low) {
            for (k = 1; k <= full_count; k++) {
                s = draw[k, f] > 0 ? value[k, it, f] * 100 / draw[k, f] : 0
                if (k == 1 || s < low) low = s
            }
            return low
        }
        function window_share(w, it,    n, v, x, i, j, t, file_index) {
            n = 0
            for (file_index = 1; file_index <= window_file_count; file_index++) {
                if (file_window[file_index] != w) continue
                x = full_count + file_index
                v[++n] = window_draw[x] > 0 ? window_value[x, it] * 100 / window_draw[x] : 0
            }
            for (i = 1; i <= n; i++) for (j = i + 1; j <= n; j++) if (v[j] < v[i]) { t = v[i]; v[i] = v[j]; v[j] = t }
            return n % 2 ? v[(n + 1) / 2] : (v[n / 2] + v[n / 2 + 1]) / 2
        }
        function span_mean(it, a,    f, sum) {
            for (f = a; f < a + span && f < frames; f++) sum += min_share[it, f]
            return sum / span
        }
        END {
            if (failed) exit 2
            if (frames == 0) { FILENAME = "full profile"; malformed("no frame") }
            for (k = 1; k <= full_count; k++)
                for (f = 0; f < frames; f++) if (!((k, f) in draw)) { FILENAME = "full profile " k; malformed("no draw line for frame " f) }
            window_file_count = split(owners, file_window, " ")
            for (file_index = 1; file_index <= window_file_count; file_index++) {
                x = full_count + file_index; w = file_window[file_index]
                for (f = window_firsts[w]; f <= window_lasts[w]; f++)
                    if (!((x, f) in window_seen)) { FILENAME = window_names[w]; malformed("no draw line for window frame " f) }
            }
            for (i = 1; i <= family_count; i++) item_of("family " families[i])
            for (it = 1; it <= item_count; it++) for (f = 0; f < frames; f++) min_share[it, f] = share(items[it], f)

            print "coverage threshold " threshold "% significant " span " frames, shares of cart_draw PC time (not PSX cost)"
            print "coverage profiles " full_count " full, frame share = minimum over the full profiles, window share = median over its profiles"
            for (w = 1; w <= window_count; w++) print "coverage window " window_names[w] " run frames " window_firsts[w] ".." window_lasts[w]
            uncovered_count = 0
            for (it = 1; it <= item_count; it++) {
                name = items[it]; above = 0; sum = 0; max = 0; listed = ""
                for (f = 0; f < frames; f++) {
                    s = min_share[it, f]; sum += s
                    if (s > max) max = s
                    if (s > threshold) { above++; listed = listed " " f }
                }
                line = name " above " above " mean " pct(sum / frames) " max " pct(max)
                in_window = 0
                for (w = 1; w <= window_count; w++) {
                    s = window_share(w, name)
                    if (s > threshold) in_window = 1
                    line = line " " window_names[w] " " pct(s)
                }
                if (above >= span && in_window) line = line " covered"
                else if (above >= span) {
                    best = 0; best_mean = -1
                    last_start = frames > span ? frames - span : 0
                    for (a = 0; a <= last_start; a++) {
                        m = span_mean(it, a)
                        if (m > best_mean) { best_mean = m; best = a }
                    }
                    uncovered[++uncovered_count] = it
                    proposed[uncovered_count] = best
                    line = line " uncovered propose resume " best " " (best + span - 1)
                } else if (above > 0) line = line " sporadic frames" listed
                else line = line " below"
                print line
            }
            remaining = uncovered_count
            for (u = 1; u <= uncovered_count; u++) open_item[u] = 1
            window_lines = 0
            while (remaining > 0) {
                best_count = 0; chosen = -1
                for (c = 1; c <= uncovered_count; c++) {
                    count = 0
                    for (u = 1; u <= uncovered_count; u++)
                        if (open_item[u] && span_mean(uncovered[u], proposed[c]) > threshold) count++
                    if (count > best_count || (count == best_count && count > 0 && proposed[c] < chosen)) {
                        best_count = count; chosen = proposed[c]
                    }
                }
                if (best_count == 0) {
                    for (u = 1; !open_item[u]; u++) ;
                    chosen = proposed[u]; open_item[u] = 0; remaining--
                }
                for (u = 1; u <= uncovered_count; u++)
                    if (open_item[u] && span_mean(uncovered[u], chosen) > threshold) { open_item[u] = 0; remaining-- }
                print "window draw-" chosen " resume " chosen " " (chosen + span - 1)
                window_lines++
            }
            if (uncovered_count == 0) print "coverage ok"
            else { print "coverage uncovered " uncovered_count " items, " window_lines " windows"; exit 3 }
        }' $fulls $files
}

subtree() {
    [ $# -ge 1 ] || fail_usage "$usage"
    for spec in "$@"; do
        name=${spec%%=*}
        files_part=${spec#*=}
        profiles=${files_part%%:*}
        map=${files_part#*:}
        if [ "$name" = "$spec" ] || [ -z "$name" ] || [ "$profiles" = "$files_part" ] || [ -z "$profiles" ] || [ -z "$map" ]; then
            fail_usage "run is not <name>=<frames.txt>[,<frames.txt>...]:<map.txt>: $spec"
        fi
        for profile in $(echo "$profiles" | tr ',' ' '); do
            [ -f "$profile" ] || fail_usage "missing profile: $profile"
        done
        window_of "$map" > /dev/null || exit 1
    done
    for spec in "$@"; do
        name=${spec%%=*}
        files_part=${spec#*=}
        profiles=$(echo "${files_part%%:*}" | tr ',' ' ')
        set -- $(window_of "${files_part#*:}")
        awk -v run="$name" -v first="$1" -v last="$2" '
            function malformed(reason) {
                print "frame-profile-check: malformed " FILENAME ": " reason > "/dev/stderr"
                failed = 1
                exit 2
            }
            function median(values, n,    i, j, t) {
                for (i = 1; i <= n; i++) for (j = i + 1; j <= n; j++) if (values[j] < values[i]) { t = values[i]; values[i] = values[j]; values[j] = t }
                return n % 2 ? values[(n + 1) / 2] : (values[n / 2] + values[n / 2 + 1]) / 2
            }
            FNR == 1 {
                if (file > 0 && !(file in root)) { FILENAME = previous; malformed("no root draw line") }
                file++; previous = FILENAME
                if ($1 != "profile" || $3 != "frames" || $4 !~ /^[0-9]+$/) malformed("no profile header")
                next
            }
            FNR == 2 && $1 == "root" && $2 == "draw" && NF == 3 {
                root[file] = $3
                if (file > 1 && $3 != root[1]) malformed("root draw " $3 ", not " root[1] " as the first profile")
                next
            }
            $1 != "frame" || $2 < first || $2 > last { next }
            $3 == "update" && NF == 6 { root_ns[file] += $6; seen[file, $2] = 1; next }
            $3 == "self" && $4 == "draw" && NF == 5 { self_ns[file] += $5; next }
            $3 == "child" && $4 == "draw" && NF == 9 {
                if (!($5 in child_index)) { child_count++; child_index[$5] = child_count; children[child_count] = $5 }
                child_ns[file, $5] += $7
                if (file == 1) {
                    called[$5] += $6
                    if (!(($5) in low) || $6 < low[$5]) low[$5] = $6
                    if ($6 > high[$5]) high[$5] = $6
                }
            }
            END {
                if (failed) exit 2
                if (!(file in root)) { FILENAME = previous; malformed("no root draw line") }
                for (k = 1; k <= file; k++)
                    for (f = first; f <= last; f++) if (!((k, f) in seen)) malformed("no draw line for window frame " f " in profile " k)
                frames = last - first + 1
                for (k = 1; k <= file; k++) v[k] = root_ns[k] / frames
                printf "root %s run %s window-frames %d root-ns-per-frame %d\n", root[1], run, frames, median(v, file) + 0.5
                listed = 0
                for (c = 1; c <= child_count; c++) {
                    name = children[c]
                    if (!called[name]) continue
                    for (k = 1; k <= file; k++) v[k] = root_ns[k] > 0 ? child_ns[k, name] * 100 / root_ns[k] : 0
                    listed++; order[listed] = name; share[name] = median(v, file)
                }
                for (i = 2; i <= listed; i++)
                    for (j = i; j > 1 && share[order[j]] > share[order[j - 1]]; j--) { t = order[j]; order[j] = order[j - 1]; order[j - 1] = t }
                for (i = 1; i <= listed; i++)
                    printf "child %s %s calls %d-%d share %.1f%%\n", root[1], order[i], low[order[i]], high[order[i]], share[order[i]]
                for (k = 1; k <= file; k++) v[k] = root_ns[k] > 0 ? self_ns[k] * 100 / root_ns[k] : 0
                printf "self %s share %.1f%%\n", root[1], median(v, file)
            }' $profiles || exit $?
    done
}

unit_cost() {
    [ $# -eq 3 ] || fail_usage "$usage"
    [ -f "$1" ] || fail_usage "missing profile: $1"
    [ -f "$2" ] || fail_usage "missing breakdown tty: $2"
    window=$(window_of "$3") || exit 1
    set -- "$1" "$2" $window
    {
        sed -n 's/^frame \([0-9]*\) family /P \1 /p' "$1"
        tr -d '\r' < "$2" | sed -n 's/^psx-draw: frame /D /p; s/^psx-draw: clk8-per-64-hblanks /C /p'
    } | awk -v first="$3" -v last="$4" '
        BEGIN { family_count = split("cls pset line rect rectfill circ circfill spr print pattern", families, " ") }
        $1 == "C" && NF == 2 { if (calibration == "") calibration = $2; next }
        $1 == "P" && NF == 9 && $6 == "units" && $8 == "switches" {
            pc[$2, $3] = $4; units[$2, $3] = $7; switches[$2, $3] = $9; next
        }
        $1 == "D" && NF == 34 && $33 == "wrapped" {
            for (i = 1; i <= family_count; i++) if ($(3 * i) != families[i]) next
            seen[$2] = 1
            for (i = 1; i <= family_count; i++) { ticks[$2, families[i]] = $(3 * i + 1); psx[$2, families[i]] = $(3 * i + 2) }
        }
        function ratio(part, whole) { return whole > 0 ? sprintf("%.3f", part / whole) : "-" }
        END {
            if (calibration == "" || calibration !~ /^[0-9]+$/ || calibration == 0) {
                print "frame-profile-check: no clk8-per-64-hblanks line in the breakdown tty" > "/dev/stderr"
                exit 2
            }
            for (r = first; r <= last; r++) {
                if (!(r in seen)) { print "calls-match differs frame " r " no psx-draw line"; exit 2 }
                for (i = 1; i <= family_count; i++) {
                    f = families[i]
                    if (!((r, f) in pc)) { print "calls-match differs frame " r " no pc family " f " line with units"; exit 2 }
                    if (pc[r, f] != psx[r, f]) {
                        print "calls-match differs frame " r " family " f " pc " pc[r, f] " psx " psx[r, f]
                        exit 2
                    }
                    calls_total[f] += pc[r, f]; units_total[f] += units[r, f]; switches_total[f] += switches[r, f]
                    ticks_total[f] += ticks[r, f]
                    if (f != "pattern") in_call_ticks += ticks[r, f]
                }
            }
            for (i = 1; i <= family_count; i++) {
                f = families[i]; h = ticks_total[f] * 64 / calibration
                printf "family %s calls %d units %d switches %d hblanks %.1f per-call %s per-unit %s\n", f, calls_total[f], \
                    units_total[f], switches_total[f], h, ratio(h, calls_total[f]), ratio(h, units_total[f])
            }
            printf "in-calls %.1f\n", in_call_ticks * 64 / calibration
            print "window-frames " (last - first + 1)
        }'
}

unit_cost_pooled() {
    [ $# -ge 1 ] || fail_usage "$usage"
    for output in "$@"; do
        [ -f "$output" ] || fail_usage "missing unit-cost output: $output"
    done
    awk '
        BEGIN { family_count = split("cls pset line rect rectfill circ circfill spr print pattern", families, " ") }
        function malformed(reason) {
            print "frame-profile-check: malformed " FILENAME ": " reason > "/dev/stderr"
            failed = 1
            exit 2
        }
        FNR == 1 {
            if (previous != "" && line_index != family_count + 2) { FILENAME = previous; malformed("truncated") }
            previous = FILENAME; line_index = 0
        }
        {
            line_index++
            if (line_index <= family_count) {
                f = families[line_index]
                if (NF != 14 || $1 != "family" || $2 != f || $3 != "calls" || $5 != "units" || $7 != "switches" || $9 != "hblanks")
                    malformed("line " FNR " is not family " f)
                calls[f] += $4; units[f] += $6; switches[f] += $8; hblanks[f] += $10
            } else if (line_index == family_count + 1 && $1 == "in-calls" && NF == 2) in_calls += $2
            else if (line_index == family_count + 2 && $1 == "window-frames" && NF == 2) frames += $2
            else malformed("line " FNR " is not a unit-cost line")
        }
        function ratio(part, whole) { return whole > 0 ? sprintf("%.3f", part / whole) : "-" }
        END {
            if (failed) exit 2
            if (line_index != family_count + 2) { FILENAME = previous; malformed("truncated") }
            for (i = 1; i <= family_count; i++) {
                f = families[i]
                printf "family %s calls %d units %d switches %d hblanks %.1f per-call %s per-unit %s\n", f, calls[f], \
                    units[f], switches[f], hblanks[f], ratio(hblanks[f], calls[f]), ratio(hblanks[f], units[f])
            }
            printf "in-calls %.1f\n", in_calls
            print "window-frames " frames
        }' "$@"
}

mode=${1:-}
[ $# -gt 0 ] && shift
case $mode in
    breakdown) breakdown "$@" ;;
    coverage) coverage "$@" ;;
    subtree) subtree "$@" ;;
    unit-cost) unit_cost "$@" ;;
    unit-cost-pooled) unit_cost_pooled "$@" ;;
    *) fail_usage "$usage" ;;
esac
