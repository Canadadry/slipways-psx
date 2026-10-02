#!/bin/sh
# usage: e2e_limits.sh <exceptions> [<events>...]
# Rend 1 si un e2e n'a pas son <nom>.txt ou son <nom>.json.sha256, ou dépasse
# 180 frames ; 600 s'il est listé dans <exceptions> (`<events> raison`, seul le
# premier champ compte). Tous les fichiers sont examinés avant de conclure.
# Une frame du fichier est une frame de jeu : on compte les "mouse_x".

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
