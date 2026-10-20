#!/bin/sh
# usage: plan0_files_check.sh list|build
# list: checks docs/love-port/plan-0-files.txt against the tree, the Makefile
# and .github/workflows/, prints each error on stderr, then the number of
# delete entries still present per tag; exit 1 on any error.
# The tracked files are git ls-files plus the untracked files git does not
# ignore, minus docs/prd/: a file not committed yet is still listed.
# build: list, then runs the kept goals in tmp/plan0_build, a copy of the kept
# files only (logs in tmp/plan0_build_logs), checks the copy still matches the
# originals and that every keep build file is reached by a kept goal.

set -u
LC_ALL=C
export LC_ALL

cd "$(dirname "$0")/.." || exit 2

LIST=docs/love-port/plan-0-files.txt

MODE=${1:-}
if [ $# -ne 1 ] || { [ "$MODE" != list ] && [ "$MODE" != build ]; }; then
    echo "usage: plan0_files_check.sh list|build" >&2
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
        if (kind == "target") {
            args = name
            for (i = 5; i <= NF; i++) args = args " " $i
            print args > (work "/target_args.txt")
        }
    }
' "$LIST" > "$work/format.txt"
while IFS= read -r line; do fail "$line"; done < "$work/format.txt"
for kind in file target ci target_args; do
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
) > "$work/database.txt"
awk -F: '/^[A-Za-z0-9_.\/-]+:([^=]|$)/ { print $1 }' "$work/database.txt" | sort -u > "$work/goals.txt"
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
if [ "$MODE" = list ] || [ $status -ne 0 ]; then
    exit $status
fi

# Not run in the copy: composites run as their kept parts, e2e-ref needs git,
# and the goals writing into the tree run last. state runs in both its forms.
COMPOSITE_GOALS="test test-all unit"
GIT_GOALS="e2e-ref"
WRITING_GOALS="e2e-baseline end-state-baseline sin-table"
STATE_EVENTS=tests/e2e/attract.json

top=$(pwd)
copy=tmp/plan0_build
logs=tmp/plan0_build_logs
rm -rf "$copy" "$logs"
mkdir -p "$copy" "$logs"
awk '$1 == "keep" { print $3 }' "$work/file.txt" | sort > "$work/kept.txt"
awk '$1 == "keep" && $2 == "build" { print $3 }' "$work/file.txt" | sort > "$work/kept_build.txt"
tar -cf - -T "$work/kept.txt" | (cd "$copy" && tar -xpf -) || {
    echo "plan0-files: copy into $copy failed" >&2
    exit 1
}

# float-grep refuses an exclusion of a missing file: the copy drops the lines
# naming a delete file, as steps 2 to 4 will, and is exempt from the identity
# check for that file alone.
PRUNED_EXCLUSIONS=tools/float_grep_exclusions.txt
awk '$1 == "delete" { print $3 }' "$work/file.txt" > "$work/deleted.txt"
awk -F'|' 'NR == FNR { deleted[$0] = 1; next }
    { path = $1; gsub(/^[ \t]+|[ \t]+$/, "", path) }
    !(path in deleted)' "$work/deleted.txt" "$PRUNED_EXCLUSIONS" > "$copy/$PRUNED_EXCLUSIONS"
echo "plan0-files: $PRUNED_EXCLUSIONS pruned of its delete files in the copy, left out of the identity check"

# The copy lives inside this repository: the ceiling keeps git from finding it.
in_copy() {
    (
        unset MAKEFLAGS MFLAGS MAKELEVEL
        GIT_CEILING_DIRECTORIES=$top/tmp
        export GIT_CEILING_DIRECTORIES
        cd "$copy" && make --no-print-directory "$@"
    )
}

listed_in() {
    case " $2 " in
        *" $1 "*) return 0 ;;
    esac
    return 1
}

