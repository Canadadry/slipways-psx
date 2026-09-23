#!/bin/sh
# usage: frames_report.sh [--etape <id>] [<events> <reference> <courant>]...
#        frames_report.sh --garde <id ou ''> [<events> <reference> <courant>]...
# Rapport : ecrit sur stdout, rend 0 des que les entrees sont lisibles ; un
# courant absent ou une etape qui n'est pas un nom simple rend 2.
# Garde : rend 1, sur stderr, si une reference posee diverge et que l'etape est
# vide ; 0 sinon. Les frames sont numerotees a partir de 0, comme frames_check.sh.

set -u

fail() {
    echo "frames_report: $1" >&2
    exit 2
}

mode=rapport
etape=
case ${1:-} in
    --etape) mode=rapport; etape=${2-}; shift 2 || fail "--etape sans valeur"
             [ -n "$etape" ] || fail "etape vide" ;;
    --garde) mode=garde; etape=${2-}; shift 2 || fail "--garde sans valeur" ;;
esac
# L'etape devient un nom de fichier sous tools/ : pas de / ni de .. .
case $etape in
    *[!A-Za-z0-9_.-]* | .* ) fail "etape '$etape' : lettres, chiffres, _ . - seulement" ;;
esac
[ $(($# % 3)) -eq 0 ] || fail "arguments par triplets <events> <reference> <courant>"

[ "$mode" = rapport ] && [ -n "$etape" ] && echo "rapport de divergence, etape $etape"

divergents=
while [ $# -ge 3 ]; do
    events=$1
    reference=$2
    current=$3
    shift 3

    [ -f "$current" ] || fail "$events : empreintes courantes absentes ($current)"
    if [ ! -f "$reference" ]; then
        [ "$mode" = rapport ] && echo "$events : aucune reference"
        continue
    fi
    if cmp -s "$reference" "$current"; then
        [ "$mode" = rapport ] && echo "$events : conforme ($(wc -l < "$current" | tr -d ' ') frames)"
        continue
    fi
    divergents="$divergents $events"
    [ "$mode" = rapport ] || continue

    echo "$events : DIVERGENT"
    # Une frame presente d'un seul cote compte comme divergente.
    awk '
        FILENAME == ARGV[1] { ref[FNR] = $0; nref = FNR; next }
        { cur[FNR] = $0; ncur = FNR }
        END {
            n = (nref > ncur) ? nref : ncur
            for (k = 1; k <= n; k++) {
                if (k <= nref && k <= ncur && ref[k] == cur[k]) continue
                f = k - 1
                count++
                if (count == 1) first = f
                if (count > 1 && f == last + 1) { last = f; continue }
                if (count > 1) ranges = ranges plage(start, last) " "
                start = f; last = f
            }
            ranges = ranges plage(start, last)
            printf "  premiere frame divergente : %d\n", first
            printf "  frames divergentes : %d\n", count
            printf "  plages : %s\n", ranges
            if (nref != ncur) printf "  longueur : %d en reference, %d obtenue(s)\n", nref, ncur
        }
        function plage(a, b) { return (a == b) ? a : a "-" b }' "$reference" "$current"
done

if [ "$mode" = garde ] && [ -z "$etape" ] && [ -n "$divergents" ]; then
    echo "frames_report: reference(s) divergente(s) :$divergents" >&2
    echo "frames_report: re-baseline refuse sans etape ; nommer l'etape : make frames-baseline ETAPE=<id>" >&2
    exit 1
fi
exit 0
