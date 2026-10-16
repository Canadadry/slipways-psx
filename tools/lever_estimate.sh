#!/bin/sh
# usage: lever_estimate.sh <frame-table> <situation-table> <levers-doc> <budget-hblanks> <one-off-allowance-vsyncs> [haircut=<pct>] [projected=<dir>]
# Applies the lever-table block of <levers-doc> (docs/psx-levers.md) to the
# frame table joined with the situation table, ranks the levers greedily by
# the steady frames over the budget they leave, checks the document's order
# against that ranking, and prints the projection (format in
# docs/psx-game.md, PC frame profile). Writes the nominal and pessimistic
# projected tables to <dir>/nominal-frames.txt and
# <dir>/pessimistic-frames.txt (default dir tmp/lever-estimate). Exit 0 once
# written, 3 when the document's order differs from the ranking, 2 on a
# malformed table or block (naming the file and line), 1 on a usage error.

set -u
LC_ALL=C
export LC_ALL

usage="usage: lever_estimate.sh <frame-table> <situation-table> <levers-doc> <budget-hblanks> <one-off-allowance-vsyncs> [haircut=<pct>] [projected=<dir>]"

fail_usage() {
    echo "lever-estimate: $1" >&2
    exit 1
}

[ $# -ge 5 ] || fail_usage "$usage"
frames=$1
situations=$2
levers=$3
budget=$4
allowance=$5
shift 5
haircut=20
projected=tmp/lever-estimate
for option in "$@"; do
    case $option in
        haircut=*) haircut=${option#haircut=} ;;
        projected=*) projected=${option#projected=} ;;
        *) fail_usage "unknown option '$option'; $usage" ;;
    esac
done
[ -f "$frames" ] || fail_usage "missing frame table: $frames"
[ -f "$situations" ] || fail_usage "missing situation table: $situations"
[ -f "$levers" ] || fail_usage "missing levers document: $levers"
case $budget in '' | *[!0-9]* | 0) fail_usage "budget is not a positive count of hblanks: '$budget'" ;; esac
case $allowance in '' | *[!0-9]* | 0) fail_usage "allowance is not a positive count of vsyncs: '$allowance'" ;; esac
echo "$haircut" | grep -Eq '^[0-9]+(\.[0-9]+)?$' && awk -v h="$haircut" 'BEGIN { exit !(h + 0 <= 100) }' \
    || fail_usage "haircut is not a percentage from 0 to 100: '$haircut'"
[ -n "$projected" ] || fail_usage "projected= names no directory"
mkdir -p "$projected" || fail_usage "cannot create $projected"
nominal_table=$projected/nominal-frames.txt
pessimistic_table=$projected/pessimistic-frames.txt

