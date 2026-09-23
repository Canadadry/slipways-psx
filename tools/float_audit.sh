#!/bin/sh
# usage: float_audit.sh <enregistrements> <source>...
# Ecrit le rapport sur la sortie standard. Rend 2 si les sources ne
# contiennent aucun appel de pico8_fixed_flr_int : une porte qui ne voit rien
# ne passe pas ; 3 si un site est en ECART, NEGATIF ou NON ATTEINT, apres le
# rapport entier ; 4 si un enregistrement a deborde (OVERFLOW) : des sites
# atteints sortiraient NON ATTEINT.

set -u

records=$1
shift

cat <<EOF
# Porte d'audit de C1 (docs/plan-2.md §7), produite par tools/float_audit.sh
# via make float-audit. Aucun rapport versionne : la porte se lit ici et sur
# le code de sortie.
#
# Les sites sont les appels pico8_fixed_flr_int( des sources, hors de sa
# definition ; les comptes viennent du build -DPICO8_FLOAT_AUDIT, apres rejeu
# de chaque fichier de FRAMES_EVENTS.
# Une ligne par appel : fichier | fonction | ligne | passages | negatifs | ecarts | statut
#
# ECART        (int)pico8_fixed_to_double(x) != pico8_fixed_flr_int(x)
# NEGATIF      x < 0 : la preuve de signe du site est refutee
# NON ATTEINT  ligne jamais executee par le rejeu : rien n'y est verifie
# Granularite : la ligne. Les appels d'une meme ligne, marques (ligne
# partagee), partagent leurs compteurs.
EOF

awk -v records="$records" '
FNR == 1 {
    func_name = "(hors fonction)"
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

    if (FILENAME !~ /(^|\/)pico8_fixed\.c$/) {
        rest = line
        while ((p = index(rest, "pico8_fixed_flr_int(")) > 0) {
            if (p == 1 || substr(rest, p - 1, 1) !~ /[A-Za-z0-9_]/) {
                sites++
                site[sites] = FILENAME SUBSEP FNR
                shown[sites] = FILENAME " | " func_name " | ligne " FNR
            }
            rest = substr(rest, p + 20)
        }
    }

    if (close_after) func_name = "(hors fonction)"
}

END {
    while ((getline entry < records) > 0) {
        if (entry ~ /^OVERFLOW/) overflow = 1
        if (split(entry, f, "\t") < 8 || f[1] != "AUD") continue
        key = f[2] SUBSEP (f[4] + 0)
        passes[key] += f[5]
        negatives[key] += f[6]
        mismatches[key] += f[7]
        if (!(key in first_bad) && f[8] != "-") first_bad[key] = f[8]
    }
    if (overflow) {
        print "float_audit: OVERFLOW dans les enregistrements, agrandir PICO8_FLOAT_AUDIT_MAX_SITES" > "/dev/stderr"
        exit 4
    }
    if (sites == 0) {
        print "float_audit: aucun site pico8_fixed_flr_int( dans les sources" > "/dev/stderr"
        exit 2
    }
    for (i = 1; i <= sites; i++) on_line[site[i]]++
    for (i = 1; i <= sites; i++) {
        key = site[i]
        status = ""
        if (!(key in passes)) {
            status = "NON ATTEINT"
            unreached++
        } else {
            reached++
            if (!(key in counted)) total += passes[key]
            counted[key] = 1
            if (mismatches[key] > 0) {
                status = "ECART"
                ecart++
            }
            if (negatives[key] > 0) {
                status = (status == "" ? "" : status " ") "NEGATIF"
                negatif++
            }
            if (status == "") status = "ok"
        }
        if (on_line[key] > 1) {
            status = status " (ligne partagee)"
            shared++
        }
        if (key in first_bad) status = status " | premier fautif " first_bad[key]
        printf "%s | passages %d | negatifs %d | ecarts %d | %s\n", shown[i], passes[key], negatives[key], mismatches[key], status
    }
    print "#"
    print "# Comptes"
    printf "audit sites %d\n", sites
    printf "audit atteint %d\n", reached
    printf "audit non atteint %d\n", unreached
    printf "audit ecart %d\n", ecart
    printf "audit negatif %d\n", negatif
    printf "audit ligne partagee %d\n", shared
    printf "audit passages %d\n", total
    if (unreached + ecart + negatif > 0) exit 3
}
' "$@"
