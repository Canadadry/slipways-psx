#!/bin/sh
# usage: e2e_limits.sh <exceptions> [<events>...]
# Exit 1 when the exception list is missing, or when an e2e is missing,
# lacks its <name>.txt or its <name>.json.sha256, or runs past 180 frames
# (600 when it is listed in <exceptions>, as `<events> reason`: only the
# first field counts), or when a keyboard_* e2e does not declare
# "keyboard_cursor": true, moves the mouse or presses a mouse button, or
# when a pad e2e ("input": "pad") carries mouse_x, mouse_y, mouse_buttons,
# dpad or keyboard_cursor.
# Every file is examined before concluding.
# A frame of the file is a game frame: the "mouse_x" keys are counted, the
# "pad" keys in a pad e2e.

set -u

exceptions=$1
shift
if [ ! -f "$exceptions" ]; then
    echo "e2e: missing exceptions list: $exceptions" >&2
    exit 1
fi

# The distinct values of member $1 in file $2, one per line.
values() {
    grep -o "\"$1\": *[0-9-]*" "$2" | sed 's/.*: *//' | sort -u
}

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

    frame_key='"mouse_x"'
    if grep -q '"input": *"pad"' "$events"; then
        frame_key='"pad":'
        for key in mouse_x mouse_y mouse_buttons dpad keyboard_cursor; do
            if grep -q "\"$key\"" "$events"; then
                echo "e2e: $events: pad e2e carries the mouse-mode field \"$key\"" >&2
                failed=1
            fi
        done
    fi
    frames=$(grep -o "$frame_key" "$events" | wc -l | tr -d ' ')
    limit=180
    if awk -v e="$events" '$1 == e { found = 1 } END { exit !found }' "$exceptions"; then
        limit=600
    fi
    if [ "$frames" -gt "$limit" ]; then
        echo "e2e: $events: $frames frames, more than $limit" >&2
        failed=1
    fi

    case ${events##*/} in
    keyboard_*)
        if ! grep -q '"keyboard_cursor": *true' "$events"; then
            echo "e2e: $events: keyboard_ e2e does not declare \"keyboard_cursor\": true" >&2
            failed=1
        fi
        for axis in mouse_x mouse_y; do
            if [ "$(values $axis "$events" | wc -l | tr -d ' ')" -gt 1 ]; then
                echo "e2e: $events: keyboard_ e2e moves the mouse ($axis)" >&2
                failed=1
            fi
        done
        if values mouse_buttons "$events" | grep -qvx 0; then
            echo "e2e: $events: keyboard_ e2e presses a mouse button" >&2
            failed=1
        fi
        ;;
    esac
done

exit $failed
