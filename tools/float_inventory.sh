#!/bin/sh
# usage: float_inventory.sh <annotation> <lua.lua> <source.c>...
# Ecrit l'inventaire sur la sortie standard. Rend 2 si un jeton flottant
# d'une ligne n'est pris par aucune categorie, 3 si la somme des categories
# ne vaut pas le nombre de jetons, 4 si l'annotation des casts est incoherente
# (orpheline, manquante, (a) non source dans <lua.lua>, etape contraire a la
# population) : l'inventaire n'est alors pas ecrit en entier.

set -u

annotation=$1
lua=$2
shift 2

cat <<'EOF'
# Inventaire des sites flottants du coeur (docs/plan-2.md §3), produit par
# tools/float_inventory.sh via make float-inventory. Ne pas editer a la main.
#
# Une ligne par jeton : fichier | fonction | categorie | rang | ligne | population | etape | extrait
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
# Population et etape des casts : lues dans tools/float_annotation.txt, qui
# porte leur justification. "-" pour les autres categories.
EOF

awk -v annotation="$annotation" -v lua="$lua" '
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

function annotation_error(message) {
    printf "float_inventory: %s\n", message > "/dev/stderr"
    annotation_failed = 4
}

# Consomme le champ de tete de `rest` ; le dernier champ garde les
# separateurs qu il contient.
function next_field(    p, field) {
    p = index(rest, " | ")
    if (p == 0) {
        field = rest
        rest = ""
        fields_missing = 1
        return field
    }
    field = substr(rest, 1, p - 1)
    rest = substr(rest, p + 3)
    return field
}

function check_lua_source(where, justification,    s, cited, found) {
    s = justification
    found = 0
    while (match(s, /lua\.lua:[0-9]+/)) {
        cited = substr(s, RSTART + 8, RLENGTH - 8) + 0
        found = 1
        if (!(cited in lua_line) || index(lua_line[cited], "flr(") == 0)
            annotation_error(where " : (a), mais " lua ":" cited " ne contient pas flr(")
        s = substr(s, RSTART + RLENGTH)
    }
    if (!found) annotation_error(where " : (a) sans renvoi lua.lua:<ligne>")
}

