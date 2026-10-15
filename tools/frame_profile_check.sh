#!/bin/sh
# usage: frame_profile_check.sh breakdown <frames.txt> <breakdown.tty.txt> <map.txt>
#        frame_profile_check.sh coverage <threshold> <full frames.txt>[,...] <name>=<frames.txt>[,...]:<map.txt>...
#        frame_profile_check.sh subtree [part=draw|part=update] <name>=<frames.txt>[,...]:<map.txt>...
#        frame_profile_check.sh unit-cost <frames.txt> <breakdown.tty.txt> <map.txt>
#        frame_profile_check.sh unit-cost-pooled <unit-cost output>...
#        frame_profile_check.sh spikes <threshold> <judge-frames.txt> <base frames.txt>[,...] [<symbol>=<frames.txt>[,...]|unresolvable]...
#        frame_profile_check.sh spread <budget> <judge-frames.txt> <spikes.txt> <gains> [<ratios>]
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
# subtree: profiles of one draw root (their "root draw <symbol>" line), or
# with part=update of one update root (its "root update" line, the frame
# line's update time, the "child update" and "self update" lines), per
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
# spikes: the frames whose judge update exceeds <threshold> hblanks, each
# walked down from cart_update60 (base profiles) through the <symbol>= sets
# (profiles rooted at that update symbol; "unresolvable" closes the walk on
# it) into the child holding at least 50 % of cart_update60's PC time
# (per-frame minimums over each set's K profiles). Prints "frame" lines,
# "class" lines (frames sharing a closed path), "open <symbol> frames <n>"
# lines and "spike-frames <n> over-1000 <m> classes <c> open <o>" (o: open
# frames). Exit 0, 3 when a frame is open, 2 on malformed input, 1 on a
# usage error.
# spread: per frame of a closed class in a spikes output, "frame <f> class
# <path> work <W> frames <N|does-not-fit> [overlaps <f2>,...]": W is the
# share times the ratio of the path's cart_update60 child (<ratios>
# "<child>=<ratio>,...", 1 when missing), capped at 1, times the judge
# update; N the fewest frames from f whose room under <budget> sums to W
# (frame f keeping update - W, draw cut by <gains> "default=<pct>[,<first>-
# <last>=<pct>]...", the last range wins), does-not-fit past 600 frames or
# the run's end; overlaps lists the other spike frames in the N frames. Then
# "class <path> work <min>-<median>-<max> frames <min>-<median>-<max>
# does-not-fit <n>" ("-" when nothing fits). Exit 0, 2 on malformed input, 1
# on a usage error.

set -u
LC_ALL=C
export LC_ALL

