#!/bin/sh
# usage: composition_residue.sh [--detail | --check <cat>[,<cat>...]] [dossier...]
# Compte, dans les .c/.h des dossiers (defaut : src tests, chemins imprimes
# tels que recus), les lignes de chaque categorie du residu de la phase 2,
# commentaires compris ; une ligne compte dans chaque categorie qu'elle touche.
#   sans option : "<cat>\t<lignes>\t<fichiers>" par categorie non nulle, puis
#                 "résidu 2 : N ligne(s), M fichier(s)" (couples fichier:ligne
#                 et fichiers distincts des categories de SORTIE) ; code 0
#                 quel que soit le compte.
#   --detail    : "<cat>\t<fichier>\t<lignes>" par couple non nul, puis le
#                 meme pied ; code 0.
#   --check     : "<cat>: <fichier>:<ligne>:<texte>" par occurrence des
#                 categories nommees, puis "résidu gardé (<cats>) : N ligne(s)" ;
#                 code 1 si N > 0. Toute categorie connue est acceptee.
# Vide (sortie de la phase 2) : aucune ligne de categorie de SORTIE, et le pied
# "résidu 2 : 0 ligne(s), 0 fichier(s)".
# Code 2 sur option ou categorie inconnue, dossier absent, aucun .c/.h, echec
# de grep ou de sed : une mesure sur rien passerait pour vide.

set -u

CATEGORIES="lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux noyau-inclut"

# Categories comptees dans le pied. Une categorie nommee ici et absente de
# CATEGORIES ne compte rien : le script ne la mesure pas encore.
SORTIE="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité noyau-inclut atteinte-à-prouver"

fail() {
    echo "composition_residue: $1" >&2
    exit 2
}

pattern() {
    case $1 in
        lien-parent) printf '%s\n' '\bpico8_[a-z0-9_]+_of\b|\b[a-z0-9_]*(emitter|probe)_of\b|(->|\.|\(\*)parent\b' ;;
        accesseurs-q6) printf '%s\n' '(\b|_)(get_(nu|fa|ra|p|kq|ql|fg|mt|fx|p_)|set_m_)\b' ;;
        défauts) printf '%s\n' '\bpico8_[a-z0-9_]+_start(_t)?\b' ;;
        nu) printf '%s\n' '\bnu\b|get_nu\b|_nu_(tags|items)\b' ;;
        aiguillage-nom) printf '%s\n' '\bhp_(pose|ho)\b|\bpico8_ho_gp_has\b|\bpico8_hp_(mr|gp)_names\b|(->|\.)(mr|gp)\b' ;;
        un-ou-faux) printf '%s\n' '\b(pk|fp)_is_bool\b' ;;
        noyau-inclut) printf '%s\n' '^#include "pico8_(game_data|map_tables|data_init)\.h"' ;;
        *) return 1 ;;
    esac
}

# pico8_state_of rend un enum depuis un nom d'etat : ce n'est pas une lignee.
LIEN_EXCLUSION='s/pico8_state_of//g'
# Le "nu" echappe de la fixture de test_data_residue.c est une chaine de
# test, pas le champ. Pas de \b dans ces sed, le sed BSD ne le connait pas.
NU_EXCLUSION='s/\\"nu\\"//g'

exclusion() {
    case $1 in
        lien-parent) printf '%s\n' "$LIEN_EXCLUSION" ;;
        nu) printf '%s\n' "$NU_EXCLUSION" ;;
    esac
}

# En-tetes du noyau, par nom de fichier : noyau-inclut ne lit qu'eux.
NOYAU="pico8_struct.h pico8_ke.h pico8_entity.h"

mode=count
checked=
case ${1:-} in
    --detail) mode=detail; shift ;;
    --check)
        mode=check
        [ $# -ge 2 ] || fail "--check sans categorie"
        checked=$2
        shift 2
        [ -n "$checked" ] || fail "--check sans categorie"
        for cat in $(printf '%s\n' "$checked" | tr ',' ' '); do
            pattern "$cat" > /dev/null || fail "categorie inconnue : $cat"
        done
        case ,$checked, in
            *,,*) fail "categorie vide dans : $checked" ;;
        esac
        ;;
    -*) fail "option inconnue : $1" ;;
