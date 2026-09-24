#!/bin/sh
# usage: float_audit.sh <liste C1> [<events> <enregistrements>]... -- <source>...
# Un fichier d'enregistrements par fichier d'evenements rejoue.
# Ecrit le rapport sur la sortie standard. Codes de sortie, apres le rapport
# entier sauf 1, 2, 4 et 6 : 1 si les arguments sont mal formes ; 2 si les
# sources ne contiennent aucun appel de pico8_fixed_flr_int, une porte qui ne
# voit rien ne passe pas ; 3 si un site est en echec ; 4 si un enregistrement
# a deborde (OVERFLOW), des sites atteints sortiraient NON ATTEINT ; 5 si la
# liste C1 a une entree sans site ou une cle dupliquee, prioritaire sur 3 ;
# 6 si un fichier d'enregistrements manque (rejeu mort) ou porte une ligne mal
# formee, ancien format compris.

set -u

usage() {
    echo "float_audit: usage: float_audit.sh <liste C1> [<events> <enregistrements>]... -- <source>..." >&2
    exit 1
}

[ $# -ge 1 ] || usage
c1_list=$1
shift
pairs=
while [ $# -gt 0 ] && [ "$1" != -- ]; do
    [ $# -ge 2 ] && [ "$2" != -- ] || usage
    pairs="$pairs$1	$2
"
    shift 2
done
[ $# -gt 0 ] || usage
shift

cat <<EOF
# Audit avant/apres par site (docs/plan-2.md §7 Phase D0a), produit par
# tools/float_audit.sh via make float-audit. Aucun rapport versionne : la
# porte se lit ici et sur le code de sortie.
#
# Les sites sont les appels pico8_fixed_flr_int( des sources, hors de sa
# definition ; les comptes viennent du build -DPICO8_FLOAT_AUDIT, apres rejeu
# de chaque fichier de FRAMES_EVENTS. Classe C1 si la cle (fichier, fonction,
# extrait, occurrence) est dans tools/float_audit_c1.txt, D sinon.
# Une ligne par appel : fichier | fonction | ligne | classe | appels | negatifs
# | fractionnaires | ancien!=nouveau | nouveau!=flr | statut, puis, s'il y en
# a, la frame du premier ancien!=nouveau par fichier d'evenements (frame 0 =
# premiere capture et initialisation, -1 = hors frame). Un enregistrement dont
# la position n'est pas un appel pico8_fixed_flr_int( (paires D2, D3) est un
# site D sans lecture statique.
# Puis, par fichier d'evenements, la plus petite de ces frames et tous les
# sites qui l'atteignent : l'entree de la regle d'attribution.
#
# Echecs C1 :
# ECART           (int)pico8_fixed_to_double(x) != pico8_fixed_flr_int(x)
# NEGATIF         x < 0 : la preuve de signe du site est refutee
# NON ATTEINT     ligne jamais executee par le rejeu : rien n'y est verifie
# Echec D :
# NOUVEAU != FLR  la nouvelle valeur differe de la reference flr
# Pour un site D, negatifs, fractionnaires et ancien!=nouveau sont des
# mesures ; zero ancien!=nouveau, non atteint compris, sort « correction non
# vue par le filet », sans echec.
# Granularite : la ligne. Les appels d'une meme ligne, marques (ligne
# partagee), partagent leurs compteurs.
EOF

# awk -v refuse une valeur qui contient un saut de ligne (awk de macOS) :
# pairs passe par l'environnement.
FLOAT_AUDIT_PAIRS=$pairs awk -v c1_list="$c1_list" '
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

function list_error(message) {
    list_errors++
    list_error_text[list_errors] = "liste C1 : " message
}

# Extrait en milieu de ligne : il peut lui-meme contenir " | ".
function read_c1_list(    entry, n, f, k, extract, key, shown_key) {
    while ((getline entry < c1_list) > 0) {
        if (entry ~ /^[ \t]*(#|$)/) continue
        n = split(entry, f, / \| /)
        if (n < 4) {
            list_error("ligne mal formee : " entry)
            continue
        }
        extract = f[3]
        for (k = 4; k < n; k++) extract = extract " | " f[k]
        key = trim(f[1]) SUBSEP trim(f[2]) SUBSEP trim(extract) SUBSEP (trim(f[n]) + 0)
        shown_key = trim(f[1]) " | " trim(f[2]) " | " trim(extract) " | " (trim(f[n]) + 0)
        if (key in c1_key) {
            list_error("cle dupliquee : " shown_key)
            continue
        }
        c1_key[key] = shown_key
        c1_order[++c1_count] = key
    }
}

BEGIN {
    read_c1_list()
}

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
                extract = trim(balanced_arg(rest, p + 20))
                base = FILENAME SUBSEP func_name SUBSEP extract
                occurrence[base]++
                site[sites] = FILENAME SUBSEP FNR
                site_c1_key[sites] = base SUBSEP occurrence[base]
                shown[sites] = FILENAME " | " func_name " | ligne " FNR
            }
            rest = substr(rest, p + 20)
        }
    }

    if (close_after) func_name = "(hors fonction)"
}

function records_error(message) {
    print "float_audit: " message > "/dev/stderr"
    exit 6
}

function read_records(e, path,    entry, n, f, key, r, frame) {
    while ((r = (getline entry < path)) > 0) {
        if (entry ~ /^OVERFLOW/) {
            overflow = 1
            continue
        }
        if (entry == "") continue
        n = split(entry, f, "\t")
        if (n != 11 || f[1] != "AUD")
            records_error("enregistrement mal forme dans " path " : " entry)
        key = f[2] SUBSEP (f[4] + 0)
        if (!(key in record_func)) {
            record_func[key] = f[3]
            record_order[++records_seen] = key
        }
        passes[key] += f[5]
        negatives[key] += f[6]
        mismatches[key] += f[7]
        fractional[key] += f[9]
        not_flr[key] += f[10]
        if (!(key in first_bad) && f[8] != "-") first_bad[key] = f[8]
        if (f[11] != "-") {
            frame = f[11] + 0
            if (!((e, key) in first_frame) || frame < first_frame[e, key]) first_frame[e, key] = frame
        }
    }
    if (r < 0) records_error("enregistrements absents : " path)
    close(path)
}

END {
    events_count = split(ENVIRON["FLOAT_AUDIT_PAIRS"], pair_lines, "\n")
    for (e = 1; e <= events_count; e++) {
        if (pair_lines[e] == "") {
            events_count = e - 1
            break
        }
        split(pair_lines[e], pair, "\t")
        events_name[e] = pair[1]
        read_records(e, pair[2])
    }
    if (overflow) {
        print "float_audit: OVERFLOW dans les enregistrements, agrandir PICO8_FLOAT_AUDIT_MAX_SITES" > "/dev/stderr"
        exit 4
    }
    if (sites == 0) {
        print "float_audit: aucun site pico8_fixed_flr_int( dans les sources" > "/dev/stderr"
        exit 2
    }
    for (i = 1; i <= sites; i++) {
        on_line[site[i]]++
        if (site_c1_key[i] in c1_key) matched[site_c1_key[i]] = 1
    }
    for (r = 1; r <= records_seen; r++) {
        key = record_order[r]
        if (key in on_line) continue
        split(key, position, SUBSEP)
        site[++sites] = key
        on_line[key] = 1
        shown[sites] = position[1] " | " record_func[key] " | ligne " position[2]
    }
    for (i = 1; i <= c1_count; i++)
        if (!(c1_order[i] in matched)) list_error("entree sans site : " c1_key[c1_order[i]])

    for (i = 1; i <= sites; i++) {
        key = site[i]
        class = (site_c1_key[i] in c1_key) ? "C1" : "D"
        class_count[class]++
        status = ""
        if (!(key in passes)) {
            unreached++
            if (class == "C1") {
                status = "NON ATTEINT"
                failed++
            } else {
                status = "NON ATTEINT, correction non vue par le filet"
                unseen++
            }
        } else {
            reached++
            if (!(key in counted)) total += passes[key]
            counted[key] = 1
            if (class == "C1") {
                if (mismatches[key] > 0) {
                    status = "ECART"
                    ecart++
                }
                if (negatives[key] > 0) {
                    status = (status == "" ? "" : status " ") "NEGATIF"
                    negatif++
                }
                if (status != "") failed++
            } else {
                if (not_flr[key] > 0) {
                    status = "NOUVEAU != FLR"
                    new_not_flr++
                    failed++
                } else if (mismatches[key] == 0) {
                    status = "correction non vue par le filet"
                    unseen++
                }
            }
            if (status == "") status = "ok"
        }
        if (on_line[key] > 1) {
            status = status " (ligne partagee)"
            shared++
        }
        if (key in first_bad) status = status " | premier fautif " first_bad[key]
        frames = ""
        for (e = 1; e <= events_count; e++)
            if ((e, key) in first_frame)
                frames = frames (frames == "" ? "" : ", ") events_name[e] " frame " first_frame[e, key]
        if (frames != "") status = status " | premier ancien!=nouveau " frames
        printf "%s | %s | appels %d | negatifs %d | fractionnaires %d | ancien!=nouveau %d | nouveau!=flr %d | %s\n", shown[i], class, passes[key], negatives[key], fractional[key], mismatches[key], not_flr[key], status
    }
    for (i = 1; i <= list_errors; i++) print list_error_text[i]
    print "#"
    print "# Premier ancien!=nouveau par fichier d evenements"
    for (e = 1; e <= events_count; e++) {
        best = ""
        named = ""
        split("", named_key)
        for (i = 1; i <= sites; i++) {
            key = site[i]
            if (!((e, key) in first_frame) || (key in named_key)) continue
            if (best == "" || first_frame[e, key] < best + 0) {
                best = first_frame[e, key]
                named = ""
                split("", named_key)
            }
            if (first_frame[e, key] == best + 0) {
                named = named (named == "" ? "" : " ; ") shown[i]
                named_key[key] = 1
            }
        }
        if (best == "") printf "%s | premier ancien!=nouveau aucun\n", events_name[e]
        else printf "%s | premier ancien!=nouveau frame %d | %s\n", events_name[e], best, named
    }
    print "#"
    print "# Comptes"
    printf "audit sites %d\n", sites
    printf "audit sites C1 %d\n", class_count["C1"]
    printf "audit sites D %d\n", class_count["D"]
    printf "audit atteint %d\n", reached
    printf "audit non atteint %d\n", unreached
    printf "audit ecart %d\n", ecart
    printf "audit negatif %d\n", negatif
    printf "audit nouveau!=flr %d\n", new_not_flr
    printf "audit correction non vue %d\n", unseen
    printf "audit erreur liste C1 %d\n", list_errors
    printf "audit ligne partagee %d\n", shared
    printf "audit passages %d\n", total
    if (list_errors > 0) exit 5
    if (failed > 0) exit 3
}
' "$@"