awk -v frames_path="$frames" -v situations_path="$situations" -v levers_path="$levers" -v budget="$budget" \
    -v haircut="$haircut" -v nominal_path="$nominal_table" -v pessimistic_path="$pessimistic_table" '
    function fail(path, line, reason) {
        print "lever-estimate: " path ":" line ": " reason > "/dev/stderr"
        failed = 1
        exit 2
    }
    function count(text) { return text ~ /^[0-9]+$/ }
    function number(text) { return text ~ /^[0-9]+(\.[0-9]+)?$/ }
    function round_half_up(x) { return int(x + 0.5 + 1e-9) }

    function read_frames(   line, n, field, i) {
        if ((getline line < frames_path) <= 0 || line != "frame update draw submit total vsyncs")
            fail(frames_path, 1, "no \"frame update draw submit total vsyncs\" header")
        frame_count = 0
        while ((getline line < frames_path) > 0) {
            n = split(line, field, " ")
            if (n != 6) fail(frames_path, frame_count + 2, "not 6 fields: " line)
            for (i = 1; i <= 6; i++) if (!count(field[i])) fail(frames_path, frame_count + 2, "not numeric: " line)
            if (field[1] != frame_count) fail(frames_path, frame_count + 2, "frame " field[1] ", expected " frame_count)
            part[frame_count, "update"] = field[2]; part[frame_count, "draw"] = field[3]
            part[frame_count, "submit"] = field[4]; total[frame_count] = field[5]
            frame_count++
        }
    }
    function read_situations(   line, n, field, lines) {
        if ((getline line < situations_path) <= 0 || line != "frame situation family")
            fail(situations_path, 1, "no \"frame situation family\" header")
        lines = 0
        while ((getline line < situations_path) > 0) {
            n = split(line, field, " ")
            if (n != 3 || !count(field[1])) fail(situations_path, lines + 2, "not \"<frame> <situation> <family>\": " line)
            if (field[1] != lines) fail(situations_path, lines + 2, "frame " field[1] ", expected " lines)
            if (field[3] != "steady" && field[3] != "one-off") fail(situations_path, lines + 2, "family is neither steady nor one-off: " line)
            label[lines] = field[2]; steady[lines] = field[3] == "steady"
            lines++
        }
        if (lines != frame_count)
            fail(situations_path, lines + 1, "situation table has " lines " frames, frame table " frame_count)
    }

    function read_levers(   line, number_of_line, inside, blocks, n, field, id, i, key) {
        number_of_line = 0; inside = 0; blocks = 0; fence = 0
        while ((getline line < levers_path) > 0) {
            number_of_line++
            if (line ~ /^```/) {
                if (inside) { inside = 0; fence = 0; continue }
                if (fence) { fence = 0; continue }
                if (line ~ /^```lever-table[ \t]*$/) {
                    blocks++
                    if (blocks > 1) fail(levers_path, number_of_line, "a second lever-table block")
                    inside = 1
                } else fence = 1
                continue
            }
            if (!inside) continue
            sub(/#.*/, "", line)
            n = split(line, field, " ")
            if (n == 0) continue
            if (field[1] == "lever") read_lever(field, n, number_of_line)
            else if (field[1] == "gain") read_gain(field, n, number_of_line)
            else if (field[1] == "scale") read_scale(field, n, number_of_line)
            else fail(levers_path, number_of_line, "not a lever, gain or scale line: " field[1])
        }
        if (blocks == 0) fail(levers_path, number_of_line, "no lever-table block")
        if (inside) fail(levers_path, number_of_line, "the lever-table block is not closed")
        for (i = 1; i <= gain_lines; i++) {
            id = gain_lever[i]
            if (!(id in lever_line)) fail(levers_path, gain_line[i], "gain for undeclared lever " id)
            gain_count[id]++
            key = id SUBSEP gain_count[id]
            gain_situation[key] = gain_situation_of[i]; gain_part[key] = gain_part_of[i]; gain_value[key] = gain_percent[i]
        }
        for (i = 1; i <= lever_count; i++) {
            id = lever_id[i]
            if (after[id] != "" && !(after[id] in lever_line))
                fail(levers_path, lever_line[id], "lever " id " is after unknown lever " after[id])
        }
        for (i = 1; i <= lever_count; i++) check_cycle(lever_id[i])
    }
    function read_lever(field, n, at,   id, i) {
        if (n < 8 || field[5] != "basis" || field[7] != "measure")
            fail(levers_path, at, "not \"lever <id> <status> <cost> basis <basis> measure <run> [probe <run>] [after <id>]\"")
        id = field[2]
        if (id !~ /^[A-Za-z0-9_-]+$/) fail(levers_path, at, "lever id is not a name: " id)
        if (id in lever_line) fail(levers_path, at, "lever " id " declared twice")
        if (field[3] != "estimated" && field[3] != "measured" && field[3] != "in-base")
            fail(levers_path, at, "status is not estimated, measured or in-base: " field[3])
        if (field[4] != "S" && field[4] != "M" && field[4] != "L") fail(levers_path, at, "cost is not S, M or L: " field[4])
        if (field[6] != "calls" && field[6] != "share") fail(levers_path, at, "basis is not calls or share: " field[6])
        lever_line[id] = at; status[id] = field[3]; cost[id] = field[4]; basis[id] = field[6]; measure[id] = field[8]
        probe[id] = ""; after[id] = ""
        for (i = 9; i <= n; i += 2) {
            if (i == n) fail(levers_path, at, field[i] " has no value")
            if (field[i] == "probe" && probe[id] == "") probe[id] = field[i + 1]
            else if (field[i] == "after" && after[id] == "") after[id] = field[i + 1]
            else fail(levers_path, at, "unexpected " field[i])
        }
        if (measure[id] == "psx-full-game-check" && probe[id] == "")
            fail(levers_path, at, "lever " id " is measured on psx-full-game-check without a probe run")
        if (measure[id] != "psx-full-game-check" && probe[id] != "")
            fail(levers_path, at, "lever " id " has a probe run but is measured on short run " measure[id])
        lever_count++; lever_id[lever_count] = id
    }
    function read_gain(field, n, at,   key) {
        if (n != 5) fail(levers_path, at, "not \"gain <id> <situation> <update|draw|submit> <percent>\"")
        if (!(field[3] in steady_rank)) {
            if (field[3] in one_off_known) fail(levers_path, at, "situation " field[3] " is one-off")
            fail(levers_path, at, "unknown situation " field[3])
        }
        if (field[4] != "update" && field[4] != "draw" && field[4] != "submit")
            fail(levers_path, at, "part is not update, draw or submit: " field[4])
        if (!number(field[5]) || field[5] + 0 > 100) fail(levers_path, at, "percent is not from 0 to 100: " field[5])
        key = field[2] SUBSEP field[3] SUBSEP field[4]
        if (key in gain_seen) fail(levers_path, at, "a second gain for " field[2] " " field[3] " " field[4])
        gain_seen[key] = 1
        gain_lines++
        gain_lever[gain_lines] = field[2]; gain_situation_of[gain_lines] = field[3]; gain_part_of[gain_lines] = field[4]
        gain_percent[gain_lines] = field[5] + 0; gain_line[gain_lines] = at
    }
    function read_scale(field, n, at) {
        if (n != 3) fail(levers_path, at, "not \"scale <calls|share> <ratio>\"")
        if (field[2] != "calls" && field[2] != "share") fail(levers_path, at, "basis is not calls or share: " field[2])
        if (field[2] in scale) fail(levers_path, at, "a second scale for " field[2])
        if (!number(field[3]) || field[3] + 0 <= 0) fail(levers_path, at, "ratio is not a positive number: " field[3])
        scale[field[2]] = field[3] + 0
    }
    function check_cycle(id,   at, steps) {
        at = id
        for (steps = 0; after[at] != ""; steps++) {
            if (steps > lever_count) fail(levers_path, lever_line[id], "lever " id " is in an after cycle")
            at = after[at]
        }
    }

    function applied_gain(id, j) {
        if (status[id] == "measured") return gain_value[id, j]
        return gain_value[id, j] * ((basis[id] in scale) ? scale[basis[id]] : 1)
    }
    # Projects every steady frame with the levers of chosen[] and the haircut; sets projected_total[],
    # over_after[situation], saved (hblanks off the committed frames over); returns the steady frames over.
    function project(cut,   id, j, key, frame, name, after_total, p, over, kind) {
        delete reduction
        for (id in chosen) for (j = 1; j <= gain_count[id]; j++)
            reduction[gain_situation[id, j], gain_part[id, j]] += applied_gain(id, j)
        for (key in reduction) {
            reduction[key] *= 1 - cut / 100
            if (reduction[key] > 100) reduction[key] = 100
        }
        delete over_after
        over = 0; saved = 0
        for (frame = 0; frame < frame_count; frame++) {
            name = label[frame]
            if (!steady[frame] || name == "title") { projected_total[frame] = total[frame]; after_total = total[frame] }
            else {
                after_total = 0
                for (p = 1; p <= 3; p++) {
                    kind = parts[p]
                    key = name SUBSEP kind
                    projected_part[frame, kind] = (key in reduction) ? round_half_up(part[frame, kind] * (1 - reduction[key] / 100)) : part[frame, kind]
                    after_total += projected_part[frame, kind]
                }
                projected_total[frame] = after_total
            }
            if (!steady[frame]) continue
            if (after_total > budget) { over++; over_after[name]++ }
            if (total[frame] > budget) saved += total[frame] - after_total
        }
        return over
    }
    function write_table(path,   frame, p, kind, line, t) {
        print "frame update draw submit total vsyncs" > path
        for (frame = 0; frame < frame_count; frame++) {
            line = frame
            for (p = 1; p <= 3; p++) {
                kind = parts[p]
                line = line " " ((steady[frame] && label[frame] != "title" && (frame SUBSEP kind) in projected_part) ? projected_part[frame, kind] : part[frame, kind])
            }
            t = projected_total[frame]
            print line " " t " " (t > budget ? int((t + budget - 1) / budget) : 0) > path
        }
        close(path)
    }
    # True when lever a ranks before lever b: fewer steady frames over, more hblanks saved, cheaper, smaller id.
    function better(over_a, saved_a, a, over_b, saved_b, b) {
        if (over_a != over_b) return over_a < over_b
        if (saved_a != saved_b) return saved_a > saved_b
        if (cost_rank[cost[a]] != cost_rank[cost[b]]) return cost_rank[cost[a]] < cost_rank[cost[b]]
        return a < b
    }

    BEGIN {
        parts[1] = "update"; parts[2] = "draw"; parts[3] = "submit"
        cost_rank["S"] = 1; cost_rank["M"] = 2; cost_rank["L"] = 3
        steady_order = "title attract map-rest map-move slipway-aim build menu tech-panel score-panel pause"
        steady_known = split(steady_order, steady_name, " ")
        for (i = 1; i <= steady_known; i++) steady_rank[steady_name[i]] = i
        split("galaxy-generation return-to-title game-end new-year research colonise structure-built slipway-built probe screen-change", one_off_name, " ")
        for (i in one_off_name) one_off_known[one_off_name[i]] = 1

        read_frames()
        read_situations()
        read_levers()

        steady_frames = 0; steady_sum = 0; start_over = 0; one_off_vsyncs = 0
        for (frame = 0; frame < frame_count; frame++) {
            name = label[frame]
            if (steady[frame]) {
                steady_frames++; steady_sum += total[frame]
                present[name] = 1; situation_frames[name]++; situation_sum[name] += total[frame]
                if (total[frame] > budget) { start_over++; over_before[name]++ }
            } else if (total[frame] > budget) one_off_vsyncs += int((total[frame] + budget - 1) / budget)
        }
        mean = steady_frames > 0 ? steady_sum / steady_frames : 0
        printf "mean-cost steady %.1f threshold %.1f\n", mean, 0.02 * mean
        for (i = 1; i <= steady_known; i++) {
            name = steady_name[i]
            if (!(name in over_before)) continue
            m = situation_sum[name] / situation_frames[name]
            printf "situation-threshold %s mean %.1f threshold %.1f\n", name, m, 0.02 * m
        }
        print "start steady-over " start_over " one-off-vsyncs " one_off_vsyncs
        in_base = ""
        for (i = 1; i <= lever_count; i++) {
            id = lever_id[i]
            if (status[id] == "in-base") in_base = in_base " " id
            else { candidates++; document[candidates] = id }
        }
        print "in-base" (in_base == "" ? " none" : in_base)

        delete chosen
        for (rank = 1; rank <= candidates; rank++) {
            best = ""
            for (i = 1; i <= candidates; i++) {
                id = document[i]
                if (id in chosen) continue
                if (after[id] != "" && !(after[id] in chosen) && status[after[id]] != "in-base") continue
                chosen[id] = 1
                over = project(0)
                delete chosen[id]
                if (best == "" || better(over, saved, id, best_over, best_saved, best)) {
                    best = id; best_over = over; best_saved = saved
                }
            }
            computed[rank] = best
            chosen[best] = 1
            nominal_over = project(0); nominal_saved = saved
            pessimistic_over = project(haircut)
            print "lever " rank " " best " " status[best] " basis " basis[best] " steady-over " nominal_over \
                " pessimistic " pessimistic_over " saved " nominal_saved
        }

        end_over = project(haircut)
        for (name in over_after) pessimistic_end[name] = over_after[name]
        write_table(pessimistic_path)
        end_pessimistic = end_over
        end_over = project(0)
        write_table(nominal_path)
        for (i = 1; i <= steady_known; i++) {
            name = steady_name[i]
            if (!(name in present)) continue
            print "situation " name " over " over_before[name] + 0 " end " over_after[name] + 0 " pessimistic " pessimistic_end[name] + 0
        }
        order = "ok"
        for (rank = 1; rank <= candidates; rank++) {
            if (document[rank] != computed[rank]) {
                order = "differs rank " rank " document " document[rank] " computed " computed[rank]
                break
            }
        }
        print "order " order
        print "end steady-over " end_over " pessimistic " end_pessimistic
        exit order == "ok" ? 0 : 3
    }
'
status=$?
case $status in 0 | 3) ;; *) exit $status ;; esac
sh tools/psx_real_game_budget.sh "$nominal_table" "$situations" "$budget" "$allowance" | sed 's/^/nominal: /'
sh tools/psx_real_game_budget.sh "$pessimistic_table" "$situations" "$budget" "$allowance" | sed 's/^/pessimistic: /'
exit $status
