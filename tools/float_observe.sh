#!/bin/sh
# usage: float_observe.sh <inventaire> <compilateur> <enregistrements>...
# Ecrit le rapport sur la sortie standard. Rend 2 si l'inventaire ne contient
# aucun cast : une jointure vide passerait pour une observation complete ; 3
# si un cast annote (b) est vu negatif (CONTRADICTION), apres le rapport entier ;
# 4 si un enregistrement a deborde (OVERFLOW) : des casts atteints sortiraient
# NON COUVERT.

set -u

inventory=$1
compiler=$2
shift 2

cat <<EOF
# Observation des casts du coeur sous le filet (docs/plan-2.md §7, C0),
# produite par tools/float_observe.sh via make float-observe.
# Ne pas editer a la main.
#
# Compilateur : $compiler
#
# Les casts et leur population sont ceux de tools/float_inventory.sh ; le
# minimum brut 16.16 et les passages viennent du build -DPICO8_FLOAT_OBSERVE,
# apres rejeu de chaque fichier de FRAMES_EVENTS.
# Une ligne par cast : fichier | fonction | rang | ligne | population | etape | minimum | passages | observation | extrait
#
# L'observation est un indice, pas une preuve : "jamais negatif" dit seulement
# que le filet n'a rien montre. Seul controle mecanique : un cast (b) vu
# negatif est une CONTRADICTION.
#
# Limite : la granularite est la ligne, pas l'expression. Les casts d'une meme
# ligne, marques (ligne partagee), recoivent le minimum commun : prudent pour
# "jamais negatif", ambigu pour "vu negatif" (lequel des casts l'a recu).
#
# minimum   valeur brute 16.16 la plus petite recue, en hexa sur 32 bits
# NON COUVERT  ligne jamais atteinte par le rejeu : ni negatif, ni positif
EOF

cat "$@" /dev/null | awk -v inventory="$inventory" '
# Hexa sur 32 bits d un entier signe, sans dependre du %X de l awk.
function hex32(v,    digits, s, k, d) {
    if (v < 0) v += 4294967296
    digits = "0123456789ABCDEF"
    s = ""
    for (k = 0; k < 8; k++) {
        d = v % 16
        s = substr(digits, d + 1, 1) s
        v = (v - d) / 16
    }
    return "0x" s
}

$1 == "OVERFLOW" {
    overflow = 1
}

$1 == "OBS" {
    split($0, f, "\t")
    key = f[2] SUBSEP (f[4] + 0)
    if (!(key in passes) || f[5] + 0 < minimum[key]) minimum[key] = f[5] + 0
    passes[key] += f[6]
}

END {
    if (overflow) {
        print "float_observe: OVERFLOW dans les enregistrements, agrandir PICO8_FLOAT_OBSERVE_MAX_SITES" > "/dev/stderr"
        exit 4
    }
    casts = 0
    while ((getline entry < inventory) > 0) {
        if (entry ~ /^#/ || entry ~ /^compte / || entry == "") continue
        split(entry, field, " \\| ")
        if (field[3] != "cast") continue
        casts++
        line = field[5]
        sub(/^ligne /, "", line)
        site[casts] = field[1] SUBSEP (line + 0)
        on_line[site[casts]]++
        head = field[1] " | " field[2] " | " field[3] " | " field[4] " | " field[5] " | " field[6] " | " field[7]
        extract[casts] = substr(entry, length(head) + 4)
        population[casts] = field[6]
        shown[casts] = field[1] " | " field[2] " | " field[4] " | " field[5] " | " field[6] " | " field[7]
    }
    for (i = 1; i <= casts; i++) {
        key = site[i]
        if (key in passes) {
            measure = "min " hex32(minimum[key]) " | passages " passes[key]
            if (minimum[key] < 0) {
                status = "vu negatif"
                negative++
                if (population[i] == "(b)") {
                    contradictions++
                    refuted[contradictions] = shown[i]
                }
            } else {
                status = "jamais negatif"
                never++
            }
        } else {
            status = "NON COUVERT"
            measure = "min - | passages 0"
            uncovered++
        }
        if (on_line[key] > 1) {
            status = status " (ligne partagee)"
            shared++
        }
        printf "%s | %s | %s | %s\n", shown[i], measure, status, extract[i]
    }
    if (casts == 0) {
        printf "float_observe: aucun cast dans %s\n", inventory > "/dev/stderr"
        exit 2
    }
    print "#"
    print "# Comptes"
    printf "observation vu negatif %d\n", negative
    printf "observation jamais negatif %d\n", never
    printf "observation non couvert %d\n", uncovered
    printf "observation ligne partagee %d\n", shared
    printf "observation contradiction %d\n", contradictions
    printf "observation total %d\n", casts
    for (i = 1; i <= contradictions; i++)
        printf "CONTRADICTION %s : annote (b), vu negatif\n", refuted[i] > "/dev/stderr"
    if (contradictions > 0) exit 3
}
'
