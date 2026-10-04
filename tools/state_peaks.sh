#!/bin/sh
# usage: state_peaks.sh <out> <events> <state> [<events> <state>...]
# Writes <out> in every case: one `replay <events> clean|stopped ...|no state`
# line per pair, then one `<zone|cap> <name> <peak> <limit> <events>` line per
# zone and cap, the highest peak over every state (a tie keeps the first
# pair). Exit 1, with `state: <events> stopped`, when a state is missing or
# starts with `stopped`.

set -u

out=$1
shift

failed=0
: > "$out.tmp"
awk_args=""
while [ $# -ge 2 ]; do
    events=$1
    state=$2
    shift 2
    status="no state"
    if [ -f "$state" ]; then
        status=$(head -n 1 "$state")
        case $status in
            stopped*) ;;
            *) status=clean ;;
        esac
        awk_args="$awk_args replay=$events $state"
    fi
    echo "replay $events $status" >> "$out.tmp"
    if [ "$status" != clean ]; then
        echo "state: $events stopped" >&2
        failed=1
    fi
done

if [ -n "$awk_args" ]; then
    awk '
        ($1 == "zone" || $1 == "cap") && NF == 5 {
            key = $1 " " $2
            if (!(key in peak)) { order[++keys] = key; peak[key] = -1 }
            if ($4 + 0 > peak[key]) { peak[key] = $4 + 0; limit[key] = $5; holder[key] = replay }
        }
        END { for (i = 1; i <= keys; i++) print order[i], peak[order[i]], limit[order[i]], holder[order[i]] }
    ' $awk_args >> "$out.tmp"
fi

mv "$out.tmp" "$out"
exit $failed
