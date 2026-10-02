#!/bin/sh
# usage: minimize.sh <minimize tool> [--min-chunk N] <input> <output> [<oracle>...]
# Without an oracle: the mechanical pass alone.
# With one: `<oracle>... <events>` must print on stdout the set of lines
# covered by that file (`line_coverage.sh cover`), with a non-zero exit
# code if it fails. A candidate is kept only if its set is exactly that of
# the input rewritten by the tool. The first candidate is the output of the
# mechanical pass; if it changes the set, the guided pass starts again from
# the input. Then ever smaller chunks of frames (half, quarter... down to
# the --min-chunk size, 1 by default) are removed as long as the set does
# not move. One oracle call per candidate: slow on a real coverage, hence
# --min-chunk. The words of the oracle must not contain a space.
# Returns 1, without writing <output>, if the oracle fails on the input.

set -u

usage() {
    sed -n '2p' "$0" | sed 's/^# //' >&2
    exit 1
}

[ $# -ge 3 ] || usage
tool=$1
shift
min_chunk=1
if [ "$1" = --min-chunk ]; then
    [ $# -ge 4 ] || usage
    min_chunk=$2
    shift 2
fi
input=$1
output=$2
shift 2

if [ $# -eq 0 ]; then
    exec "$tool" mechanical "$input" "$output"
fi

work=$(mktemp -d "${TMPDIR:-/tmp}/minimize.XXXXXX") || exit 1
trap 'rm -rf "$work"' EXIT

calls=0
# Returns 0 if the set of the candidate file is that of the raw session.
same_lines() {
    calls=$((calls + 1))
    "$oracle_cmd" $oracle_args "$1" > "$work/candidate.lines" 2> /dev/null || return 1
    cmp -s "$work/candidate.lines" "$work/raw.lines"
}

oracle_cmd=$1
shift
oracle_args=$*

# Every output goes through p8_input_save, which always writes a d-pad of
# six booleans: judging against the input as it is would make the loader
# line that completes a four-entry d-pad a line no candidate covers.
"$tool" rewrite "$input" "$work/raw.json" || exit 1
if ! "$oracle_cmd" $oracle_args "$work/raw.json" > "$work/raw.lines"; then
    echo "minimize: the oracle fails on $input" >&2
    exit 1
fi
raw_frames=$("$tool" count "$input") || exit 1

"$tool" mechanical "$input" "$work/current.json" || exit 1
mechanical_frames=$("$tool" count "$work/current.json") || exit 1
if ! same_lines "$work/current.json"; then
    echo "minimize: the mechanical pass changes the covered lines; the guided pass starts again from $input" >&2
    cp "$work/raw.json" "$work/current.json" || exit 1
fi

n=$("$tool" count "$work/current.json") || exit 1
size=$((n / 2))
[ "$size" -ge "$min_chunk" ] || size=$min_chunk
while [ "$size" -ge "$min_chunk" ] && [ "$size" -ge 1 ]; do
    i=0
    while [ "$i" -lt "$n" ]; do
        count=$size
        [ $((i + count)) -le "$n" ] || count=$((n - i))
        if [ "$count" -lt "$n" ] \
            && "$tool" drop "$work/current.json" "$work/candidate.json" "$i" "$count" \
            && same_lines "$work/candidate.json"; then
            mv "$work/candidate.json" "$work/current.json" || exit 1
            n=$((n - count))
        else
            i=$((i + count))
        fi
    done
    echo "minimize: chunks of $size: $n frame(s)" >&2
    size=$((size / 2))
done

cp "$work/current.json" "$output" || exit 1
echo "minimize: $input: $raw_frames raw frame(s), $mechanical_frames after the mechanical pass, $n after the guided pass ($calls candidate(s) judged)" >&2
