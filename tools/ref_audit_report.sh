#!/bin/sh
# Reverse REF-cast audit: run the instrumented binaries, fold their
# per-site tallies together, and print one line per cast site.
#
# See docs/prd/triage/reverse-direction-ref-cast-audit.md,
# src/pico8_ref_audit.h and docs/audit-ref-cast-reverse.md. Invoked by
# `make ref-audit`, which builds the binaries this script runs.
#
# The two halves the PRD requires are both here. The STATIC half is the
# grep below: it enumerates every PICO8_REF_AS_LIST/_CONST occurrence in
# src/, which is now the single spelling of a "REF value read as a
# pico8_list_t*" conversion in this port, so the list cannot silently
# miss one the way a hand-kept inventory did before. The DYNAMIC half is
# the run: each instrumented binary records, per site, which type tag
# actually arrived. Joining the two is the point -- a site present in the
# grep but absent from the run is reported NOT COVERED, never "safe".
#
# Arguments: the replays to run, each optionally preceded by --from-title
# (`make ref-audit` passes the net, FRAMES_EVENTS). At least one.
set -e

if [ $# -eq 0 ]; then
    echo "usage: $0 [--from-title] <events.json> [[--from-title] <events.json>]..." >&2
    exit 2
fi

ROOT=$(dirname "$0")/..
cd "$ROOT"

OUT=tmp/ref_audit_raw.txt
REPORT=tmp/ref_audit_report.txt
rm -f "$OUT"
export PICO8_REF_AUDIT_OUT="$OUT"

run() {
    printf '  running %s\n' "$*"
    # A non-zero exit is reported but not fatal: this script measures
    # coverage, `make test` is what judges correctness. Do read it,
    # though -- a binary that died on abort() (a failed assert, a
    # sanitizer report) skipped its atexit dump entirely, so it
    # contributed NO sites at all and every site only it reaches will be
    # printed NOT COVERED. See src/pico8_ref_audit.h's own note.
    "$@" >/dev/null 2>&1 || printf '    (exit %s)\n' "$?"
}

echo "== running instrumented binaries =="
while [ $# -gt 0 ]; do
    if [ "$1" = --from-title ]; then
        [ $# -ge 2 ] || { echo "$0: --from-title without an events file" >&2; exit 2; }
        run ./tmp/pico8_replay_refaudit --from-title "$2"
        shift 2
    else
        run ./tmp/pico8_replay_refaudit "$1"
        shift
    fi
done
for bin in tmp/test_pico8_playability_refaudit \
           tmp/test_pico8_slipway_drag_refaudit \
           tmp/test_pico8_empty_planet_cleared_refaudit \
           tmp/test_pico8_planet_gh_row_lq_refaudit \
           tmp/test_pico8_data_init_refaudit \
           tmp/test_pico8_planet_kj_dispatch_refaudit \
           tmp/test_pico8_ga_export_panel_by_refaudit; do
    if [ -x "$bin" ]; then
        run "./$bin"
    fi
done

echo "== folding =="
awk -v raw="$OUT" '
BEGIN {
    # STATIC half: every cast site, straight out of the source.
    # Code lines only: the macro name also appears in the per-site
    # classification comments this audit wrote, and a comment is not a
    # site. Drop anything whose first non-blank character is a comment
    # marker.
    cmd = "grep -n \"PICO8_REF_AS_LIST\" src/*.c | grep -v \"pico8_ref_audit.h\" | grep -vE \":[0-9]+:[[:space:]]*(\\*|/\\*|//)\"";
    while ((cmd | getline line) > 0) {
        split(line, f, ":");
        file = f[1]; ln = f[2] + 0;
        # The macro name may sit on a different physical line from the
        # one __LINE__ reports for a multi-line invocation, so remember
        # the whole set and resolve observations to the nearest site at
        # or after the macro line.
        key = file ":" ln;
        static_site[key] = 1;
        sites[file] = sites[file] " " ln;
        nstatic++;
    }
    close(cmd);

    # DYNAMIC half.
    while ((getline line < raw) > 0) {
        n = split(line, f, "\t");
        if (f[1] == "MISMATCH") { if (!(line in mism)) { mism[line] = 1; nmism++; mismline[nmism] = line; } continue; }
        if (f[1] != "SITE") { if (f[1] == "OVERFLOW") print "WARNING: " line; continue; }
        file = f[2]; ln = f[3] + 0;
        if (file !~ /^src\//) file = "src/" file;
        key = resolve(file, ln);
        for (i = 4; i <= n; i++) {
            split(f[i], kv, "=");
            obs[key "|" kv[1]] += kv[2];
        }
        seen[key] = 1;
    }
    close(raw);

    covered = 0; uncovered = 0; suspicious = 0;
    printf "%-46s %-6s %s\n", "SITE", "LINE", "OBSERVED";
    m = split(sortedkeys(), ks, " ");
    for (i = 1; i <= m; i++) {
        key = ks[i];
        split(key, p, ":");
        if (!(key in seen)) {
            printf "%-46s %-6s NOT COVERED\n", p[1], p[2];
            uncovered++;
            continue;
        }
        covered++;
        line = "";
        for (t = 1; t <= 5; t++) {
            name = tagname(t);
            v = obs[key "|" name];
            if (v > 0) line = line sprintf("%s=%d ", name, v);
        }
        flag = "";
        if (obs[key "|ob_table"] > 0 || obs[key "|instance"] > 0 || obs[key "|unknown"] > 0) {
            flag = "   <<< NOT A LIST";
            suspicious++;
        }
        printf "%-46s %-6s %s%s\n", p[1], p[2], line, flag;
    }
    printf "\n%d cast site(s) in src/, %d reached, %d NOT COVERED, %d receiving something other than a list\n",
        nstatic, covered, uncovered, suspicious;
    # The accessor-entry channel: anything that reached a pico8_list_*/
    # pico8_ob_table_* primitive carrying the wrong tag, whatever route it
    # took there -- the check the cast-site list cannot make for itself.
    printf "\naccessor-entry channel (pico8_list_*/pico8_ob_table_* primitives): ";
    if (nmism == 0) {
        printf "no tag mismatch observed\n";
    } else {
        printf "%d distinct mismatch(es)\n", nmism;
        for (i = 1; i <= nmism; i++) print "  " mismline[i];
    }
}
function tagname(t) {
    if (t == 1) return "list";
    if (t == 2) return "ob_table";
    if (t == 3) return "instance";
    if (t == 4) return "unknown";
    return "null";
}
function resolve(file, ln,   n, a, i, best) {
    # __LINE__ for a multi-line macro invocation can land after the line
    # the macro name is on; map it back to the nearest static site at or
    # before it, within a few lines.
    n = split(sites[file], a, " ");
    best = ln;
    for (i = 1; i <= n; i++) {
        if (a[i] + 0 <= ln && ln - (a[i] + 0) <= 4) {
            if (best == ln || a[i] + 0 > best) best = a[i] + 0;
        }
    }
    return file ":" best;
}
function sortedkeys(   k, out, arr, n, i, j, tmp) {
    n = 0;
    for (k in static_site) { n++; arr[n] = k; }
    for (i = 1; i < n; i++)
        for (j = i + 1; j <= n; j++) {
            split(arr[i], x, ":"); split(arr[j], y, ":");
            if (x[1] > y[1] || (x[1] == y[1] && x[2] + 0 > y[2] + 0)) {
                tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
            }
        }
    out = "";
    for (i = 1; i <= n; i++) out = out arr[i] " ";
    return out;
}
' /dev/null | tee "$REPORT"

echo
echo "raw tallies: $OUT"
echo "report:      $REPORT"