# One make run per line, `<goal> [KEY=VALUE ...]`, in run order.
: > "$work/runs.txt"
: > "$work/last_runs.txt"
while read -r goal args; do
    grep -qx "keep [a-z]* $goal" "$work/target.txt" || continue
    listed_in "$goal" "$COMPOSITE_GOALS $GIT_GOALS" && continue
    if listed_in "$goal" "$WRITING_GOALS"; then
        echo "$goal${args:+ $args}" >> "$work/last_runs.txt"
        continue
    fi
    echo "$goal${args:+ $args}" >> "$work/runs.txt"
    [ "$goal" = state ] && echo "state EVENTS=$STATE_EVENTS" >> "$work/runs.txt"
done < "$work/target_args.txt"
sed -n 's/^UNIT_TESTS :*= //p' "$work/database.txt" | tr ' ' '\n' | while read -r test; do
    [ -n "$test" ] || continue
    grep -qx "keep [a-z]* tests/$test.c" "$work/file.txt" && echo "unit-run-$test"
done >> "$work/runs.txt"
cat "$work/last_runs.txt" >> "$work/runs.txt"

runs=0
failed=0
while read -r run; do
    runs=$((runs + 1))
    log="$logs/$runs-${run%% *}.log"
    if ! in_copy $run > "$log" 2>&1 < /dev/null; then
        tail -n 20 "$log" >&2
        fail "goal $run: failed in the copy, log in $log"
        failed=$((failed + 1))
    fi
done < "$work/runs.txt"
echo "plan0-files: $runs goal runs in $copy, $failed failed"

while IFS= read -r path; do
    [ "$path" = "$PRUNED_EXCLUSIONS" ] && continue
    cmp -s "$path" "$copy/$path" || fail "$path: differs in the copy after the goals"
done < "$work/kept.txt"
(cd "$copy" && find . -type f ! -path './tmp/*') | sed 's|^\./||' | sort > "$work/copy_files.txt"
comm -23 "$work/copy_files.txt" "$work/kept.txt" > "$work/new_files.txt"
git check-ignore --stdin < "$work/new_files.txt" | sort > "$work/ignored.txt"
comm -23 "$work/new_files.txt" "$work/ignored.txt" > "$work/written.txt"
while IFS= read -r path; do fail "$path: written into the copy by a goal, not listed"; done < "$work/written.txt"

# Reach: every file make considers for the kept goals, git ones included, and
# the run-time reads make cannot see: the paths their commands name, those a
# considered C source names in a string, and those the e2e replays record.
grep -v ' ' "$work/runs.txt" > "$work/plain_runs.txt"
echo "$GIT_GOALS" | tr ' ' '\n' >> "$work/plain_runs.txt"
dry_runs() {
    in_copy "$@" MAKE_LOCK_HELD=1 $(cat "$work/plain_runs.txt") < /dev/null 2> /dev/null
    grep ' ' "$work/runs.txt" | while read -r run; do
        in_copy "$@" MAKE_LOCK_HELD=1 $run < /dev/null 2> /dev/null
    done
}
dry_runs -n -d | sed -n "s/^ *Considering target file [\`']\(.*\)'\.\$/\1/p" | sed 's|^\./||' | sort -u > "$work/considered.txt"
{
    cat "$work/considered.txt"
    dry_runs -n | tr -s ' \t;|&()"=<>' '\n'
    grep '\.c$' "$work/considered.txt" | while IFS= read -r source; do
        [ -f "$copy/$source" ] && grep -ohE '"[A-Za-z0-9_./-]+"' "$copy/$source" | tr -d '"'
    done
    cat "$copy"/tmp/frames/tests/e2e/*.d 2> /dev/null | tr -s ' \t' '\n\n' | sed 's/:$//'
} | sed 's|^\./||' | sort -u > "$work/reached.txt"
comm -23 "$work/kept_build.txt" "$work/reached.txt" > "$work/unreached.txt"
while IFS= read -r path; do fail "$path: keep build, reached by no kept goal"; done < "$work/unreached.txt"

if [ $status -eq 0 ]; then
    echo "plan0-files: build ok"
fi
exit $status
