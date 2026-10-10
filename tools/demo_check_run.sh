#!/bin/sh
# usage: demo_check_run.sh <dir> <test_demo_check> <test_attract_demo> <pad_author> <replay> <replay_sanitize>
#                          <demo.pad> <demo.json> <cycle.json> <save source.json> <title end-state> <sha256 command>
# Clears <dir>, runs every make demo-check job with its files in <dir>, then
# every check; prints `demo-check: <file>: <reason>` per failure, exit 1 if any.
# The hash is compared with <cycle.json>.sha256 only after the content checks pass.

set -u

if [ $# -ne 12 ]; then
    echo "usage: demo_check_run.sh <dir> <test_demo_check> <test_attract_demo> <pad_author> <replay> <replay_sanitize> <demo.pad> <demo.json> <cycle.json> <save source.json> <title end-state> <sha256 command>" >&2
    exit 2
fi

dir=$1
test_demo_check=$2
test_attract_demo=$3
pad_author=$4
replay=$5
replay_sanitize=$6
demo_pad=$7
demo_json=$8
cycle=$9
save_source=${10}
title_reference=${11}
sha256_command=${12}
tools=$(dirname "$0")
hash_reference=$cycle.sha256

content_failed=0
failed=0

fail() {
    echo "demo-check: $1: $2"
    content_failed=1
}

rm -rf "$dir" && mkdir -p "$dir" || exit 1

"$test_demo_check" || fail "$test_demo_check" "exit status $?"

"$test_attract_demo" "$dir/attract_cycle.json" "$save_source" > "$dir/attract_demo.log" 2>&1 &
attract_demo=$!
"$pad_author" "$demo_pad" "$dir/demo.json" --no-aim-budget > "$dir/regenerate.log" 2>&1 &
regenerate=$!
ATTRACT_STATE="$dir/attract.txt" STATE="$dir/attract_cycle.end.txt" SAVE_HEX="$dir/attract_cycle.end.hex" \
    "$replay" "$cycle" "$dir/attract_cycle.bin" "$dir/attract_cycle.dump.txt" > "$dir/attract_cycle.log" 2>&1 &
cycle_replay=$!
"$replay_sanitize" "$cycle" /dev/null "$dir/attract_cycle.sanitize.dump.txt" > "$dir/attract_cycle.sanitize.log" 2>&1 &
cycle_sanitize=$!

STATE="$dir/record.txt" "$pad_author" "$demo_pad" "$dir/record.json" --stop-at probe --no-aim-budget \
    > "$dir/record.log" 2>&1
record_status=$?
REPLAY_STARTS=2 STATE="$dir/end.txt" "$replay" "$demo_json" /dev/null "$dir/end.dump.txt" > "$dir/end.log" 2>&1
end_status=$?

if [ "$record_status" -ne 0 ]; then
    cat "$dir/record.log"
    fail "$demo_pad" "pad_author --stop-at probe exit status $record_status"
fi
if [ "$end_status" -ne 0 ]; then
    cat "$dir/end.log"
    fail "$demo_json" "REPLAY_STARTS=2 replay exit status $end_status"
fi
sh "$tools/demo_check.sh" "$demo_json" "$dir/record.txt" "$dir/end.txt.start1" "$dir/end.txt" || content_failed=1

wait "$attract_demo"
attract_demo_status=$?
cat "$dir/attract_demo.log"
[ "$attract_demo_status" -eq 0 ] || fail "$test_attract_demo" "exit status $attract_demo_status"

wait "$cycle_replay"
cycle_status=$?
if [ "$cycle_status" -ne 0 ]; then
    cat "$dir/attract_cycle.log"
    fail "$cycle" "replay exit status $cycle_status"
fi
sh "$tools/demo_check.sh" "$demo_json" "$dir/record.txt" "$dir/attract.txt.1" "$dir/attract.txt.2" || content_failed=1
[ ! -e "$dir/attract.txt.3" ] || fail "$cycle" "a third playback"
if sh "$tools/end_state.sh" normalize "$dir/end.txt" > "$dir/end.reference.txt" &&
    sh "$tools/end_state.sh" check "$cycle" "$dir/attract.txt.1" "$dir/end.reference.txt" \
        "$cycle" "$dir/attract.txt.2" "$dir/end.reference.txt"; then
    echo "demo-check: both playbacks of $cycle end where the resumed replay of $demo_json ends"
else
    fail "$cycle" "a playback does not end where the resumed replay of $demo_json ends"
fi

if sh "$tools/end_state.sh" check "$cycle" "$dir/attract.txt.1.return" "$title_reference" \
    "$cycle" "$dir/attract.txt.2.return" "$title_reference" \
    "$cycle" "$dir/attract_cycle.end.txt" "$title_reference"; then
    echo "demo-check: $cycle is back on the title at both returns and at its end"
else
    fail "$cycle" "not the title of $title_reference at a return or at its end"
fi
for hex in "$dir/attract.txt.1.return.hex" "$dir/attract.txt.2.return.hex" "$dir/attract_cycle.end.hex"; do
    cmp -s "$hex" "$dir/attract_cycle.json.save.hex" || fail "$hex" "not the initial_save of $save_source byte for byte"
done

wait "$cycle_sanitize"
sanitize_status=$?
if [ "$sanitize_status" -ne 0 ]; then
    cat "$dir/attract_cycle.sanitize.log"
    fail "$cycle" "$replay_sanitize exit status $sanitize_status"
fi

if ! cmp -s "$dir/attract_cycle.json" "$cycle"; then
    fail "$cycle" "not what test_attract_demo writes (cp $dir/attract_cycle.json $cycle, then rebaseline its hash)"
fi

# Not a content check: the cycle plays the compiled demo.json, whatever demo.pad says.
wait "$regenerate"
regenerate_status=$?
if [ "$regenerate_status" -ne 0 ]; then
    cat "$dir/regenerate.log"
    echo "demo-check: $demo_pad: pad_author exit status $regenerate_status"
    failed=1
elif cmp -s "$dir/demo.json" "$demo_json"; then
    echo "demo-check: $demo_pad regenerates $demo_json byte for byte"
else
    echo "demo-check: $demo_json: not what $demo_pad generates (make pad-author)"
    failed=1
fi

if [ "$content_failed" -ne 0 ]; then
    echo "demo-check: $cycle: hash not compared: the content checks failed"
    exit 1
fi
hash=$($sha256_command "$dir/attract_cycle.bin") || { echo "demo-check: $cycle: no capture hash"; exit 1; }
echo "${hash%% *}" > "$dir/attract_cycle.sha256"
if [ ! -f "$hash_reference" ]; then
    echo "demo-check: $hash_reference: no reference; the capture hash is ${hash%% *}"
    echo "demo-check:   after reading the content checks above: cp $dir/attract_cycle.sha256 $hash_reference"
    exit 1
fi
expected=$(cat "$hash_reference")
if [ "$expected" != "${hash%% *}" ]; then
    echo "demo-check: $cycle: the capture hash differs"
    echo "demo-check:   expected ($hash_reference): $expected"
    echo "demo-check:   actual   ($dir/attract_cycle.sha256): ${hash%% *}"
    echo "demo-check:   if intended, after reading the content checks above: cp $dir/attract_cycle.sha256 $hash_reference"
    exit 1
fi
echo "demo-check: $cycle: capture hash matches (${hash%% *})"
exit "$failed"
