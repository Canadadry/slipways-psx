#!/bin/sh
# usage: constants_check.sh <defaillances attendues> <inventaire> <releve> <pico8_fixed.c> <repertoire de travail> [<source.c>...]
# Rend 0 si les litteraux en ECART de constants_compare.sh sont exactement ceux
# de la liste (un par ligne, # pour un commentaire), en n'imprimant que les
# comptes. Rend 1 sinon, ou si le comparateur echoue, ou si la liste nomme un
# litteral absent de l'inventaire et des sources ; la table complete part
# alors sur stderr.

set -u

expected=$1
shift
work=$4
table=$work/table.txt

fail() {
    echo "constants_check: $1" >&2
    exit 1
}

[ -f "$expected" ] || fail "liste des defaillances attendues $expected absente"
mkdir -p "$work" || fail "repertoire de travail $work impossible a creer"
sh "$(dirname "$0")/constants_compare.sh" "$@" > "$table" || fail "le comparateur s'arrete en erreur"

verdict=$(awk -F' [|] ' '
    FILENAME == ARGV[1] { if ($0 !~ /^#/ && NF) listed[$0] = 1; next }
    NF == 5 { status[$1] = $4 }
    END {
        for (lit in listed) if (!(lit in status)) print "litteral liste absent de l inventaire : " lit
        for (lit in listed) if (status[lit] == "ok") print "liste perimee : retirer " lit
        for (lit in status) if (status[lit] == "ECART" && !(lit in listed)) print "regression : " lit
    }' "$expected" "$table" | sort)

if [ -n "$verdict" ]; then
    cat "$table" >&2
    echo "$verdict" | sed 's/^/constants_check: /' >&2
    fail "les ECART ne sont pas ceux de $expected"
fi
tail -n 1 "$table"
