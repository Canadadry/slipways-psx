#!/bin/sh
# usage: e2e_limits.sh <exceptions> [<events>...]
# Exit 1 when the exception list is missing, or when an e2e is missing,
# lacks its <name>.txt or its <name>.json.sha256, or runs past 180 frames
# (600 when it is listed in <exceptions>, as `<events> reason`: only the
# first field counts). Every file is examined before concluding.
# A frame of the file is a game frame: the "mouse_x" keys are counted.

set -u

exceptions=$1
shift
if [ ! -f "$exceptions" ]; then
    echo "e2e: missing exceptions list: $exceptions" >&2
    exit 1
fi

failed=0
for events in "$@"; do
    if [ ! -f "$events" ]; then
        echo "e2e: $events missing" >&2
        failed=1
        continue
    fi
    for companion in "${events%.json}.txt" "$events.sha256"; do
        if [ ! -f "$companion" ]; then
            echo "e2e: $events: $companion missing" >&2
            failed=1
        fi
    done

    frames=$(grep -o '"mouse_x"' "$events" | wc -l | tr -d ' ')
    limit=180
    if awk -v e="$events" '$1 == e { found = 1 } END { exit !found }' "$exceptions"; then
        limit=600
    fi
    if [ "$frames" -gt "$limit" ]; then
        echo "e2e: $events: $frames frames, more than $limit" >&2
        failed=1
    fi
done

exit $failed
