#!/bin/sh
# usage: lever_estimate.sh <judge-frames.txt> <spike-frames.txt> <gain-table> measured|estimated [<option>=<value>...]
#   options: allocation=sequential|independent (default sequential)
#            haircut=<pct> (default 0: every gain divided by 1 + pct/100)
#            alone=<lever id> (that lever's rows only, spreading kept)
#            run=<name> (the in-game@<name> rows apply; required when the table has any)
# Per judge frame, the frame after the levers of the gain table (rows of the
# end's kind; the measured end reads measured rows only), then per spike
# frame its class's work spread over the frames that follow, as
# frame_profile_check.sh spread does. Gain table rows:
#   budget <hblanks>
#   range <name> <first>-<last>
#   class <label> <spikes path>
#   override <frame> <path> <share%>
#   ratio default|<label> <child>=<rho>[,...]
#   lever <id> measured|estimated draw|update|submit work|per-call|scale <group>/<alt> <scope> <gain%>|abs:<hblanks>
#   spread <id> measured|estimated <label>|* responsible|responsible+save:<share%>
# Scopes, most specific first: range:<name>, class:<label>, class:*,
# ordinary (not a spike frame), in-game@<run>, in-game (outside every range),
# all; one lever's most specific matching row counts on each part. abs: is
# allowed on update with a class scope only: hblanks off the update outside
# the spreadable work, rounded half up, floored at 0. A "#" that starts a word
# comments out the rest of a gain table line; one inside a path does not.
# Composition per part: inside a layer, rows summed within an alt, the
# largest alt of a group, groups summed, capped at 100; across layers
# part x (1 - work)(1 - per-call)(1 - scale), rounded half up.
# A spike frame's work is ceil(share x rho x update) (rho: the class's ratio
# for the path's cart_update60 child, else default's, else 1; the portion
# capped at 1), shrunk in proportion to update's relative gains; with a
# responsible+save row, ceil(share x update) more, capped at the rest. A
# spike frame without a spread row fits only on its own.
# independent: each spread counts the frames that follow at their own total
# (frame_profile_check.sh spread). sequential: spike frames in order consume
# one shared room array, the room of a spike frame counted without its
# spread work; a spread that does not fit consumes nothing.
# Output, frame lines in frame order:
#   frame <f> before <t> after <t'> update <u'> draw <d'> submit <s'> fits|over <x>
#   frame <f> before <t> class <label> work <w> spread <a> frames <n>|does-not-fit [overlaps <f>,...]
#   class <label> work <min>-<median>-<max> frames <min>-<median>-<max>|- does-not-fit <k>
#   summary end <end> allocation <a> haircut <h> run <run>|- frames <n> over <k> worst <f> <t'> mean <m> saved <h> spikes <s> does-not-fit <d>
#   lever <id> alone saved <hblanks> frames-fitting <n>    (without alone=, one per lever id)
# over, worst and mean are over the frames that are not spike frames; saved
# sums before - after over every frame, a spike frame's after taken before
# spreading; frames-fitting counts the frames that fit and the spike frames
# whose spread fits. Exit 0 whatever the verdict, 2 on a malformed input
# (naming the file and line), 1 on a usage error.

set -u
LC_ALL=C
export LC_ALL

usage="usage: lever_estimate.sh <judge-frames.txt> <spike-frames.txt> <gain-table> measured|estimated [allocation=sequential|independent] [haircut=<pct>] [alone=<lever id>] [run=<name>]"

fail_usage() {
    echo "lever-estimate: $1" >&2
    exit 1
}

[ $# -ge 4 ] || fail_usage "$usage"
judge=$1
spikes=$2
gains=$3
end=$4
shift 4
[ -f "$judge" ] || fail_usage "missing judge table: $judge"
[ -f "$spikes" ] || fail_usage "missing spike frames: $spikes"
[ -f "$gains" ] || fail_usage "missing gain table: $gains"
case $end in
    measured | estimated) ;;
    *) fail_usage "end is not measured or estimated: '$end'" ;;