function read_annotation(    entry, n, status, where, file, fn, extract, occurrence, population, step, justification, key) {
    n = 0
    while ((status = (getline entry < annotation)) > 0) {
        n++
        if (entry ~ /^#/ || entry ~ /^[ \t]*$/) continue
        where = annotation ":" n
        rest = entry
        fields_missing = 0
        file = next_field()
        fn = next_field()
        extract = next_field()
        occurrence = next_field()
        population = next_field()
        step = next_field()
        justification = rest
        if (fields_missing) {
            annotation_error(where " : 7 champs attendus : " entry)
            continue
        }
        if (occurrence !~ /^[1-9][0-9]*$/) annotation_error(where " : occurrence invalide : " occurrence)
        if (trim(justification) == "") annotation_error(where " : justification vide")
        if (population == "(a)") {
            if (step != "C1" && step != "D1") annotation_error(where " : (a) exige l etape C1 ou D1, pas " step)
            check_lua_source(where, justification)
        } else if (population == "(b)") {
            if (step != "C1") annotation_error(where " : (b) exige l etape C1, pas " step)
        } else if (population == "(c)") {
            if (step != "D1") annotation_error(where " : (c) exige l etape D1, pas " step)
        } else if (population == "a auditer") {
            if (step != "-") annotation_error(where " : a auditer exige l etape -, pas " step)
        } else {
            annotation_error(where " : population inconnue : " population)
        }
        key = file SUBSEP fn SUBSEP extract SUBSEP occurrence
        if (key in annotated_population) {
            annotation_error(where " : cle deja annotee a la ligne " annotated_at[key])
            continue
        }
        annotated_population[key] = population
        annotated_step[key] = step
        annotated_at[key] = n
        key_at_line[n] = key
    }
    if (status < 0) annotation_error(annotation " : illisible")
    annotation_lines = n
}

function read_lua(    entry, n) {
    n = 0
    while ((getline entry < lua) > 0) lua_line[++n] = entry
}

# Cle d annotation : l occurrence compte les casts de meme extrait dans la
# fonction, jamais le rang, qui glisserait des qu un frere est converti.
function annotate_cast(extract,    base, key) {
    base = FILENAME SUBSEP func_name SUBSEP extract
    occurrence[base]++
    key = base SUBSEP occurrence[base]
    if (key in annotated_population) {
        used[key] = 1
        cast_population = annotated_population[key]
        cast_step = annotated_step[key]
    } else {
        annotation_error(FILENAME ":" FNR " : cast non annote : " FILENAME " | " func_name " | " extract " | " occurrence[base])
        cast_population = "?"
        cast_step = "?"
    }
    per_population[cast_population]++
    per_step[cast_step]++
}

function emit(category, extract, carries_float_word,    key, population, step) {
    key = FILENAME SUBSEP func_name SUBSEP category
    rank[key]++
    population = "-"
    step = "-"
    if (category == "cast") {
        annotate_cast(extract)
        population = cast_population
        step = cast_step
    }
    printf "%s | %s | %s | %d | ligne %d | %s | %s | %s\n", FILENAME, func_name, category, rank[key], FNR, population, step, extract
    per_category[category]++
    total++
    if (carries_float_word) line_float_tokens++
}

BEGIN {
    read_lua()
    read_annotation()
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
                emit("cast", trim(balanced_arg(line, j + 22)), 1)
                i = j + 21
                continue
            }
        }
        if (substr(line, i, 4) == "%.4f") {
            emit("reecrit", "%.4f", 0)
            i += 3
            continue
        }
        if (!boundary_before(line, i)) continue
        if (substr(line, i, 24) == "pico8_fixed_from_double(") {
            arg = trim(balanced_arg(line, i + 24))
            if (arg ~ /^[-+]?([0-9]+\.?[0-9]*|\.[0-9]+)([eE][-+]?[0-9]+)?$/) {
                emit("constante", arg, 1)
            } else if (is_fixed_module) {
                emit("autre", "pico8_fixed_from_double(" arg ")", 1)
            } else {
                emit("reecrit", "pico8_fixed_from_double(" arg ")", 1)
            }
            i += 23
        } else if (substr(line, i, 23) == "pico8_fixed_from_double" && boundary_after(line, i + 23)) {
            emit("autre", "pico8_fixed_from_double", 1)
            i += 22
        } else if (substr(line, i, 21) == "pico8_fixed_to_double" && boundary_after(line, i + 21)) {
            emit("autre", "pico8_fixed_to_double", 1)
            i += 20
        } else if (substr(line, i, 4) == "pow(" || substr(line, i, 4) == "sin(" || substr(line, i, 4) == "cos(") {
            emit("reecrit", substr(line, i, 3), 0)
            i += 2
        } else if (substr(line, i, 6) == "double" && boundary_after(line, i + 6)) {
            emit("autre", "double", 1)
            i += 5
        } else if (substr(line, i, 5) == "float" && boundary_after(line, i + 5)) {
            emit("autre", "float", 1)
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
    for (n = 1; n <= annotation_lines; n++) {
        if ((n in key_at_line) && !(key_at_line[n] in used)) {
            split(key_at_line[n], part, SUBSEP)
            annotation_error(annotation ":" n " : annotation orpheline, aucun cast " part[1] " | " part[2] " | " part[3] " | " part[4])
        }
    }
    if (annotation_failed) exit annotation_failed
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
    printf "compte population (a) %d\n", per_population["(a)"]
    printf "compte population (b) %d\n", per_population["(b)"]
    printf "compte population (c) %d\n", per_population["(c)"]
    printf "compte population a auditer %d\n", per_population["a auditer"]
    printf "compte etape C1 %d\n", per_step["C1"]
    printf "compte etape D1 %d\n", per_step["D1"]
    printf "compte etape a auditer %d\n", per_step["-"]
}
' "$@"
