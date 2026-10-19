#!/bin/sh
# usage: plan0_files_check.sh list
# list: checks docs/love-port/plan-0-files.txt against the tree, the Makefile
# and .github/workflows/, prints each error on stderr, then the number of
# delete entries still present per tag; exit 1 on any error.
# The tracked files are git ls-files plus the untracked files git does not
# ignore, minus docs/prd/: a file not committed yet is still listed.

set -u
LC_ALL=C
export LC_ALL

cd "$(dirname "$0")/.." || exit 2

LIST=docs/love-port/plan-0-files.txt

if [ "${1:-}" != list ] || [ $# -ne 1 ]; then
    echo "usage: plan0_files_check.sh list" >&2
    exit 2
fi
if [ ! -f "$LIST" ]; then
    echo "plan0-files: $LIST missing" >&2
    exit 1
fi

work=$(mktemp -d "${TMPDIR:-/tmp}/plan0_files.XXXXXX") || exit 2
trap 'rm -rf "$work"' EXIT

status=0
fail() {
    echo "plan0-files: $*" >&2
    status=1
}

# Writes <kind>.txt as `<mark> <tag> <name>` lines; prints the format errors.
awk -v work="$work" '
    /^#/ || /^$/ { next }
    {
        where = "line " NR ": "
        if ($0 !~ /^[^ \t]+( [^ \t]+)+$/) { print where "fields not separated by single spaces"; next }
        if (NF < 4) { print where "fewer than 4 fields"; next }
        kind = $1; mark = $2; tag = $3; name = $4
        if (kind != "file" && kind != "target" && kind != "ci") { print where "unknown kind " kind; next }
        if (mark == "keep") {
            if (tag != "build" && tag != "plan") { print where "tag " tag " is not build or plan"; next }
        } else if (mark == "delete") {
            if (tag != "psx" && tag != "raylib" && tag != "tool") { print where "tag " tag " is not psx, raylib or tool"; next }
        } else { print where "unknown mark " mark; next }
        if (NF > 4 && kind != "target") { print where "arguments on a " kind " line"; next }
        for (i = 5; i <= NF; i++)
            if ($i !~ /^[A-Za-z_][A-Za-z0-9_]*=/) { print where "argument " $i " is not KEY=VALUE"; next }
        if (name in seen) { print where name " already on line " seen[name]; next }
        seen[name] = NR
        print mark, tag, name > (work "/" kind ".txt")
    }
' "$LIST" > "$work/format.txt"
while IFS= read -r line; do fail "$line"; done < "$work/format.txt"
for kind in file target ci; do
    touch "$work/$kind.txt"
done

# Files: every tracked file listed; a listed file that is not tracked is a
# delete line whose file a later step already removed.
git ls-files --cached --others --exclude-standard | grep -v '^docs/prd/' | sort -u > "$work/tracked.txt"
awk '{ print $3 }' "$work/file.txt" | sort > "$work/listed.txt"
comm -23 "$work/tracked.txt" "$work/listed.txt" > "$work/unlisted.txt"
while IFS= read -r path; do fail "$path: tracked, not listed"; done < "$work/unlisted.txt"
while read -r mark tag path; do
    [ "$mark" = keep ] && [ ! -f "$path" ] && fail "$path: marked keep, missing"
done < "$work/file.txt"

# Targets: every name a goal of make's database.
(
    unset MAKEFLAGS MFLAGS MAKELEVEL
    make -pn MAKE_LOCK_HELD=1 help 2> /dev/null
) | awk -F: '/^[A-Za-z0-9_.\/-]+:([^=]|$)/ { print $1 }' | sort -u > "$work/goals.txt"
if [ ! -s "$work/goals.txt" ]; then
    fail "make -pn printed no goal"
fi
while read -r mark tag goal; do
    grep -qxF "$goal" "$work/goals.txt" || fail "target $goal: not a Makefile goal"
done < "$work/target.txt"

# CI: `<workflow>:<job> <goal>` per make goal of a run: step, `<workflow>:<job> -`
# for a job with none.
for workflow in .github/workflows/*.yml; do
    [ -f "$workflow" ] || continue
    awk -v file="${workflow##*/}" '
        /^[^ #]/ { in_jobs = ($0 ~ /^jobs:/); next }
        in_jobs && /^  [A-Za-z0-9_-]+:[ \t]*$/ {
            job = $1; sub(/:$/, "", job)
            print file ":" job, "-"
            next
        }
        in_jobs && job != "" && /run:/ {
            n = split($0, words, /[ \t;&|]+/)
            for (i = 1; i < n; i++)
                if (words[i] == "make")
                    for (j = i + 1; j <= n && words[j] !~ /^(-|.*=)/ && words[j] != ""; j++)
                        print file ":" job, words[j]
        }
    ' "$workflow"
done | sort -u > "$work/ci_goals.txt"
awk '{ print $1 }' "$work/ci_goals.txt" | sort -u > "$work/jobs.txt"
awk '{ print $3 }' "$work/ci.txt" | sort > "$work/listed_jobs.txt"
comm -23 "$work/jobs.txt" "$work/listed_jobs.txt" > "$work/unlisted_jobs.txt"
while IFS= read -r job; do fail "ci $job: in a workflow, not listed"; done < "$work/unlisted_jobs.txt"
while read -r mark tag job; do
    file=${job%%:*}
    if [ -f ".github/workflows/$file" ]; then
        grep -qxF "$job" "$work/jobs.txt" || fail "ci $job: no such job in .github/workflows/$file"
    elif [ "$mark" = keep ]; then
        fail "ci $job: marked keep, .github/workflows/$file missing"
    fi
    [ "$mark" = keep ] || continue
    awk -v job="$job" '$1 == job && $2 != "-" { print $2 }' "$work/ci_goals.txt" > "$work/called.txt"
    while IFS= read -r goal; do
        grep -qx "keep [a-z]* $goal" "$work/target.txt" || fail "ci $job: calls $goal, not a target keep line"
    done < "$work/called.txt"
done < "$work/ci.txt"

for tag in psx raylib tool; do
    present=0
    while read -r mark line_tag name; do
        [ "$mark" = delete ] && [ "$line_tag" = "$tag" ] && [ -f "$name" ] && present=$((present + 1))
    done < "$work/file.txt"
    while read -r mark line_tag name; do
        [ "$mark" = delete ] && [ "$line_tag" = "$tag" ] && grep -qxF "$name" "$work/goals.txt" && present=$((present + 1))
    done < "$work/target.txt"
    while read -r mark line_tag name; do
        [ "$mark" = delete ] && [ "$line_tag" = "$tag" ] && grep -qxF "$name" "$work/jobs.txt" && present=$((present + 1))
    done < "$work/ci.txt"
    echo "plan0-files: delete $tag: $present still present"
done

if [ $status -eq 0 ]; then
    echo "plan0-files: list ok"
fi
exit $status
