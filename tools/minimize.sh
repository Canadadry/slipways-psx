#!/bin/sh
# usage: minimize.sh <pico8_minimize> [--min-chunk N] <entrée> <sortie> [<oracle>...]
# Sans oracle : l'étage mécanique seul.
# Avec : `<oracle>... <events>` doit écrire sur stdout l'ensemble de lignes
# couvertes par ce fichier (line_coverage.sh cover), code non nul s'il
# échoue. Un candidat n'est gardé que si son ensemble est exactement celui
# de l'entrée réécrite par l'outil. Le premier candidat est la sortie de
# l'étage mécanique ; s'il change l'ensemble, la passe repart de l'entrée.
# Puis des tranches de frames de plus en plus petites (moitié, quart...
# jusqu'à N, défaut 1) sont retirées tant que l'ensemble ne bouge pas. Un
# appel d'oracle par candidat : lent sur une vraie couverture, d'où
# --min-chunk. Les mots de l'oracle ne doivent pas contenir d'espace.
# Rend 1, sans écrire <sortie>, si l'oracle échoue sur l'entrée.

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
# Rend 0 si l'ensemble de $1 est celui de la session brute.
same_lines() {
    calls=$((calls + 1))
    "$oracle_cmd" $oracle_args "$1" > "$work/candidate.lines" 2> /dev/null || return 1
    cmp -s "$work/candidate.lines" "$work/raw.lines"
}

oracle_cmd=$1
shift
oracle_args=$*

# Toute sortie passe par pico8_input_save, qui écrit toujours un d-pad de
# 6 booléens : juger contre l'entrée telle quelle ferait de la ligne du
# chargeur qui complète un d-pad de 4 une ligne que nul candidat ne couvre.
"$tool" rewrite "$input" "$work/raw.json" || exit 1
if ! "$oracle_cmd" $oracle_args "$work/raw.json" > "$work/raw.lines"; then
    echo "minimize: l'oracle échoue sur $input" >&2
    exit 1
fi
raw_frames=$("$tool" count "$input") || exit 1

"$tool" mechanical "$input" "$work/current.json" || exit 1
mechanical_frames=$("$tool" count "$work/current.json") || exit 1
if ! same_lines "$work/current.json"; then
    echo "minimize: l'étage mécanique change les lignes couvertes, la passe repart de $input" >&2
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
    echo "minimize: tranches de $size : $n frame(s)" >&2
    size=$((size / 2))
done

cp "$work/current.json" "$output" || exit 1
echo "minimize: $input : $raw_frames frame(s) brutes, $mechanical_frames après l'étage mécanique, $n après la passe guidée ($calls candidat(s) jugé(s))" >&2
