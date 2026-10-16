#!/bin/sh
# usage: steady_profile.sh overhead <breakdown.tty>:<plain.tty>:<map.txt>...
#        steady_profile.sh profile <overhead-per-call> <groups.txt> <map.txt> <plain.tty> <breakdown.tty> <draw frames.txt>[,...] <cart frames.txt>[,...]
#        steady_profile.sh no-run <situation> <budget> <judge-frames.txt> <situations.txt> <groups.txt> <draw frames.txt>[,...] <cart frames.txt>[,...]
# Draw profiles are tmp/frame_profile's rooted at draw_layers and cart_update60, cart profiles at
# cart_draw and input_step_run; PC figures are medians over each set of a child's whole time (ns,
# not net: net leaves out the hooks inside the primitives but not their time). Every figure is in hblanks
# per measured frame. Exit 0 once written, 2 on malformed input or a calls mismatch, 1 on a
# usage error.

set -u
LC_ALL=C
export LC_ALL

here=$(dirname "$0")
usage="usage: steady_profile.sh overhead <breakdown.tty>:<plain.tty>:<map.txt>...
       steady_profile.sh profile <overhead-per-call> <groups.txt> <map.txt> <plain.tty> <breakdown.tty> <draw frames.txt>[,...] <cart frames.txt>[,...]
       steady_profile.sh no-run <situation> <budget> <judge-frames.txt> <situations.txt> <groups.txt> <draw frames.txt>[,...] <cart frames.txt>[,...]"

fail_usage() {
    echo "steady-profile: $1" >&2
    exit 1
}

require_file() {
    [ -f "$2" ] || fail_usage "missing $1: $2"
}

