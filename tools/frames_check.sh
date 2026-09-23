#!/bin/sh
# usage: frames_check.sh <outil pico8_frames> [<events> <reference> <courant> <capture>]...
# Rend 0 si aucune reference posee ne diverge ; tous les rejeux sont examines
# avant de conclure.

set -u

MAX_LISTED=10
tool=$1
shift

failed=0
while [ $# -ge 4 ]; do
    events=$1
    reference=$2
    current=$3
    capture=$4
    shift 4

    if [ ! -f "$reference" ]; then
        echo "frames: $events : aucune reference ($reference absent) -- rien compare."
        echo "frames:   pour la poser sur le code actuel : make frames-baseline"
        continue
    fi
    if [ ! -f "$current" ]; then
        echo "frames: $events : empreintes courantes absentes ($current)" >&2
        failed=1
        continue
    fi

    if cmp -s "$reference" "$current"; then
        echo "frames: $events : conforme ($(wc -l < "$current" | tr -d ' ') frames)"
        continue
    fi

    failed=1
    report=$(awk -v max="$MAX_LISTED" '
        NR == FNR { ref[FNR] = $0; nref = FNR; next }
        {
            ncur = FNR
            if (FNR <= nref && ref[FNR] != $0) {
                count++
                if (count <= max) listed = listed " " (FNR - 1)
            }
        }
        END {
            if (count > 0) printf "%d frame(s) divergente(s), premieres :%s\n", count, listed
            if (nref != ncur) printf "%d ligne(s) en reference, %d obtenue(s)\n", nref, ncur
            first = (count > 0) ? substr(listed, 2) : ((nref < ncur) ? nref : ncur)
            sub(/ .*/, "", first)
            printf "FIRST %s\n", first
        }' "$reference" "$current")
    first=$(printf '%s\n' "$report" | sed -n 's/^FIRST //p')
    {
        echo "frames: DIVERGENT $events -- le port ne dessine plus la meme chose."
        printf '%s\n' "$report" | grep -v '^FIRST ' | sed 's/^/frames:   /'
        echo "frames:   reference $reference, obtenu $current"
        echo "frames:   emettre une frame N : $tool frame $capture $first"
        echo "frames:   si la divergence est voulue : make frames-baseline"
    } >&2
done

exit $failed
