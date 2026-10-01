#!/bin/sh
# usage: alloc_guard.sh <liste d'exceptions> <racine des objets> <objet.o>...
# Compte, par objet, les relocations vers malloc, calloc, realloc, free et la
# poignee pico8_zone_transitoire, adossee a la libc (aussi sous la forme
# _malloc de Darwin), et exige l'egalite stricte avec la liste. L'objet
# <racine>/src/x.o y figure sous src/x.c. Rend 1 sur tout ecart, 2 sans
# objet : une garde sur rien passerait pour verte. $OBJDUMP remplace objdump.

set -u

list=$1
root=$2
shift 2

if [ $# -eq 0 ]; then
    echo "alloc_guard: aucun objet" >&2
    exit 2
fi

if [ ! -r "$list" ]; then
    echo "alloc_guard: liste illisible : $list" >&2
    exit 2
fi

objdump=${OBJDUMP:-objdump}
actual=$(mktemp)
report=$(mktemp)
trap 'rm -f "$actual" "$report"' EXIT

for obj in "$@"; do
    src=${obj#"$root"/}
    src=${src%.o}.c
    # Seules les sections de code et de donnees comptent : les relocations de
    # .debug_* et .eh_frame ne sont pas des appels.
    count=$("$objdump" -r "$obj" | awk '
        /^RELOCATION RECORDS FOR/ { skip = ($0 ~ /debug|eh_frame/); next }
        skip || NF < 3 { next }
        {
            sym = $NF
            sub(/[-+]0x[0-9a-fA-F]+$/, "", sym)
            sub(/^_/, "", sym)
            if (sym == "malloc" || sym == "calloc" || sym == "realloc" || sym == "free" || sym == "pico8_zone_transitoire") n++
        }
        END { print n + 0 }')
    echo "$src $count" >> "$actual"
done

{
    awk '{ print "A", $1, $2 }' "$actual"
    sed -e 's/#.*//' "$list" | awk 'NF >= 2 { print "L", $1, $2 }'
} | awk '
    $1 == "A" { seen[$2] = 1; got[$2] = $3 }
    $1 == "L" { listed[$2] = $3 }
    END {
        for (f in got) {
            if (got[f] > 0 && !(f in listed)) {
                printf "alloc_guard: %s alloue %d fois, hors liste ; ligne a ajouter : %s %d\n", f, got[f], f, got[f]
            } else if ((f in listed) && got[f] == 0) {
                printf "alloc_guard: %s n\047alloue plus ; ligne a retirer : %s %s\n", f, f, listed[f]
            } else if ((f in listed) && got[f] != listed[f]) {
                printf "alloc_guard: %s alloue %d fois, %s inscrit(s) ; ligne a ecrire : %s %d\n", f, got[f], listed[f], f, got[f]
            }
        }
        for (f in listed) {
            if (!(f in seen)) {
                printf "alloc_guard: %s n\047est pas dans les objets recus ; ligne a retirer : %s %s\n", f, f, listed[f]
            }
        }
    }' | LC_ALL=C sort > "$report"
cat "$report"
awk '$2 > 0 { files++; calls += $2 } END { printf "verrou objets %d allouent %d appels %d\n", NR, files, calls }' "$actual"
[ ! -s "$report" ]