usage="usage: frame_profile_check.sh breakdown <frames.txt> <breakdown.tty.txt> <map.txt>
       frame_profile_check.sh coverage <threshold> <full frames.txt>[,...] <name>=<frames.txt>[,...]:<map.txt>...
       frame_profile_check.sh subtree [part=draw|part=update] <name>=<frames.txt>[,...]:<map.txt>...
       frame_profile_check.sh unit-cost <frames.txt> <breakdown.tty.txt> <map.txt>
       frame_profile_check.sh unit-cost-pooled <unit-cost output>...
       frame_profile_check.sh spikes <threshold> <judge-frames.txt> <base frames.txt>[,...] [<symbol>=<frames.txt>[,...]|unresolvable]...
       frame_profile_check.sh spread <budget> <judge-frames.txt> <spikes.txt> <gains> [<ratios>]"

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
    part=draw
    case ${1:-} in
        part=draw | part=update) part=${1#part=}; shift ;;
        part=*) fail_usage "part is not draw or update: $1" ;;
    esac
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
        awk -v run="$name" -v first="$1" -v last="$2" -v part="$part" '
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
                if (file > 0 && !(file in root)) { FILENAME = previous; malformed("no root " part " line") }
                file++; previous = FILENAME
                if ($1 != "profile" || $3 != "frames" || $4 !~ /^[0-9]+$/) malformed("no profile header")
                next
            }
            (part == "update" || FNR == 2) && $1 == "root" && $2 == part && NF == 3 {
                root[file] = $3
                if (file > 1 && $3 != root[1]) malformed("root " part " " $3 ", not " root[1] " as the first profile")
                next
            }
            $1 != "frame" || $2 < first || $2 > last { next }
            $3 == "update" && NF == 6 { root_ns[file] += part == "update" ? $4 : $6; seen[file, $2] = 1; next }
            $3 == "self" && $4 == part && NF == 5 { self_ns[file] += $5; next }
            $3 == "child" && $4 == part && NF == 9 {
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
                if (!(file in root)) { FILENAME = previous; malformed("no root " part " line") }
                for (k = 1; k <= file; k++)
                    for (f = first; f <= last; f++) if (!((k, f) in seen)) malformed("no " part " line for window frame " f " in profile " k)
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

spikes() {
    [ $# -ge 3 ] || fail_usage "$usage"
    threshold=$1
    judge=$2
    case $threshold in
        '' | *[!0-9]*) fail_usage "threshold is not a count of hblanks: '$threshold'" ;;
    esac
    [ -f "$judge" ] || fail_usage "missing judge table: $judge"
    files=
    file_sets=
    set_names=cart_update60
    unresolvable=
    for profile in $(echo "$3" | tr ',' ' '); do
        [ -f "$profile" ] || fail_usage "missing profile: $profile"
        files="$files $profile"
        file_sets="$file_sets cart_update60"
    done
    [ -n "$files" ] || fail_usage "$usage"
    shift 3
    for spec in "$@"; do
        symbol=${spec%%=*}
        profiles=${spec#*=}
        case $symbol in
            '' | *[!A-Za-z0-9_]*) fail_usage "set is not <symbol>=<frames.txt>[,...] or <symbol>=unresolvable: $spec" ;;
        esac
        if [ "$symbol" = "$spec" ] || [ -z "$profiles" ] || [ "$symbol" = cart_update60 ]; then
            fail_usage "set is not <symbol>=<frames.txt>[,...] or <symbol>=unresolvable: $spec"
        fi
        case " $set_names $unresolvable " in
            *" $symbol "*) fail_usage "set given twice: $symbol" ;;
        esac
        if [ "$profiles" = unresolvable ]; then
            unresolvable="$unresolvable $symbol"
            continue
        fi
        set_names="$set_names $symbol"
        for profile in $(echo "$profiles" | tr ',' ' '); do
            [ -f "$profile" ] || fail_usage "missing profile: $profile"
            files="$files $profile"
            file_sets="$file_sets $symbol"
        done
    done
    awk -v threshold="$threshold" -v file_sets="$file_sets" -v set_names="$set_names" -v unresolvable="$unresolvable" '
        BEGIN {
            split(file_sets, set_of_file, " ")
            set_count = split(set_names, sets, " ")
            for (i = 1; i <= set_count; i++) is_set[sets[i]] = 1
            n = split(unresolvable, list, " ")
            for (i = 1; i <= n; i++) is_unresolvable[list[i]] = 1
            file = 0
        }
        function malformed(reason) {
            print "frame-profile-check: malformed " FILENAME ": " reason > "/dev/stderr"
            failed = 1
            exit 2
        }
        function check_root(    s) {
            if (file > 1 && !((file - 1) in root)) { FILENAME = previous; malformed("no root update line") }
        }
        function median(values, n,    i, j, t) {
            for (i = 1; i <= n; i++) for (j = i + 1; j <= n; j++) if (values[j] < values[i]) { t = values[i]; values[i] = values[j]; values[j] = t }
            return n % 2 ? values[(n + 1) / 2] : (values[n / 2] + values[n / 2 + 1]) / 2
        }
        FNR == 1 {
            check_root()
            file++; previous = FILENAME
            if (file == 1) {
                if ($1 != "frame" || $2 != "update" || $3 != "draw") malformed("no judge header")
                next
            }
            if ($1 != "profile" || $3 != "frames" || $4 !~ /^[0-9]+$/) malformed("no profile header")
            if ($4 != judge_frames) malformed("frames " $4 ", not " judge_frames " as the judge table")
            set = set_of_file[file - 1]
            set_k[set]++; k = set_k[set]
            next
        }
        file == 1 {
            if (NF < 2 || $1 != judge_frames || $2 !~ /^[0-9]+$/) malformed("line " FNR " is not frame " judge_frames)
            judge_update[$1] = $2
            if ($2 > threshold) { spike[$1] = 1; spike_count++ }
            judge_frames++
            next
        }
        $1 == "root" && $2 == "update" && NF == 3 {
            if ($3 != set) malformed("root update " $3 ", not " set)
            root[file - 1] = $3
            next
        }
        $1 != "frame" { next }
        $3 == "update" && NF == 6 {
            if (set == "cart_update60") {
                update_seen[k, $2] = 1
                if (spike[$2] && (k == 1 || $4 < pc_update[$2])) pc_update[$2] = $4
            }
            next
        }
        $3 == "child" && $4 == "update" && NF == 9 {
            if (k == 1 && $6 > 0) { runs[set, $5]++; if (spike[$2]) spike_runs[set, $5]++ }
            if (!spike[$2]) next
            key = set SUBSEP $2 SUBSEP $5
            if (!(key in seen_count)) { child_list[set, $2] = child_list[set, $2] " " $5; seen_count[key] = 0 }
            if (seen_count[key] == 0 || $7 < low[key]) low[key] = $7
            seen_count[key]++
        }
        function child_ns(s, f, name,    key) {
            key = s SUBSEP f SUBSEP name
            return seen_count[key] == set_k[s] ? low[key] : 0
        }
        END {
            if (failed) exit 2
            check_root()
            if (file < 2) { FILENAME = previous; malformed("no profile") }
            for (i = 2; i <= set_count; i++)
                if (set_k[sets[i]] != set_k["cart_update60"])
                    { FILENAME = sets[i]; malformed(set_k[sets[i]] " profiles, not " set_k["cart_update60"] " as the base set") }
            for (k = 1; k <= set_k["cart_update60"]; k++)
                for (f = 0; f < judge_frames; f++)
                    if (!((k, f) in update_seen)) { FILENAME = "base profile " k; malformed("no update line for frame " f) }
            class_count = 0; open_frames = 0; over = 0
            for (f = 0; f < judge_frames; f++) {
                if (!spike[f]) continue
                if (judge_update[f] > 1000) over++
                d = pc_update[f]; node = "cart_update60"; path = node; share = 100
                parent = ""; last = ""; tags = ""; open = ""; multi = 0; delete visited; visited[node] = 1
                while (1) {
                    n = split(child_list[node, f], names, " ")
                    best = ""; best_ns = -1
                    for (i = 1; i <= n; i++) { t = child_ns(node, f, names[i]); if (t > best_ns) { best_ns = t; best = names[i] } }
                    if (best == "" || d <= 0 || best_ns * 2 < d) break
                    path = path "/" best; share = best_ns * 100 / d; parent = node; last = best
                    symbol = best; sub(/#.*/, "", symbol)
                    if (symbol in is_unresolvable) { tags = tags " unresolvable"; break }
                    if (!(symbol in is_set)) { open = symbol; break }
                    if (symbol in visited) break
                    if (symbol != best) multi = 1
                    visited[symbol] = 1; node = symbol
                }
                if (multi) tags = " multi-site" tags
                if (path == "cart_update60") tags = tags " unsettled"
                line = sprintf("frame %d update %d pc-update-ns %d path %s share %.1f%%%s", f, judge_update[f], d, path, share, tags)
                if (open != "") {
                    print line " open " open
                    open_frames++
                    if (!(open in open_count)) open_symbols[++open_symbol_count] = open
                    open_count[open]++
                    continue
                }
                print line
                if (!(path in class_index)) {
                    class_index[path] = ++class_count; class_paths[class_count] = path; class_tags[class_count] = tags
                    if (parent == "") { class_runs[class_count] = judge_frames; class_spike_runs[class_count] = spike_count }
                    else { class_runs[class_count] = runs[parent, last] + 0; class_spike_runs[class_count] = spike_runs[parent, last] + 0 }
                }
                c = class_index[path]
                m = ++class_frames[c]
                class_share[c, m] = share
                class_ratio[c, m] = d > 0 ? judge_update[f] * 1000 / d : 0
                class_list[c] = class_list[c] (m > 1 ? "," : "") f
                class_sum[c] += judge_update[f]
                if (judge_update[f] > 1000) class_over[c]++
            }
            for (i = 1; i <= class_count; i++) order[i] = i
            for (i = 2; i <= class_count; i++)
                for (j = i; j > 1 && (class_sum[order[j]] > class_sum[order[j - 1]] || class_sum[order[j]] == class_sum[order[j - 1]] && class_paths[order[j]] < class_paths[order[j - 1]]); j--)
                    { t = order[j]; order[j] = order[j - 1]; order[j - 1] = t }
            for (i = 1; i <= class_count; i++) {
                c = order[i]; m = class_frames[c]
                for (j = 1; j <= m; j++) { v[j] = class_share[c, j]; r[j] = class_ratio[c, j] }
                low_share = v[1]; high_share = v[1]
                for (j = 2; j <= m; j++) { if (v[j] < low_share) low_share = v[j]; if (v[j] > high_share) high_share = v[j] }
                printf "class %s frames %d over-1000 %d update-sum %d share %.1f%%-%.1f%%-%.1f%% runs-on %d spike-runs %d hblanks-per-pc-us %.3f list %s%s\n", \
                    class_paths[c], m, class_over[c], class_sum[c], low_share, median(v, m), high_share, class_runs[c], \
                    class_spike_runs[c], median(r, m), class_list[c], class_tags[c]
            }
            for (i = 1; i <= open_symbol_count; i++) print "open " open_symbols[i] " frames " open_count[open_symbols[i]]
            print "spike-frames " spike_count + 0 " over-1000 " over " classes " class_count " open " open_frames
            if (open_frames > 0) exit 3
        }' "$judge" $files
}

spread() {
    [ $# -eq 4 ] || [ $# -eq 5 ] || fail_usage "$usage"
    budget=$1
    judge=$2
    spike_list=$3
    gains=$4
    ratios=${5:-}
    case $budget in
        '' | *[!0-9]*) fail_usage "budget is not a count of hblanks: '$budget'" ;;
    esac
    [ -f "$judge" ] || fail_usage "missing judge table: $judge"
    [ -f "$spike_list" ] || fail_usage "missing spikes output: $spike_list"
    echo "$gains" | grep -Eq '^default=[0-9]+(\.[0-9]+)?(,[0-9]+-[0-9]+=[0-9]+(\.[0-9]+)?)*$' \
        || fail_usage "gains are not default=<pct>[,<first>-<last>=<pct>]...: '$gains'"
    echo "$gains" | tr ',' '\n' | awk -F= '$2 > 100 { exit 1 } $1 ~ /-/ { split($1, r, "-"); if (r[1] + 0 > r[2] + 0) exit 1 }' \
        || fail_usage "a gain above 100 or a range backwards: '$gains'"
    if [ -n "$ratios" ]; then
        echo "$ratios" | grep -Eq '^[A-Za-z0-9_]+=[0-9]+(\.[0-9]+)?(,[A-Za-z0-9_]+=[0-9]+(\.[0-9]+)?)*$' \
            || fail_usage "ratios are not <child>=<ratio>[,...]: '$ratios'"
    fi
    awk -v budget="$budget" -v gains="$gains" -v ratios="$ratios" '
        BEGIN {
            cap = 600
            n = split(gains, specs, ",")
            for (i = 1; i <= n; i++) {
                split(specs[i], kv, "=")
                if (kv[1] == "default") { default_gain = kv[2] + 0; continue }
                split(kv[1], range, "-")
                range_count++; range_first[range_count] = range[1] + 0; range_last[range_count] = range[2] + 0
                range_gain[range_count] = kv[2] + 0
            }
            n = split(ratios, specs, ",")
            for (i = 1; i <= n; i++) { split(specs[i], kv, "="); ratio_of[kv[1]] = kv[2] + 0 }
            file = 0
        }
        function malformed(reason) {
            print "frame-profile-check: malformed " FILENAME ": " reason > "/dev/stderr"
            failed = 1
            exit 2
        }
        function gain(f,    i, g) {
            g = default_gain
            for (i = 1; i <= range_count; i++) if (f >= range_first[i] && f <= range_last[i]) g = range_gain[i]
            return g
        }
        function room(f, update,    r) {
            r = budget - submit[f] - int(draw[f] * (100 - gain(f)) / 100 + 0.5 + 1e-9) - update
            return r > 0 ? r : 0
        }
        function median(values, n,    i, j, t) {
            for (i = 1; i <= n; i++) for (j = i + 1; j <= n; j++) if (values[j] < values[i]) { t = values[i]; values[i] = values[j]; values[j] = t }
            return n % 2 ? values[(n + 1) / 2] : (values[n / 2] + values[n / 2 + 1]) / 2
        }
        FNR == 1 { file++ }
        file == 1 && FNR == 1 {
            if ($1 != "frame" || $2 != "update" || $3 != "draw" || $4 != "submit") malformed("no judge header")
            next
        }
        file == 1 {
            if (NF < 4 || $1 != judge_frames || $2 !~ /^[0-9]+$/ || $3 !~ /^[0-9]+$/ || $4 !~ /^[0-9]+$/)
                malformed("line " FNR " is not frame " judge_frames)
            update[$1] = $2; draw[$1] = $3; submit[$1] = $4
            judge_frames++
            next
        }
        $1 != "frame" { next }
        {
            if (NF < 10 || $2 !~ /^[0-9]+$/ || $3 != "update" || $5 != "pc-update-ns" || $7 != "path" || $9 != "share" \
                || $10 !~ /^[0-9]+(\.[0-9]+)?%$/)
                malformed("line " FNR " is not a spikes frame line")
            f = $2 + 0
            if (f >= judge_frames) malformed("frame " f " is not in the judge table")
            if ($4 != update[f]) malformed("frame " f " update " $4 ", not " update[f] " as the judge table")
            is_spike[f] = 1
            for (i = 11; i <= NF; i++) if ($i == "open") next
            closed_count++; closed_frame[closed_count] = f; closed_path[closed_count] = $8
            share = $10; sub(/%$/, "", share); closed_share[closed_count] = share + 0
        }
        END {
            if (failed) exit 2
            if (file < 2) malformed("no spikes output")
            for (s = 1; s <= closed_count; s++) {
                f = closed_frame[s]; path = closed_path[s]
                child = ""
                if (split(path, parts, "/") > 1) { child = parts[2]; sub(/#.*/, "", child) }
                rho = (child in ratio_of) ? ratio_of[child] : 1
                portion = closed_share[s] * rho / 100
                if (portion > 1) portion = 1
                work = portion * update[f]
                work = int(work - 1e-9) + (work - 1e-9 > int(work - 1e-9) ? 1 : 0)
                if (work < 0) work = 0
                total = room(f, update[f] - work); frames = 1
                while (total < work && frames < cap && f + frames < judge_frames) { total += room(f + frames, update[f + frames]); frames++ }
                fits = total >= work
                line = sprintf("frame %d class %s work %d frames %s", f, path, work, fits ? frames : "does-not-fit")
                if (fits) {
                    overlaps = ""
                    for (i = f + 1; i < f + frames; i++) if (i in is_spike) overlaps = overlaps (overlaps == "" ? "" : ",") i
                    if (overlaps != "") line = line " overlaps " overlaps
                }
                print line
                if (!(path in class_index)) { class_index[path] = ++class_count; class_paths[class_count] = path }
                c = class_index[path]
                class_works[c, ++class_size[c]] = work
                if (fits) class_fits[c, ++class_fit_count[c]] = frames
                else class_misses[c]++
            }
            for (c = 1; c <= class_count; c++) {
                m = class_size[c]
                for (j = 1; j <= m; j++) w[j] = class_works[c, j]
                low = w[1]; high = w[1]
                for (j = 2; j <= m; j++) { if (w[j] < low) low = w[j]; if (w[j] > high) high = w[j] }
                work_range = low "-" median(w, m) "-" high
                m = class_fit_count[c] + 0
                if (m == 0) frame_range = "-"
                else {
                    for (j = 1; j <= m; j++) w[j] = class_fits[c, j]
                    low = w[1]; high = w[1]
                    for (j = 2; j <= m; j++) { if (w[j] < low) low = w[j]; if (w[j] > high) high = w[j] }
                    frame_range = low "-" median(w, m) "-" high
                }
                print "class " class_paths[c] " work " work_range " frames " frame_range " does-not-fit " class_misses[c] + 0
            }
        }' "$judge" "$spike_list"
}

mode=${1:-}
[ $# -gt 0 ] && shift
case $mode in
    breakdown) breakdown "$@" ;;
    coverage) coverage "$@" ;;
    subtree) subtree "$@" ;;
    unit-cost) unit_cost "$@" ;;
    unit-cost-pooled) unit_cost_pooled "$@" ;;
    spikes) spikes "$@" ;;
    spread) spread "$@" ;;
    *) fail_usage "$usage" ;;
esac