# usage: window_of <map>; prints "<run-first> <run-last>" (frame_profile_check.sh's rule).
window_of() {
    require_file map "$1"
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

psx_lines() {
    tr -d '\r' < "$2" | sed -n "s/^psx-frame: frame /$1 /p; s/^psx-draw: frame /D /p; s/^psx-draw: clk8-per-64-hblanks /C /p"
}

# usage: profile_lines <tag> <frames.txt>[,...]
profile_lines() {
    k=0
    for file in $(echo "$2" | tr ',' ' '); do
        k=$((k + 1))
        sed "s/^/$1 $k /" "$file"
    done
}

check_profiles() {
    for file in $(echo "$1" | tr ',' ' '); do
        require_file profile "$file"
    done
}

overhead() {
    [ $# -ge 1 ] || fail_usage "$usage"
    for spec in "$@"; do
        breakdown_tty=${spec%%:*}
        rest=${spec#*:}
        plain_tty=${rest%%:*}
        map=${rest#*:}
        [ "$breakdown_tty" != "$spec" ] && [ "$plain_tty" != "$rest" ] || fail_usage "run is not <breakdown.tty>:<plain.tty>:<map.txt>: $spec"
        require_file "breakdown tty" "$breakdown_tty"
        require_file "plain tty" "$plain_tty"
        window=$(window_of "$map") || exit 1
        echo "R $window"
        psx_lines B "$breakdown_tty" | grep -v '^C '
        psx_lines P "$plain_tty" | grep '^P '
    done | awk '
        BEGIN { family_count = split("cls pset line rect rectfill circ circfill spr print", families, " ") }
        function malformed(reason) { print "steady-profile: " reason > "/dev/stderr"; failed = 1; exit 2 }
        function close_run(   r) {
            if (!runs) return
            for (r = first; r <= last; r++)
                if (!((runs, r, "B") in seen) || !((runs, r, "D") in seen) || !((runs, r, "P") in seen))
                    malformed("run " runs ": no breakdown, draw or plain line for frame " r)
        }
        $1 == "R" { close_run(); runs++; first = $2; last = $3; next }
        $2 < first || $2 > last { next }
        $1 == "B" && NF == 12 { breakdown += $6; seen[runs, $2, "B"] = 1; next }
        $1 == "P" && NF == 12 { plain += $6; seen[runs, $2, "P"] = 1; next }
        $1 == "D" && NF == 34 {
            for (i = 1; i <= family_count; i++) calls += $(3 * i + 2)
            seen[runs, $2, "D"] = 1; next
        }
        END {
            if (failed) exit 2
            close_run()
            if (failed) exit 2
            if (calls == 0) malformed("no backend call in any window")
            printf "overhead-per-call %.4f runs %d calls %d breakdown-draw %d plain-draw %d\n", (breakdown - plain) / calls, runs, calls, breakdown, plain
        }'
}

# The shared join: tagged lines on stdin, mode profile or no-run.
join_awk() {
    awk -v mode="$1" -v first="${2:-0}" -v last="${3:-0}" -v overhead="${4:-0}" -v situation="${5:-}" -v budget="${6:-0}" '
        BEGIN {
            family_count = split("cls pset line rect rectfill circ circfill spr print", families, " ")
            group_count = split("planets slipways particles hud panels map overlays frame", groups, " ")
            for (i = 1; i <= group_count; i++) known_group[groups[i]] = 1
            for (i = 1; i <= family_count; i++) family_index[families[i]] = i
        }
        function malformed(reason) { print "steady-profile: " reason > "/dev/stderr"; failed = 1; exit 2 }
        function base(symbol) { sub(/#.*/, "", symbol); sub(/@.*/, "", symbol); return symbol }
        function wanted(f) {
            if (mode == "profile") return f >= first && f <= last
            return label[f] == situation && judge_total[f] > budget
        }
        function median(values, n,    i, j, t) {
            for (i = 1; i <= n; i++) for (j = i + 1; j <= n; j++) if (values[j] < values[i]) { t = values[i]; values[i] = values[j]; values[j] = t }
            return n % 2 ? values[(n + 1) / 2] : (values[n / 2] + values[n / 2 + 1]) / 2
        }
        $1 == "G" && (NF == 1 || $2 ~ /^#/) { next }
        $1 == "G" {
            if (NF != 3 || !($3 in known_group)) malformed("group table line is not <symbol> <group>: " substr($0, 3))
            group_of[$2] = $3; next
        }
        $1 == "J" { if (NF == 7 && $2 ~ /^[0-9]+$/) { judge_update[$2] = $3; judge_draw[$2] = $4; judge_submit[$2] = $5; judge_total[$2] = $6 }; next }
        $1 == "S" { if (NF == 4 && $2 ~ /^[0-9]+$/) label[$2] = $3; next }
        $1 == "C" && NF == 2 { if (calibration == "") calibration = $2; next }
        ($1 == "P" || $1 == "B") && NF == 12 {
            if (!wanted($2)) next
            if ($3 != "update" || $5 != "draw" || $7 != "submit" || $9 != "total") malformed("malformed psx-frame line: frame " $2)
            seen[$1, $2] = 1
            if ($1 == "P") { plain_update += $4; plain_draw += $6; plain_submit += $8; plain_total += $10 }
            else breakdown_draw += $6
            next
        }
        $1 == "D" && NF == 34 {
            if (!wanted($2)) next
            seen["D", $2] = 1
            for (i = 1; i <= family_count; i++) {
                if ($(3 * i) != families[i]) malformed("malformed psx-draw line: frame " $2)
                ticks[families[i]] += $(3 * i + 1); calls[families[i]] += $(3 * i + 2)
            }
            next
        }
        ($1 == "A" || $1 == "K") && $3 == "root" {
            expected = $1 == "A" ? ($4 == "draw" ? "draw_layers" : "cart_update60") : ($4 == "draw" ? "cart_draw" : "input_step_run")
            if ($5 != expected) malformed("profile " $2 " of the " ($1 == "A" ? "draw" : "cart") " set has root " $4 " " $5 ", not " expected)
            roots[$1, $2]++; next
        }
        ($1 == "A" || $1 == "K") && $3 == "frame" {
            set = $1; k = $2; f = $4
            if (k > count[set]) count[set] = k
            if (!wanted(f)) next
            if (set == "A") profile_frames[k, f] = 1
            if ($5 == "update" && NF == 8) { if (set == "K") input[k] += $6; next }
            if ($5 == "child" && NF == 11) {
                name = base($7)
                if ($6 == "update") { if (set == "A") { update_net[k, name] += $9; piece[name] = 1 }; next }
                if (set == "K" && name == "draw_layers") { layers_net[k] += $9; next }
                net[set, k, name] += $9; member[set, name] = 1; next
            }
            if ($5 == "self" && NF == 7) {
                name = set == "A" ? "draw_layers" : "cart_draw"
                if ($6 == "update") { if (set == "A") { update_net[k, "cart_update60"] += $7; piece["cart_update60"] = 1 }; next }
                net[set, k, name] += $7; member[set, name] = 1; next
            }
            if (($5 == "child-family" && NF == 10) || ($5 == "self-family" && NF == 9)) {
                if ($6 != "draw") next
                if ($5 == "self-family") { name = set == "A" ? "draw_layers" : "cart_draw"; family = $7; ns = $9 }
                else { name = base($7); family = $8; ns = $10 }
                if (!(family in family_index)) next
                if (set == "K" && name == "draw_layers") { layers_family[k, family] += ns; layers_in[k] += ns; next }
                family_ns[set, k, name, family] += ns; in_ns[set, k, name] += ns; member[set, name] = 1
            }
            next
        }
        function per_frame(x) { return sprintf("%.1f", x / frames) }
        function share(x) { return sprintf("%.1f%%", measured > 0 ? x * 100 / measured : 0) }
        # The median over a set of one per-profile figure; kind: net, outside or a family name.
        function set_median(set, name, kind,    k, v, x) {
            for (k = 1; k <= count[set]; k++) {
                if (kind == "net") x = net[set, k, name]
                else if (kind == "outside") { x = net[set, k, name] - in_ns[set, k, name]; if (x < 0) x = 0 }
                else x = family_ns[set, k, name, kind]
                v[k] = x
            }
            return median(v, count[set])
        }
        function layers_median(kind,    k, v, x) {
            for (k = 1; k <= count["K"]; k++) {
                if (kind == "net") x = layers_net[k]
                else if (kind == "outside") { x = layers_net[k] - layers_in[k]; if (x < 0) x = 0 }
                else x = layers_family[k, kind]
                v[k] = x
            }
            return median(v, count["K"])
        }
        # weight[name] for kind: the draw set scaled to the cart set'"'"'s draw_layers, plus the cart set.
        function weights(kind, weight,    name, total, scale) {
            total = 0
            for (name in function_name) if (("A", name) in member) total += set_median("A", name, kind)
            scale = total > 0 ? layers_median(kind) / total : 0
            for (name in function_name) {
                weight[name] = 0
                if (("A", name) in member) weight[name] += set_median("A", name, kind) * scale
                if (("K", name) in member) weight[name] += set_median("K", name, kind)
            }
        }
        function split_update(    name, k, v, total, value, order, n, i, j, t, other, folded) {
            for (k = 1; k <= count["K"]; k++) v[k] = input[k]
            value["input_step_run"] = median(v, count["K"])
            for (name in piece) {
                for (k = 1; k <= count["A"]; k++) v[k] = update_net[k, name]
                value[name] = median(v, count["A"])
            }
            total = 0
            for (name in value) total += value[name]
            if (total == 0 && update_sum > 0) malformed("no PC update time in the frames")
            n = 0
            for (name in value) { value[name] = total > 0 ? update_sum * value[name] / total : 0; order[++n] = name }
            for (i = 1; i <= n; i++) for (j = i + 1; j <= n; j++)
                if (value[order[j]] > value[order[i]] || value[order[j]] == value[order[i]] && order[j] < order[i]) { t = order[i]; order[i] = order[j]; order[j] = t }
            other = 0; folded = 0
            for (i = 1; i <= n; i++) {
                if (value[order[i]] < measured / 100) { other += value[order[i]]; folded = 1; continue }
                print "update " order[i] " " per_frame(value[order[i]]) " share " share(value[order[i]])
                lines_sum += value[order[i]]
            }
            if (folded) { print "update other " per_frame(other) " share " share(other); lines_sum += other }
        }
        END {
            if (failed) exit 2
            if (count["A"] == 0 || count["K"] == 0) malformed("no profile line")
            for (set_k = 1; set_k <= count["A"]; set_k++) if (roots["A", set_k] != 2) malformed("draw profile " set_k " has no root lines")
            for (set_k = 1; set_k <= count["K"]; set_k++) if (roots["K", set_k] != 2) malformed("cart profile " set_k " has no root lines")
            frames = 0
            if (mode == "profile") {
                if (calibration == "" || calibration !~ /^[0-9]+$/ || calibration == 0) malformed("no clk8-per-64-hblanks line in the breakdown tty")
                for (f = first; f <= last; f++) {
                    if (!(("P", f) in seen)) malformed("no plain psx-frame line for frame " f)
                    if (!(("B", f) in seen)) malformed("no breakdown psx-frame line for frame " f)
                    if (!(("D", f) in seen)) malformed("no psx-draw line for frame " f)
                    if (!((1, f) in profile_frames)) malformed("no draw profile line for frame " f)
                    frames++
                }
                update_sum = plain_update; submit_sum = plain_submit; measured = plain_total
            } else {
                for (f in label) if (wanted(f)) {
                    if (!(f in judge_total)) malformed("no judge line for frame " f)
                    if (!((1, f) in profile_frames)) malformed("no draw profile line for frame " f)
                    frames++; update_sum += judge_update[f]; submit_sum += judge_submit[f]; draw_sum += judge_draw[f]
                    measured += judge_total[f]
                }
                if (frames == 0) malformed("no " situation " frame over " budget)
            }
            for (key in member) { split(key, parts, SUBSEP); function_name[parts[2]] = 1 }
            for (name in function_name) if (!(name in group_of)) malformed("no group for symbol " name " in the group table")
            if (mode == "profile") {
                total_calls = 0; in_calls = 0
                for (i = 1; i <= family_count; i++) {
                    family = families[i]
                    total_calls += calls[family]
                    hblanks = ticks[family] * 64 / calibration; in_calls += hblanks
                    primitive = hblanks - overhead * calls[family]
                    weights(family, weight); weight_total = 0
                    for (name in function_name) weight_total += weight[name]
                    if (weight_total == 0) { if (primitive != 0) malformed("PSX time in " family " but no PC time"); continue }
                    for (name in function_name) in_primitives[name] += primitive * weight[name] / weight_total
                }
                outside = breakdown_draw - in_calls
                print "window run-frames " first "-" last " frames " frames " overhead-per-call " overhead " calls " total_calls
                kind = "outside"; code_sum = outside
            } else {
                print "no-run " situation " frames " frames " budget " budget " shares pc-time"
                kind = "net"; code_sum = draw_sum
            }
            weights(kind, weight); weight_total = 0
            for (name in function_name) weight_total += weight[name]
            if (weight_total == 0 && code_sum != 0) malformed("no PC time outside primitives")
            for (name in function_name) {
                game_code[name] = weight_total > 0 ? code_sum * weight[name] / weight_total : 0
                total[name] = in_primitives[name] + game_code[name]
                group_primitives[group_of[name]] += in_primitives[name]; group_code[group_of[name]] += game_code[name]
                lines_sum += total[name]
            }
            for (i = 1; i <= group_count; i++) {
                g = groups[i]; t = group_primitives[g] + group_code[g]
                if (mode == "profile") print "group " g " in-primitives " per_frame(group_primitives[g]) " game-code " per_frame(group_code[g]) " total " per_frame(t) " share " share(t)
                else print "group " g " total " per_frame(t) " share " share(t)
            }
            n = 0
            for (name in function_name) order[++n] = name
            for (i = 1; i <= n; i++) for (j = i + 1; j <= n; j++)
                if (total[order[j]] > total[order[i]] || total[order[j]] == total[order[i]] && order[j] < order[i]) { t = order[i]; order[i] = order[j]; order[j] = t }
            for (i = 1; i <= n; i++) {
                name = order[i]
                if (mode == "profile") print "function " name " group " group_of[name] " in-primitives " per_frame(in_primitives[name]) " game-code " per_frame(game_code[name]) " share " share(total[name])
                else print "function " name " group " group_of[name] " total " per_frame(total[name]) " share " share(total[name])
            }
            split_update()
            print "submit " per_frame(submit_sum) " share " share(submit_sum)
            lines_sum += submit_sum
            residual = lines_sum - measured
            floor = 3 * frames; if (measured / 100 > floor) floor = measured / 100
            if (mode == "profile") {
                verdict = (residual <= floor && -residual <= floor) ? "ok" : "failed"
                print "adds-up " verdict " sum " per_frame(lines_sum) " measured " per_frame(measured) " residual " per_frame(residual) " noise-floor " per_frame(floor)
            } else print "adds-up n/a no-run sum " per_frame(lines_sum) " measured " per_frame(measured)
        }'
}

profile() {
    [ $# -eq 7 ] || fail_usage "$usage"
    overhead=$1
    case $overhead in
        '' | *[!0-9.]* | *.*.* | .) fail_usage "overhead is not a number: '$overhead'" ;;
    esac
    require_file "group table" "$2"
    require_file "plain tty" "$4"
    require_file "breakdown tty" "$5"
    check_profiles "$6"
    check_profiles "$7"
    window=$(window_of "$3") || exit 1
    for file in $(echo "$6,$7" | tr ',' ' '); do
        if ! check=$(sh "$here/frame_profile_check.sh" breakdown "$file" "$5" "$3"); then
            echo "$check" | head -n 1
            exit 2
        fi
    done
    set -- "$@" $window
    {
        sed 's/^/G /' "$2"
        psx_lines P "$4" | grep '^P '
        psx_lines B "$5"
        profile_lines A "$6"
        profile_lines K "$7"
    } | join_awk profile "$8" "$9" "$1"
}

no_run() {
    [ $# -eq 7 ] || fail_usage "$usage"
    case $2 in
        '' | *[!0-9]*) fail_usage "budget is not a count of hblanks: '$2'" ;;
    esac
    require_file "judge frame table" "$3"
    require_file "situation table" "$4"
    require_file "group table" "$5"
    check_profiles "$6"
    check_profiles "$7"
    {
        sed 's/^/G /' "$5"
        sed 's/^/J /' "$3"
        sed 's/^/S /' "$4"
        profile_lines A "$6"
        profile_lines K "$7"
    } | join_awk no-run 0 0 0 "$1" "$2"
}

[ $# -ge 1 ] || fail_usage "$usage"
mode=$1
shift
case $mode in
    overhead) overhead "$@" ;;
    profile) profile "$@" ;;
    no-run) no_run "$@" ;;
    *) fail_usage "$usage" ;;
esac
