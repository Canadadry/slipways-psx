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
        echo "frames: $events : aucune reference ($reference absent) -- rien capture ni compare."
        echo "frames:   pour la poser sur le code actuel : make e2e-baseline"
        continue
    fi
    if [ ! -f "$current" ]; then
        echo "frames: $events : empreinte courante absente ($current)" >&2
        failed=1
        continue
    fi

    attendue=$(cat "$reference")
    obtenue=$(cat "$current")
    if [ "$attendue" = "$obtenue" ]; then
        echo "frames: $events : conforme ($obtenue)"
        continue
    fi

    failed=1
    {
        echo "frames: DIVERGENT $events -- le port ne dessine plus la meme chose."
        echo "frames:   attendue ($reference) : $attendue"
        echo "frames:   obtenue  ($current) : $obtenue"
        echo "frames:   reconstruire la reference et trouver la premiere frame : make e2e-ref"
        echo "frames:   si la divergence est voulue : make e2e-baseline"
    } >&2
done

exit $failed
