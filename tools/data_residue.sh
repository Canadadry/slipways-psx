#!/bin/sh
# usage: data_residue.sh [--detail | --check <cat>[,<cat>...]] [dossier...]
# Compte, dans les .c/.h des dossiers (defaut : src tests, chemins imprimes
# tels que recus), les lignes de chaque categorie du residu de la phase D,
# commentaires compris ; une ligne compte dans chaque categorie qu'elle touche.
#   sans option : "<cat>\t<lignes>\t<fichiers>" par categorie non nulle, puis
#                 "résidu D : N ligne(s), M fichier(s)" (couples fichier:ligne
#                 et fichiers distincts) ; code 0 quel que soit le compte.
#   --detail    : "<cat>\t<fichier>\t<lignes>" par couple non nul, puis le
#                 meme pied ; code 0.
#   --check     : "<cat>: <fichier>:<ligne>:<texte>" par occurrence des
#                 categories nommees, puis "résidu gardé (<cats>) : N ligne(s)" ;
#                 code 1 si N > 0.
# Vide (sortie de la phase D) : la seule ligne "résidu D : 0 ligne(s), 0 fichier(s)".
# Code 2 sur option ou categorie inconnue, dossier absent, aucun .c/.h, echec
# de grep : une mesure sur rien passerait pour vide.

set -u

CATEGORIES="valeur ob stash type-tag ref-audit par-nom dispatch overrides spec liste"

fail() {
    echo "data_residue: $1" >&2
    exit 2
}

pattern() {
    case $1 in
        valeur) printf '%s\n' '\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h' ;;
        ob) printf '%s\n' '\bpico8_ob(_[A-Za-z0-9_]+)?\b|\bPICO8_OB_[A-Z0-9_]+|\bpico8_fi(_fn)?\b|pico8_ob\.h' ;;
        stash) printf '%s\n' '\bpico8_unstash\b|\bpico8_stash_[A-Za-z0-9_]+|pico8_stash\.h|\bpico8_(r|iz)\(\)' ;;
        type-tag) printf '%s\n' '\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h' ;;
        ref-audit) printf '%s\n' 'pico8_ref_audit\.h|\bPICO8_REF_[A-Z0-9_]+' ;;
        par-nom) printf '%s\n' '\bpico8_entity_(get|set)_field\b|\b[a-z0-9_]*field_truthy\(|'\
'\bpico8_[a-z0-9_]+_(set_)?field(_fn)?\b|(->|\.)(set_)?field\b|\b[a-z0-9_]+_take_[a-z0-9_]+\(' ;;
        dispatch) printf '%s\n' '\bpico8_value_function(_t)?\b|\bpico8_mv\(' ;;
        overrides) printf '%s\n' '\boverrides\b' ;;
        spec) printf '%s\n' '\bg_[a-z0-9_]+_(spec|defaults)\b|\bpico8_[a-z0-9_]+_(default|defaults_reset)\b|'\
'\b[a-z0-9_]+_default_(fixed|table|vec|string|box|list|bool|hf)\b' ;;
        liste) printf '%s\n' '\bpico8_list_[a-z_]+\b|\bPICO8_REF_AS_LIST[A-Z_]*|'\
'\bPICO8_TYPE_TAG_LIST\b|pico8_list\.h' ;;
        *) return 1 ;;
    esac
}

# Ces sept _default sont des variantes a argument omis du cart, pas des defauts
# de classe : les compter rendrait spec impossible a eteindre. Pas de \b ici,
# le sed BSD ne le connait pas.
SPEC_EXCLUSION='s/(p8_spr|p8_fixed_rnd|hud_select_palette_block|hud_print_outlined|layout_row_draw_at|draw_functions_draw_rows|hud_draw_bar)_default//g'

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

# Rend dans $work/<cat> les lignes "<fichier>:<ligne>:<texte>", dans l'ordre
# des fichiers puis des lignes. grep rend 1 quand rien ne correspond.
collect() {
    status=0
    if [ "$1" = spec ]; then
        grep -lE "$(pattern spec)" $(cat "$work/files") > "$work/candidates" || status=$?
        [ "$status" -le 1 ] || fail "echec de grep"
        : > "$work/spec"
        while IFS= read -r file; do
            status=0
            sed -E "$SPEC_EXCLUSION" "$file" > "$work/excluded" || fail "echec de sed"
            grep -nE "$(pattern spec)" "$work/excluded" > "$work/numbers" || status=$?
            [ "$status" -le 1 ] || fail "echec de grep"
            cut -d: -f1 "$work/numbers" > "$work/numbers.cut"
            awk -v f="$file" 'NR == FNR { keep[$1] = 1; next } FNR in keep { print f ":" FNR ":" $0 }' \
                "$work/numbers.cut" "$file" >> "$work/spec"
        done < "$work/candidates"
    else
        grep -HnE "$(pattern "$1")" $(cat "$work/files") > "$work/$1" || status=$?
        [ "$status" -le 1 ] || fail "echec de grep"
    fi
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
    cut -d: -f1,2 "$work/$cat" >> "$work/all"
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
echo "résidu D : $lines ligne(s), $files fichier(s)"