esac
allocation=sequential
haircut=0
alone=
run=
for option in "$@"; do
    case $option in
        allocation=sequential | allocation=independent) allocation=${option#*=} ;;
        haircut=*)
            haircut=${option#*=}
            echo "$haircut" | grep -Eq '^[0-9]+(\.[0-9]+)?$' || fail_usage "haircut is not a percentage: '$haircut'"
            ;;
        alone=?*) alone=${option#*=} ;;
        run=?*) run=${option#*=} ;;
        *) fail_usage "unknown option: '$option'" ;;
    esac
done

awk -v end="$end" -v allocation="$allocation" -v haircut="$haircut" -v alone="$alone" -v run="$run" \
    -v judge_name="$judge" -v spikes_name="$spikes" -v gains_name="$gains" '
    BEGIN {
        cap = 600
        level["range"] = 7; level["class"] = 6; level["class:*"] = 5; level["ordinary"] = 4
        level["in-game@"] = 3; level["in-game"] = 2; level["all"] = 1
    }
    function malformed_at(name, line, reason) {
        print "lever-estimate: malformed " name ": line " line ": " reason > "/dev/stderr"
        failed = 2
        exit 2
    }
    function malformed(reason) {
        malformed_at(FILENAME, FNR, reason)
    }
    function usage_error(reason) {
        print "lever-estimate: " reason > "/dev/stderr"
        failed = 1
        exit 1
    }
    function is_number(text) {
        return text ~ /^[0-9]+(\.[0-9]+)?$/
    }
    function ceil_work(x,    y) {
        y = int(x - 1e-9) + (x - 1e-9 > int(x - 1e-9) ? 1 : 0)
        return y < 0 ? 0 : y
    }
    function median(values, n,    i, j, t) {
        for (i = 1; i <= n; i++) for (j = i + 1; j <= n; j++) if (values[j] < values[i]) { t = values[i]; values[i] = values[j]; values[j] = t }
        return n % 2 ? values[(n + 1) / 2] : (values[n / 2] + values[n / 2 + 1]) / 2
    }
    file == 1 && FNR == 1 {
        if ($1 != "frame" || $2 != "update" || $3 != "draw" || $4 != "submit") malformed("no judge header")
        next
    }
    file == 1 {
        if (NF < 4 || $1 != frame_count || $2 !~ /^[0-9]+$/ || $3 !~ /^[0-9]+$/ || $4 !~ /^[0-9]+$/)
            malformed("not frame " frame_count)
        update[$1] = $2 + 0; draw[$1] = $3 + 0; submit[$1] = $4 + 0
        frame_count++
        next
    }
    file == 2 && $1 == "frame" {
        if (NF < 10 || $2 !~ /^[0-9]+$/ || $3 != "update" || $5 != "pc-update-ns" || $7 != "path" || $9 != "share" \
            || $10 !~ /^[0-9]+(\.[0-9]+)?%$/)
            malformed("not a spikes frame line")
        f = $2 + 0
        if (f >= frame_count) malformed("frame " f " is not in the judge table")
        if ($4 + 0 != update[f]) malformed("frame " f " update " $4 ", not " update[f] " as the judge table")
        if (f in is_spike) malformed("frame " f " twice")
        is_spike[f] = 1; spike_line[f] = FNR; spike_path[f] = $8
        share = $10; sub(/%$/, "", share); spike_share[f] = share + 0
        next
    }
    file == 2 { next }
    file == 3 {
        sub(/(^|[ \t])#.*/, "")
        $0 = $0
        if (NF == 0) next
        if ($1 == "budget") {
            if (NF != 2 || $2 !~ /^[0-9]+$/) malformed("not budget <hblanks>")
            if (budget != "") malformed("a second budget")
            budget = $2 + 0
        } else if ($1 == "range") {
            if (NF != 3 || $3 !~ /^[0-9]+-[0-9]+$/) malformed("not range <name> <first>-<last>")
            split($3, ends, "-")
            if (ends[1] + 0 > ends[2] + 0) malformed("range " $2 " backwards")
            if ($2 in range_first) malformed("range " $2 " twice")
            range_first[$2] = ends[1] + 0; range_last[$2] = ends[2] + 0; range_names[++range_count] = $2
        } else if ($1 == "class") {
            if (NF != 3) malformed("not class <label> <path>")
            if ($2 in class_path) malformed("class " $2 " twice")
            if ($3 in class_of_path) malformed("path " $3 " in two classes")
            class_path[$2] = $3; class_of_path[$3] = $2
        } else if ($1 == "override") {
            if (NF != 4 || $2 !~ /^[0-9]+$/ || !is_number($4)) malformed("not override <frame> <path> <share%>")
            if (!(($2 + 0) in is_spike)) malformed("frame " $2 " is not in the spike frames")
            if (($2 + 0) in override_path) malformed("frame " $2 " overridden twice")
            override_path[$2 + 0] = $3; override_share[$2 + 0] = $4 + 0
        } else if ($1 == "ratio") {
            if (NF != 3 || $3 !~ /^[A-Za-z0-9_]+=[0-9]+(\.[0-9]+)?(,[A-Za-z0-9_]+=[0-9]+(\.[0-9]+)?)*$/)
                malformed("not ratio <scope> <child>=<rho>[,...]")
            if ($2 != "default") reference[++reference_count] = $2 SUBSEP FNR
            n = split($3, specs, ",")
            for (i = 1; i <= n; i++) {
                split(specs[i], kv, "=")
                if (($2, kv[1]) in ratio_of) malformed("ratio " $2 " " kv[1] " twice")
                ratio_of[$2, kv[1]] = kv[2] + 0
            }
        } else if ($1 == "lever") {
            read_lever()
        } else if ($1 == "spread") {
            if (NF != 5 || ($3 != "measured" && $3 != "estimated") || $5 !~ /^responsible(\+save:[0-9]+(\.[0-9]+)?)?$/)
                malformed("not spread <id> <kind> <label>|* responsible|responsible+save:<share%>")
            if ($4 != "*") reference[++reference_count] = $4 SUBSEP FNR
            if (($4, "spread") in permission_line) malformed("a second spread row for " $4)
            permission_line[$4, "spread"] = FNR
            if ($5 ~ /save/) { save = $5; sub(/.*:/, "", save); save_share[$4] = save + 0 }
            permitted[$4] = 1
        } else {
            malformed("unknown row " $1)
        }
        next
    }
    function read_lever(    scope, type, gain) {
        if (NF != 8) malformed("not lever <id> <kind> <part> <layer> <group>/<alt> <scope> <gain>")
        if ($3 != "measured" && $3 != "estimated") malformed("kind " $3 " is not measured or estimated")
        if ($4 != "draw" && $4 != "update" && $4 != "submit") malformed("part " $4 " is not draw, update or submit")
        if ($5 != "work" && $5 != "per-call" && $5 != "scale") malformed("layer " $5 " is not work, per-call or scale")
        if ($6 !~ /^[^\/]+\/[^\/]+$/) malformed("group " $6 " is not <group>/<alt>")
        scope = $7
        if (scope == "all" || scope == "in-game" || scope == "ordinary" || scope == "class:*") type = scope
        else if (scope ~ /^in-game@./) type = "in-game@"
        else if (scope ~ /^class:./) type = "class"
        else if (scope ~ /^range:./) type = "range"
        else malformed("scope " scope " is unknown")
        gain = $8
        if (gain ~ /^abs:/) {
            sub(/^abs:/, "", gain)
            if (!is_number(gain)) malformed("gain " $8 " is not abs:<hblanks>")
            if ($4 != "update" || (type != "class" && type != "class:*")) malformed("abs: outside an update class scope")
            row_abs[row_count + 1] = 1
        } else if (!is_number(gain) || gain + 0 > 100) {
            malformed("gain " $8 " is not a percentage from 0 to 100")
        }
        if (($2, $4, scope) in row_of) malformed("lever " $2 " " $4 " " scope " twice")
        row_count++
        row_of[$2, $4, scope] = row_count
        row_line[row_count] = FNR; row_id[row_count] = $2; row_kind[row_count] = $3; row_part[row_count] = $4
        row_layer[row_count] = $5; split($6, ga, "/"); row_group[row_count] = ga[1]; row_alt[row_count] = ga[2]
        row_type[row_count] = type; row_arg[row_count] = substr(scope, index(scope, type == "in-game@" ? "@" : ":") + 1)
        row_gain[row_count] = gain / (1 + haircut / 100)
        if (type == "in-game@") has_run[row_arg[row_count]] = 1
        if (type == "class" || type == "range") reference[++reference_count] = (type == "range" ? "range:" : "") row_arg[row_count] SUBSEP FNR
        if (!($2 in lever_index)) { lever_index[$2] = ++lever_count; lever_ids[lever_count] = $2 }
    }
    # Fills fac[part] (the kept fraction) and absolute[part] for one frame signature.
    function gains_of(range_name, label, only,    i, key, best, p, l, g, a, alt_sum, group_best, layer_sum, abs_alt, abs_group, abs_sum, lv) {
        for (i = 1; i <= row_count; i++) {
            if (only != "" && row_id[i] != only) continue
            if (end == "measured" && row_kind[i] != "measured") continue
            lv = 0
            if (row_type[i] == "range") { if (range_name == row_arg[i]) lv = level["range"] }
            else if (row_type[i] == "class") { if (label == row_arg[i]) lv = level["class"] }
            else if (row_type[i] == "class:*") { if (label != "") lv = level["class:*"] }
            else if (row_type[i] == "ordinary") { if (label == "") lv = level["ordinary"] }
            else if (row_type[i] == "in-game@") { if (range_name == "" && row_arg[i] == run) lv = level["in-game@"] }
            else if (row_type[i] == "in-game") { if (range_name == "") lv = level["in-game"] }
            else lv = level["all"]
            if (lv == 0) continue
            key = row_id[i] SUBSEP row_part[i]
            if (!(key in best) || lv > best_level[key]) { best[key] = i; best_level[key] = lv }
        }
        for (key in best) {
            i = best[key]; p = row_part[i]; l = (i in row_abs) ? "abs" : row_layer[i]
            alt_sum[p, l, row_group[i], row_alt[i]] += row_gain[i]
        }
        for (key in alt_sum) {
            split(key, k, SUBSEP)
            if (!((k[1], k[2], k[3]) in group_best) || alt_sum[key] > group_best[k[1], k[2], k[3]]) group_best[k[1], k[2], k[3]] = alt_sum[key]
        }
        for (key in group_best) { split(key, k, SUBSEP); layer_sum[k[1], k[2]] += group_best[key] }
        for (p in parts) {
            fac[p] = 1
            for (l = 1; l <= 3; l++) {
                g = layer_sum[p, layers[l]] + 0
                if (g > 100) g = 100
                fac[p] = fac[p] * (100 - g)
            }
            absolute[p] = layer_sum[p, "abs"] + 0
        }
        delete best; delete best_level
    }
    function kept(value, part) {
        return int(value * fac[part] / 1000000 + 0.5 + 1e-9)
    }
    # One estimate with every row (only == "") or one lever id: fills after[], spread[], fits[],
    # frames_used[], work_after[]; returns saved hblanks, sets fitting.
    function estimate(only,    f, g, sig, u2, d2, s2, rest, w, need, total, frames, saved, take) {
        delete sig_done
        saved = 0; fitting = 0
        for (f = 0; f < frame_count; f++) {
            sig = frame_range[f] SUBSEP frame_label[f]
            if (!(sig in sig_done)) {
                gains_of(frame_range[f], frame_label[f], only)
                for (p in parts) { sig_fac[sig, p] = fac[p]; sig_abs[sig, p] = absolute[p] }
                sig_done[sig] = 1
            }
            for (p in parts) { fac[p] = sig_fac[sig, p]; absolute[p] = sig_abs[sig, p] }
            u2 = kept(update[f], "update"); d2 = kept(draw[f], "draw"); s2 = kept(submit[f], "submit")
            spread[f] = 0
            if (f in is_spike) {
                w = update[f] > 0 ? ceil_work(work[f] * u2 / update[f]) : 0
                rest = int(u2 - w - absolute["update"] + 0.5 + 1e-9)
                if (rest < 0) rest = 0
                work_after[f] = w
                if (frame_label[f] in permitted || "*" in permitted) {
                    spread[f] = w
                    if (frame_label[f] in save_share) save = save_share[frame_label[f]]
                    else if ("*" in save_share) save = save_share["*"]
                    else save = -1
                    if (save >= 0) { save = ceil_work(save * u2 / 100); spread[f] += save < rest ? save : rest }
                }
                up[f] = w + rest
            } else {
                up[f] = u2
            }
            dp[f] = d2; sp[f] = s2
            after[f] = up[f] + d2 + s2
            saved += before[f] - after[f]
            room[f] = budget - after[f] + spread[f]
            if (room[f] < 0) room[f] = 0
        }
        for (s = 1; s <= spike_count; s++) {
            f = spike_frames[s]
            if (!(frame_label[f] in permitted || "*" in permitted)) {
                fits[f] = after[f] <= budget; frames_used[f] = 1
                continue
            }
            need = spread[f]
            total = room[f]; frames = 1
            while (total < need && frames < cap && f + frames < frame_count) {
                g = f + frames
                total += allocation == "independent" ? (budget - after[g] > 0 ? budget - after[g] : 0) : room[g]
                frames++
            }
            fits[f] = total >= need; frames_used[f] = frames
            if (fits[f] && allocation == "sequential") {
                for (g = f; g < f + frames && need > 0; g++) { take = room[g] < need ? room[g] : need; room[g] -= take; need -= take }
            }
        }
        for (f = 0; f < frame_count; f++) {
            if (f in is_spike) fitting += fits[f]
            else fitting += after[f] <= budget
        }
        return saved
    }
    END {
        if (failed) exit failed
        if (file < 3) usage_error("no gain table read")
        if (budget == "") malformed_at(gains_name, FNR, "no budget row")
        for (i = 1; i <= reference_count; i++) {
            split(reference[i], r, SUBSEP)
            if (r[1] ~ /^range:/) { name = substr(r[1], 7); if (!(name in range_first)) malformed_at(gains_name, r[2], "range " name " is not defined") }
            else if (!(r[1] in class_path)) malformed_at(gains_name, r[2], "class " r[1] " is not defined")
        }
        if (alone != "" && !(alone in lever_index)) usage_error("alone=" alone ": no lever row has that id")
        if (run != "" && !(run in has_run)) usage_error("run=" run ": no in-game@" run " row in " gains_name)
        if (run == "") for (name in has_run) usage_error("the gain table has in-game@<run> rows: pass run=<name>")
        parts["update"] = 1; parts["draw"] = 1; parts["submit"] = 1
        layers[1] = "work"; layers[2] = "per-call"; layers[3] = "scale"
        for (f = 0; f < frame_count; f++) {
            before[f] = update[f] + draw[f] + submit[f]
            frame_range[f] = ""
            for (i = 1; i <= range_count; i++) if (f >= range_first[range_names[i]] && f <= range_last[range_names[i]]) frame_range[f] = range_names[i]
            frame_label[f] = ""
            if (!(f in is_spike)) continue
            spike_frames[++spike_count] = f
            if (!(spike_path[f] in class_of_path))
                malformed_at(spikes_name, spike_line[f], "frame " f " path " spike_path[f] " has no class row in " gains_name)
            frame_label[f] = class_of_path[spike_path[f]]
            path = (f in override_path) ? override_path[f] : spike_path[f]
            share = (f in override_share) ? override_share[f] : spike_share[f]
            child = ""
            if (split(path, steps, "/") > 1) { child = steps[2]; sub(/#.*/, "", child) }
            if ((frame_label[f], child) in ratio_of) rho = ratio_of[frame_label[f], child]
            else if (("default", child) in ratio_of) rho = ratio_of["default", child]
            else rho = 1
            portion = share * rho / 100
            if (portion > 1) portion = 1
            work[f] = ceil_work(portion * update[f])
        }
        saved = estimate(alone)
        worst = -1; over = 0; ordinary = 0; sum = 0; misses = 0
        for (f = 0; f < frame_count; f++) {
            if (!(f in is_spike)) {
                line = sprintf("frame %d before %d after %d update %d draw %d submit %d %s", f, before[f], after[f], up[f], dp[f], sp[f],
                    after[f] <= budget ? "fits" : "over " after[f] - budget)
                print line
                ordinary++; sum += after[f]; over += after[f] > budget
                if (worst < 0 || after[f] > after[worst]) worst = f
                continue
            }
            line = sprintf("frame %d before %d class %s work %d spread %d frames %s", f, before[f], frame_label[f], work_after[f], spread[f],
                fits[f] ? frames_used[f] : "does-not-fit")
            if (fits[f]) {
                overlaps = ""
                for (g = f + 1; g < f + frames_used[f]; g++) if (g in is_spike) overlaps = overlaps (overlaps == "" ? "" : ",") g
                if (overlaps != "") line = line " overlaps " overlaps
            }
            print line
            label = frame_label[f]
            if (!(label in class_index)) { class_index[label] = ++class_count; class_labels[class_count] = label }
            c = class_index[label]
            class_works[c, ++class_size[c]] = work_after[f]
            if (fits[f]) class_fits[c, ++class_fit_count[c]] = frames_used[f]
            else { class_misses[c]++; misses++ }
        }
        for (c = 1; c <= class_count; c++) {
            m = class_size[c]
            for (j = 1; j <= m; j++) v[j] = class_works[c, j]
            low = v[1]; high = v[1]
            for (j = 2; j <= m; j++) { if (v[j] < low) low = v[j]; if (v[j] > high) high = v[j] }
            work_range = low "-" median(v, m) "-" high
            m = class_fit_count[c] + 0
            if (m == 0) frame_range_text = "-"
            else {
                for (j = 1; j <= m; j++) v[j] = class_fits[c, j]
                low = v[1]; high = v[1]
                for (j = 2; j <= m; j++) { if (v[j] < low) low = v[j]; if (v[j] > high) high = v[j] }
                frame_range_text = low "-" median(v, m) "-" high
            }
            print "class " class_labels[c] " work " work_range " frames " frame_range_text " does-not-fit " class_misses[c] + 0
        }
        printf "summary end %s allocation %s haircut %s run %s frames %d over %d worst %s mean %s saved %d spikes %d does-not-fit %d\n", \
            end, allocation, haircut + 0, run == "" ? "-" : run, frame_count, over, worst < 0 ? "- -" : worst " " after[worst], \
            ordinary ? sprintf("%.2f", sum / ordinary) : "-", saved, spike_count, misses
        if (alone != "") exit 0
        for (i = 1; i <= lever_count; i++) {
            saved = estimate(lever_ids[i])
            print "lever " lever_ids[i] " alone saved " saved " frames-fitting " fitting
        }
    }' file=1 "$judge" file=2 "$spikes" file=3 "$gains"