esac
[ $# -gt 0 ] || set -- src tests

LC_ALL=C
export LC_ALL

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

for dir in "$@"; do
    [ -d "$dir" ] || fail "dossier absent : $dir"
done
find "$@" \( -name '*.c' -o -name '*.h' \) -type f | sort > "$work/files"
[ -s "$work/files" ] || fail "aucun .c/.h dans : $*"

: > "$work/noyau"
while IFS= read -r file; do
    for name in $NOYAU; do
        [ "${file##*/}" = "$name" ] && printf '%s\n' "$file" >> "$work/noyau"
    done
done < "$work/files"

# Rend dans $work/<cat> les lignes "<fichier>:<ligne>:<texte>", dans l'ordre
# des fichiers puis des lignes. grep rend 1 quand rien ne correspond.
# L'exclusion est un sed sur le texte, jamais un filtre sur la ligne entiere :
# un filtre cacherait une vraie occurrence voisine.
collect() {
    list="$work/files"
    [ "$1" = noyau-inclut ] && list="$work/noyau"
    : > "$work/$1"
    [ -s "$list" ] || return 0
    status=0
    excl=$(exclusion "$1")
    if [ -z "$excl" ]; then
        grep -HnE "$(pattern "$1")" $(cat "$list") > "$work/$1" || status=$?
        [ "$status" -le 1 ] || fail "echec de grep"
        return 0
    fi
    grep -lE "$(pattern "$1")" $(cat "$list") > "$work/candidates" || status=$?
    [ "$status" -le 1 ] || fail "echec de grep"
    while IFS= read -r file; do
        status=0
        sed -E "$excl" "$file" > "$work/excluded" || fail "echec de sed"
        grep -nE "$(pattern "$1")" "$work/excluded" > "$work/numbers" || status=$?
        [ "$status" -le 1 ] || fail "echec de grep"
        cut -d: -f1 "$work/numbers" > "$work/numbers.cut"
        awk -v f="$file" 'NR == FNR { keep[$1] = 1; next } FNR in keep { print f ":" FNR ":" $0 }' \
            "$work/numbers.cut" "$file" >> "$work/$1"
    done < "$work/candidates"
}

if [ "$mode" = check ]; then
    : > "$work/all"
    for cat in $(printf '%s\n' "$checked" | tr ',' ' '); do
        collect "$cat"
        sed "s|^|$cat: |" "$work/$cat"
        cut -d: -f1,2 "$work/$cat" >> "$work/all"
    done
    n=$(sort -u "$work/all" | wc -l | tr -d ' ')
    echo "résidu gardé ($checked) : $n ligne(s)"
    [ "$n" -eq 0 ]
    exit
fi

: > "$work/all"
for cat in $CATEGORIES; do
    collect "$cat"
    [ -s "$work/$cat" ] || continue
    case " $SORTIE " in
        *" $cat "*) cut -d: -f1,2 "$work/$cat" >> "$work/all" ;;
    esac
    if [ "$mode" = detail ]; then
        cut -d: -f1 "$work/$cat" | uniq -c | awk -v c="$cat" '{ print c "\t" $2 "\t" $1 }'
    else
        lines=$(wc -l < "$work/$cat" | tr -d ' ')
        files=$(cut -d: -f1 "$work/$cat" | sort -u | wc -l | tr -d ' ')
        printf '%s\t%s\t%s\n' "$cat" "$lines" "$files"
    fi
done
lines=$(sort -u "$work/all" | wc -l | tr -d ' ')
files=$(cut -d: -f1 "$work/all" | sort -u | wc -l | tr -d ' ')
echo "résidu 2 : $lines ligne(s), $files fichier(s)"
