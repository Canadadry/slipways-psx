#!/bin/sh
# usage: float_coverage.sh <inventaire> <compilateur> <sortie gcov -t>...
# Ecrit le rapport sur la sortie standard. Rend 2 si l'inventaire ne contient
# aucun site : une jointure vide passerait pour une couverture complete.

set -u

inventory=$1
compiler=$2
shift 2

cat <<EOF
# Couverture des sites flottants du coeur sous le filet (docs/plan-2.md §7,
# Phase B), produite par tools/float_coverage.sh via make float-coverage.
# Ne pas editer a la main.
#
# Compilateur : $compiler
#
# Les sites sont ceux de tools/float_inventory.sh ; les passages viennent de
# gcov, apres rejeu de chaque fichier de FRAMES_EVENTS par un build --coverage.
# Une ligne par site : fichier | fonction | categorie | rang | ligne | couverture | population | etape | extrait
#
# Limite : la granularite est la ligne, pas l'expression. Deux sites d'une meme
# ligne recoivent le meme compte, et un site dans une branche jamais prise
# (un ternaire, par exemple) recoit celui de la ligne.
#
# NON COUVERT (#####)           ligne executable jamais atteinte
# NON COUVERT (ligne sans code) gcov n'y rattache aucune instruction : le site
#                               n'est pas mesure. Le plus souvent, c'est la suite
#                               d'une instruction ecrite sur plusieurs lignes ;
#                               "; ligne N : X" donne la ligne mesuree qui la
#                               precede, a relire avant d'en conclure quoi que ce soit
# NON COUVERT (absente de gcov) fichier ou ligne sans sortie gcov
EOF

cat "$@" /dev/null | awk -v inventory="$inventory" '
function trim(s) {
    sub(/^[ \t]+/, "", s)
    sub(/[ \t]+$/, "", s)
    return s
}

function preceding(file, line,    k, key) {
    for (k = line - 1; k >= 1; k--) {
        key = file SUBSEP k
        if (!(key in state)) continue
        if (state[key] == "n") return " ; ligne " k " : " passes[key]
        if (state[key] == "#") return " ; ligne " k " : #####"
    }
    return ""
}

/^ *-: *0:Source:/ {
    source = $0
    sub(/^ *-: *0:Source:/, "", source)
    next
}

{
    colon1 = index($0, ":")
    if (colon1 == 0) next
    rest = substr($0, colon1 + 1)
    colon2 = index(rest, ":")
    if (colon2 == 0) next
    count = trim(substr($0, 1, colon1 - 1))
    line = trim(substr(rest, 1, colon2 - 1)) + 0
    if (line == 0) next
    sub(/\*$/, "", count)
    key = source SUBSEP line
    if (count == "-") {
        if (!(key in state)) state[key] = "-"
    } else if (count == "#####" || count == "=====") {
        if (!(key in state) || state[key] == "-") state[key] = "#"
    } else {
        state[key] = "n"
        passes[key] += count
    }
}

END {
    sites = 0
    while ((getline entry < inventory) > 0) {
        if (entry ~ /^#/ || entry ~ /^compte / || entry == "") continue
        n = split(entry, field, " \\| ")
        file = field[1]
        category = field[3]
        line = field[5]
        sub(/^ligne /, "", line)
        key = file SUBSEP (line + 0)
        if (state[key] == "n") {
            status = "passages " passes[key]
            covered[category]++
            covered_total++
        } else {
            if (state[key] == "#") status = "NON COUVERT (#####)"
            else if (state[key] == "-") status = "NON COUVERT (ligne sans code" preceding(file, line + 0) ")"
            else status = "NON COUVERT (absente de gcov)"
            uncovered[category]++
            uncovered_total++
        }
        head = field[1] " | " field[2] " | " field[3] " | " field[4] " | " field[5]
        tail = substr(entry, length(head) + 4)
        printf "%s | %s | %s\n", head, status, tail
        sites++
    }
    if (sites == 0) {
        printf "float_coverage: aucun site dans %s\n", inventory > "/dev/stderr"
        exit 2
    }
    print "#"
    print "# Comptes"
    split("constante cast reecrit autre", order, " ")
    for (k = 1; k <= 4; k++) {
        printf "couverture %s couverts %d non-couverts %d\n", order[k], covered[order[k]], uncovered[order[k]]
    }
    printf "couverture total couverts %d non-couverts %d\n", covered_total, uncovered_total
}
'
