#!/bin/sh
# usage: constant_variants.sh <racine src> <sources> <inventaire> <releve> <outil d'empreintes>
#                             <sortie> <litteral ou ''> [<events> <reference>]...
# <sources> : une chaine, les .c du pilote de rejeu, chemins sous <racine src>.
# Par litteral (celui nomme, sinon chaque ECART de constants_compare.sh) :
# <sortie>/lit-<litteral>/ est efface puis recoit la copie des .c/.h de la
# racine ou pico8_fixed_from_double(<litteral>) devient la valeur brute du
# releve, le pilote compile depuis cette copie, une capture, des empreintes et
# un dump par fichier d'evenements, et rapport.txt (frames_report.sh). La
# synthese va dans <sortie>/synthese.txt. Le pilote est lance depuis le
# repertoire courant : <pilote> <events> <capture> <dump>.
# Rend 0 meme sans aucune divergence ; 1 si une variante ne se construit pas
# (remplacements != sites de l'inventaire, compilation) ou si un rejeu echoue.

set -u

root=$1
sources=$2
inventory=$3
probe=$4
hash_tool=$5
out=$6
literal=$7
shift 7

tools=$(dirname "$0")

fail() {
    echo "constant_variants: $1" >&2
    exit 1
}

[ $(($# % 2)) -eq 0 ] || fail "arguments par paires <events> <reference>"
mkdir -p "$out" || fail "sortie $out impossible a creer"
table=$out/comparateur.txt
sh "$tools/constants_compare.sh" "$inventory" "$probe" "$root/pico8_fixed.c" "$out/comparateur" > "$table" \
    || fail "le comparateur s'arrete en erreur"

if [ -n "$literal" ]; then
    awk -F' [|] ' -v lit="$literal" 'NF == 5 && $1 == lit { found = 1 } END { exit !found }' "$table" \
        || fail "litteral $literal absent des constantes de $inventory"
    selected=$literal
else
    selected=$(awk -F' [|] ' 'NF == 5 && $4 == "ECART" { print $1 }' "$table")
fi

summary=$out/synthese.txt
: > "$summary.tmp"

for lit in $selected; do
    name=lit-$lit
    vdir=$out/$name
    raw=$(awk -F' [|] ' -v lit="$lit" 'NF == 5 && $1 == lit { print $2 }' "$table")
    # 0xIIII.FFFF lu comme un entier 32 bits signe, en decimal pour le C.
    value=$(echo "$raw" | awk '{
        h = tolower($1); sub(/^0x/, "", h); sub(/\./, "", h)
        v = 0
        for (i = 1; i <= length(h); i++) v = v * 16 + index("0123456789abcdef", substr(h, i, 1)) - 1
        if (v >= 2147483648) v -= 4294967296
        printf "%.0f\n", v
    }')

    rm -rf "$vdir"
    mkdir -p "$vdir/src" "$vdir/frames" || fail "variante $name : $vdir impossible a creer"
    : > "$vdir/remplacements.txt"
    for f in "$root"/*.c "$root"/*.h; do
        [ -f "$f" ] || continue
        # Correspondance exacte, parenthese fermante comprise : la variante 0.1
        # ne touche ni 0.12 ni 0.125.
        awk -v token="pico8_fixed_from_double($lit)" -v repl="((pico8_fixed_t)($value))" \
            -v file="$f" -v count="$vdir/remplacements.txt" '
            {
                line = $0; res = ""
                while ((p = index(line, token)) > 0) {
                    res = res substr(line, 1, p - 1) repl
                    line = substr(line, p + length(token))
                    n++
                }
                print res line
            }
            END { if (n) print file " | " n >> count }' "$f" > "$vdir/src/$(basename "$f")" \
            || fail "variante $name : copie de $f impossible"
    done

    check=$(awk -F' [|] ' -v lit="$lit" '
        FILENAME == ARGV[1] { if (!/^#/ && $3 == "constante" && $NF == lit) { want[$1]++; total++ }; next }
        { got[$1] = $2; done += $2 }
        END {
            for (f in got) if (!(f in want)) print "remplacement hors inventaire dans " f
            if (done != total) print done " remplacement(s), " total " site(s) dans l inventaire"
        }' "$inventory" "$vdir/remplacements.txt")
    [ -z "$check" ] || fail "variante $name : $check"

    vsources=
    for s in $sources; do
        vsources="$vsources $vdir/src/${s#"$root"/}"
    done
    # shellcheck disable=SC2086
    ${CC:-cc} ${CFLAGS:--std=c99 -Wall -Wextra -Werror} -o "$vdir/pico8_replay" $vsources -lm \
        > "$vdir/compilation.txt" 2>&1 \
        || { cat "$vdir/compilation.txt" >&2; fail "variante $name : compilation en echec"; }

    triplets=
    i=0
    for arg in "$@"; do
        i=$((i + 1))
        if [ $((i % 2)) -eq 1 ]; then
            events=$arg
            continue
        fi
        reference=$arg
        base=$vdir/frames/$(echo "$events" | tr '/' '_')
        "$vdir/pico8_replay" "$events" "$base.bin" "$base.dump.txt" > "$base.log" 2>&1 \
            || { cat "$base.log" >&2; fail "variante $name : $events : rejeu en echec"; }
        "$hash_tool" hashes "$base.bin" "$base.frames.hash" \
            || fail "variante $name : $events : empreintes impossibles"
        triplets="$triplets $events $reference $base.frames.hash"
    done
    # shellcheck disable=SC2086
    sh "$tools/frames_report.sh" $triplets > "$vdir/rapport.txt" \
        || fail "variante $name : rapport de divergence impossible"

    {
        echo "variante $name : $lit -> $raw"
        awk -F' [|] ' -v lit="$lit" '!/^#/ && $3 == "constante" && $NF == lit { print "  site : " $1 " | " $2 " | " $4 }' "$inventory"
        awk '
            / : conforme / { sub(/ : conforme .*/, ""); print "  " $0 " : 0 frame"; next }
            / : aucune reference$/ { print "  " $0; next }
            / : DIVERGENT$/ { sub(/ : DIVERGENT$/, ""); ev = $0; next }
            /^  premiere frame divergente : / { first = $NF; next }
            /^  frames divergentes : / { n = $NF; next }
            /^  plages : / { sub(/^  plages : /, ""); print "  " ev " : premiere " first " | frames " n " | plages " $0; next }
            /^  longueur : / { print "  " $0 }' "$vdir/rapport.txt"
    } >> "$summary.tmp"
done

mv "$summary.tmp" "$summary"
cat "$summary"
