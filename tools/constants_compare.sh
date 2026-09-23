#!/bin/sh
# usage: constants_compare.sh <inventaire> <releve> <pico8_fixed.c> <repertoire de travail> [<source.c>...]
# Une ligne par litteral distinct, jetons constante de l'inventaire et appels
# PICO8_FIXED_LIT(<hexa>, <litteral>) des sources reunis :
#   <litteral> | <hexa releve> | <hexa C> | ok|ECART | <sites>
# puis la ligne des comptes. Un litteral est ECART si une de ses valeurs
# differe du releve ; <hexa C> liste alors ses valeurs distinctes, separees
# par des virgules. La valeur C vient d'un programme engendre dans le
# repertoire de travail, qui compile le texte exact de chaque appel contre le
# vrai pico8_fixed.h. Sans source, seul l'inventaire est lu. Rend 0 si la
# table est complete, meme avec des ECART ; 1 si le bloc A manque, si un
# litteral n'y figure pas, s'il n'y a aucune constante, si un appel
# PICO8_FIXED_LIT n'a pas la forme stricte, ou si le programme ne compile pas.

set -u

inventory=$1
probe=$2
fixed_c=$3
work=$4
shift 4

fail() {
    echo "constants_compare: $1" >&2
    exit 1
}

mkdir -p "$work" || fail "repertoire de travail $work impossible a creer"
sites=$work/sites.txt
releve=$work/releve.txt
gen=$work/constants_values.c
bin=$work/constants_values

awk -F' [|] ' '!/^#/ && $3 == "constante" { print $NF "\tpico8_fixed_from_double(" $NF ")" }' \
    "$inventory" > "$sites" || fail "inventaire $inventory illisible"

if [ $# -gt 0 ]; then
    awk '
    {
        rest = $0
        while ((p = index(rest, "PICO8_FIXED_LIT")) > 0) {
            rest = substr(rest, p)
            if (!match(rest, /^PICO8_FIXED_LIT\(0x[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f], [0-9]*\.?[0-9]+\)/)) {
                printf "constants_compare: %s:%d : appel PICO8_FIXED_LIT hors de la forme stricte\n", FILENAME, FNR > "/dev/stderr"
                bad = 1
                break
            }
            call = substr(rest, 1, RLENGTH)
            lit = substr(call, 29, length(call) - 29)
            print lit "\t" call
            rest = substr(rest, RLENGTH + 1)
        }
    }
    END { exit bad }' "$@" >> "$sites" || fail "appel PICO8_FIXED_LIT mal forme"
fi
[ -s "$sites" ] || fail "aucune constante dans $inventory ni dans les sources"

awk '/^## a literals$/ { in_a = 1; found = 1; next }
     /^## / { in_a = 0 }
     in_a && NF == 2 { print $1, tolower($2) }
     END { exit found ? 0 : 1 }' "$probe" > "$releve" \
    || fail "bloc A (## a literals) introuvable dans $probe"

missing=$(awk -F'\t' 'NR == FNR { split($0, f, " "); seen[f[1]] = 1; next } !($1 in seen) && !($1 in told) { told[$1] = 1; print $1 }' "$releve" "$sites")
[ -z "$missing" ] || fail "litteral absent du bloc A de $probe : $(echo $missing)"

{
    echo '#include <stdint.h>'
    echo '#include <stdio.h>'
    echo '#include "pico8_fixed.h"'
    echo 'static void show(int call, pico8_fixed_t v) {'
    echo '    uint32_t u = (uint32_t)v;'
    printf '%s\n' '    printf("%d 0x%04x.%04x\n", call, (unsigned)(u >> 16), (unsigned)(u & 0xffffu));'
    echo '}'
    echo 'int main(void) {'
    awk -F'\t' '!($2 in seen) { seen[$2] = ++k; printf "    show(%d, %s);\n", k, $2 }' "$sites"
    echo '    return 0;'
    echo '}'
} > "$gen"

${CC:-cc} -std=c99 -Wall -Wextra -Werror -I "$(dirname "$fixed_c")" -o "$bin" "$gen" "$fixed_c" -lm \
    || fail "le programme engendre $gen ne compile pas"
"$bin" > "$work/valeurs.txt" || fail "$bin s'arrete en erreur"

awk 'FILENAME == ARGV[1] { rel[$1] = $2; next }
     FILENAME == ARGV[2] { value[$1] = $2; next }
     {
         split($0, f, "\t")
         lit = f[1]
         if (!(f[2] in call)) call[f[2]] = ++k
         v = value[call[f[2]]]
         if (!(lit in n)) order[++lits] = lit
         n[lit]++
         if (!((lit, v) in has)) {
             has[lit, v] = 1
             if (lit in c) c[lit] = c[lit] "," v
             else c[lit] = v
             if (v != rel[lit]) off[lit] = 1
         }
     }
     END {
         for (i = 1; i <= lits; i++) {
             lit = order[i]
             status = (lit in off) ? "ECART" : "ok"
             print lit " | " rel[lit] " | " c[lit] " | " status " | " n[lit]
             total += n[lit]; if (status == "ok") ok++; else ecart++
         }
         printf "litteraux %d | sites %d | ok %d | ecart %d\n", lits, total, ok, ecart
     }' \
    "$releve" "$work/valeurs.txt" "$sites"
