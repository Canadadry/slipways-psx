#!/bin/sh
# usage: float_inventory.sh <source.c>...
# Ecrit l'inventaire sur la sortie standard. Rend 2 si un jeton flottant
# d'une ligne n'est pris par aucune categorie, 3 si la somme des categories
# ne vaut pas le nombre de jetons : l'inventaire n'est alors pas ecrit en entier.

set -u

cat <<'EOF'
# Inventaire des sites flottants du coeur (docs/plan-2.md §3), produit par
# tools/float_inventory.sh via make float-inventory. Ne pas editer a la main.
#
# Une ligne par jeton : fichier | fonction | categorie | rang | ligne | population | extrait
# Identite d'un site : fichier, fonction englobante, categorie, rang parmi les
# jetons de meme categorie dans cette fonction. Le numero de ligne sert a y
# aller, jamais de cle.
#
# Categories :
#   constante  pico8_fixed_from_double(<litteral numerique>)
#   cast       (int) suivi de pico8_fixed_to_double(
#   reecrit    pow( sin( cos( de la libc, la chaine "%.4f", et
#              pico8_fixed_from_double( sur une expression non litterale
#              hors de pico8_fixed.c
#   autre      tout autre jeton double/float, pico8_fixed_to_double hors cast,
#              pico8_fixed_from_double hors des regles ci-dessus (dont les
#              definitions de pico8_fixed.c, qui implementent la conversion)
#
# Population des casts : "a auditer" partout. Le plan ne donne aucune regle
# deductible du code seul : (a) exige la correspondance avec un flr( de
# pico8/lua.lua, (b) une preuve de signe par site, (c) est le reste des deux.
# C'est la Phase C qui les etablira.
EOF

awk '
function word_char(c) {
    return c ~ /[A-Za-z0-9_]/
}

function boundary_before(s, i) {
    return i == 1 || !word_char(substr(s, i - 1, 1))
}

function boundary_after(s, i) {
    return i > length(s) || !word_char(substr(s, i, 1))
}

# Argument d une parenthese ouverte juste avant `start`, jusqu a la fermante
# assortie ; le reste de la ligne si elle ne se ferme pas sur cette ligne.
function balanced_arg(s, start,    depth, k, c) {
    depth = 1
    for (k = start; k <= length(s); k++) {
        c = substr(s, k, 1)
        if (c == "(") depth++
        else if (c == ")") {
            depth--
            if (depth == 0) return substr(s, start, k - start)
        }
    }
    return substr(s, start)
}

function trim(s) {
    sub(/^[ \t]+/, "", s)
    sub(/[ \t]+$/, "", s)
    return s
}

function count_substr(s, needle,    n, p) {
    n = 0
    while ((p = index(s, needle)) > 0) {
        n++
        s = substr(s, p + length(needle))
    }
    return n
}

function emit(category, population, extract, carries_float_word,    key) {
    key = FILENAME SUBSEP func_name SUBSEP category
    rank[key]++
    printf "%s | %s | %s | %d | ligne %d | %s | %s\n", FILENAME, func_name, category, rank[key], FNR, population, extract
    per_category[category]++
    total++
    if (carries_float_word) line_float_tokens++
    if (category == "cast") casts_to_audit++
}

FNR == 1 {
    func_name = "(hors fonction)"
    is_fixed_module = (FILENAME ~ /(^|\/)pico8_fixed\.c$/)
}

{
    line = $0
    close_after = 0
    if (line ~ /^[A-Za-z_]/ && index(line, "(") > 0) {
        head = substr(line, 1, index(line, "(") - 1)
        if (match(head, /[A-Za-z_][A-Za-z0-9_]*$/) && head !~ /=/) {
            func_name = substr(head, RSTART, RLENGTH)
            if (line ~ /;[ \t]*$/) close_after = 1
        }
    }
    if (line ~ /^}/) close_after = 1

    line_float_tokens = 0
    n = length(line)
    for (i = 1; i <= n; i++) {
        if (substr(line, i, 5) == "(int)") {
            j = i + 5
            while (substr(line, j, 1) == " ") j++
            if (substr(line, j, 22) == "pico8_fixed_to_double(") {
                emit("cast", "a auditer", trim(balanced_arg(line, j + 22)), 1)
                i = j + 21
                continue
            }
        }
        if (substr(line, i, 4) == "%.4f") {
            emit("reecrit", "-", "%.4f", 0)
            i += 3
            continue
        }
        if (!boundary_before(line, i)) continue
        if (substr(line, i, 24) == "pico8_fixed_from_double(") {
            arg = trim(balanced_arg(line, i + 24))
            if (arg ~ /^[-+]?([0-9]+\.?[0-9]*|\.[0-9]+)([eE][-+]?[0-9]+)?$/) {
                emit("constante", "-", arg, 1)
            } else if (is_fixed_module) {
                emit("autre", "-", "pico8_fixed_from_double(" arg ")", 1)
            } else {
                emit("reecrit", "-", "pico8_fixed_from_double(" arg ")", 1)
            }
            i += 23
        } else if (substr(line, i, 23) == "pico8_fixed_from_double" && boundary_after(line, i + 23)) {
            emit("autre", "-", "pico8_fixed_from_double", 1)
            i += 22
        } else if (substr(line, i, 21) == "pico8_fixed_to_double" && boundary_after(line, i + 21)) {
            emit("autre", "-", "pico8_fixed_to_double", 1)
            i += 20
        } else if (substr(line, i, 4) == "pow(" || substr(line, i, 4) == "sin(" || substr(line, i, 4) == "cos(") {
            emit("reecrit", "-", substr(line, i, 3), 0)
            i += 2
        } else if (substr(line, i, 6) == "double" && boundary_after(line, i + 6)) {
            emit("autre", "-", "double", 1)
            i += 5
        } else if (substr(line, i, 5) == "float" && boundary_after(line, i + 5)) {
            emit("autre", "-", "float", 1)
            i += 4
        }
    }

    words = count_substr(line, "double") + count_substr(line, "float")
    if (words != line_float_tokens) {
        printf "float_inventory: %s:%d : %d occurrence(s) de double/float, %d classee(s) -- jeton hors categorie : %s\n", FILENAME, FNR, words, line_float_tokens, line > "/dev/stderr"
        failed = 2
        exit failed
    }

    if (close_after) func_name = "(hors fonction)"
}

END {
    if (failed) exit failed
    sum = per_category["constante"] + per_category["cast"] + per_category["reecrit"] + per_category["autre"]
    if (sum != total) {
        printf "float_inventory: %d jetons, mais %d dans les categories\n", total, sum > "/dev/stderr"
        exit 3
    }
    print "#"
    print "# Comptes"
    printf "compte constante %d\n", per_category["constante"]
    printf "compte cast %d\n", per_category["cast"]
    printf "compte reecrit %d\n", per_category["reecrit"]
    printf "compte autre %d\n", per_category["autre"]
    printf "compte total %d\n", total
    printf "compte population (a) 0\n"
    printf "compte population (b) 0\n"
    printf "compte population (c) 0\n"
    printf "compte population a auditer %d\n", casts_to_audit
}
' "$@"
