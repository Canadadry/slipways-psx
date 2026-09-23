#!/bin/sh
# usage: constants_compare.sh <inventaire> <releve> <pico8_fixed.c> <repertoire de travail>
# Une ligne par litteral distinct des jetons constante de l'inventaire :
#   <litteral> | <hexa releve> | <hexa C> | ok|ECART | <sites>
# puis la ligne des comptes. La valeur C vient d'un programme engendre dans le
# repertoire de travail, qui passe le texte exact du litteral a la vraie
# pico8_fixed_from_double. Rend 0 si la table est complete, meme avec des
# ECART ; 1 si le bloc A manque, si un litteral n'y figure pas, s'il n'y a
# aucune constante, ou si le programme ne compile pas.

set -u

inventory=$1
probe=$2
fixed_c=$3
work=$4

fail() {
    echo "constants_compare: $1" >&2
    exit 1
}

mkdir -p "$work" || fail "repertoire de travail $work impossible a creer"
sites=$work/sites.txt
releve=$work/releve.txt
gen=$work/constants_values.c
bin=$work/constants_values

awk -F' [|] ' '!/^#/ && $3 == "constante" {
    if (!($NF in n)) order[++k] = $NF
    n[$NF]++
} END { for (i = 1; i <= k; i++) print order[i], n[order[i]] }' "$inventory" > "$sites" \
    || fail "inventaire $inventory illisible"
[ -s "$sites" ] || fail "aucune constante dans $inventory"

awk '/^## a literals$/ { in_a = 1; found = 1; next }
     /^## / { in_a = 0 }
     in_a && NF == 2 { print $1, tolower($2) }
     END { exit found ? 0 : 1 }' "$probe" > "$releve" \
    || fail "bloc A (## a literals) introuvable dans $probe"

missing=$(awk 'NR == FNR { seen[$1] = 1; next } !($1 in seen) { print $1 }' "$releve" "$sites")
[ -z "$missing" ] || fail "litteral absent du bloc A de $probe : $(echo $missing)"

{
    echo '#include <stdint.h>'
    echo '#include <stdio.h>'
    echo '#include "pico8_fixed.h"'
    echo 'static void show(const char *lit, pico8_fixed_t v) {'
    echo '    uint32_t u = (uint32_t)v;'
    printf '%s\n' '    printf("%s 0x%04x.%04x\n", lit, (unsigned)(u >> 16), (unsigned)(u & 0xffffu));'
    echo '}'
    echo 'int main(void) {'
    awk '{ printf "    show(\"%s\", pico8_fixed_from_double(%s));\n", $1, $1 }' "$sites"
    echo '    return 0;'
    echo '}'
} > "$gen"

${CC:-cc} -std=c99 -Wall -Wextra -Werror -I "$(dirname "$fixed_c")" -o "$bin" "$gen" "$fixed_c" -lm \
    || fail "le programme engendre $gen ne compile pas"
"$bin" > "$work/valeurs.txt" || fail "$bin s'arrete en erreur"

awk 'FILENAME == ARGV[1] { rel[$1] = $2; next }
     FILENAME == ARGV[2] { c[$1] = $2; next }
     {
         status = (rel[$1] == c[$1]) ? "ok" : "ECART"
         print $1 " | " rel[$1] " | " c[$1] " | " status " | " $2
         lits++; total += $2; if (status == "ok") ok++; else ecart++
     }
     END { printf "litteraux %d | sites %d | ok %d | ecart %d\n", lits, total, ok, ecart }' \
    "$releve" "$work/valeurs.txt" "$sites"
