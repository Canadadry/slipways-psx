#!/bin/sh
# usage: frames_check.sh [<events> <reference .sha256> <empreinte courante>]...
# Rend 0 si aucune reference posee ne diverge ; tous les rejeux sont examines
# avant de conclure.

set -u

failed=0
while [ $# -ge 3 ]; do
    events=$1
    reference=$2
    current=$3
    shift 3

    if [ ! -f "$reference" ]; then
        echo "frames: $events: no reference ($reference missing) -- not compared."
        echo "frames:   to set it from the current code: make e2e-baseline"
        continue
    fi
    if [ ! -f "$current" ]; then
        echo "frames: $events: current hash missing ($current)" >&2
        failed=1
        continue
    fi

    expected=$(cat "$reference")
    actual=$(cat "$current")
    if [ "$expected" = "$actual" ]; then
        echo "frames: $events: matches ($actual)"
        continue
    fi

    failed=1
    {
        echo "frames: DIVERGENT $events -- the game no longer draws the same thing."
        echo "frames:   expected ($reference): $expected"
        echo "frames:   actual   ($current): $actual"
        echo "frames:   rebuild the reference and find the first diverging frame: make e2e-ref"
        echo "frames:   if the divergence is intended: make e2e-baseline"
    } >&2
done

exit $failed
