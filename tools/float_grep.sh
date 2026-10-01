#!/bin/sh
# usage: float_grep.sh <cc> <cflags> <exclusions> <repertoire> <source.c>...
# Cherche toute trace de flottant dans le coeur : les sources recues plus les
# en-tetes que <cc> <cflags> -MM leur trouve. Tout .c/.h de <repertoire> est
# soit dans ce coeur, soit dans <exclusions> (lignes "<chemin> | <justification>",
# # en tete de ligne pour un commentaire), jamais les deux, jamais aucun.
# Rend 1 si une ligne est trouvee, 2 sur partition incoherente, liste
# d'exclusion invalide, coeur vide ou echec du compilateur : une mesure sur
# rien passerait pour verte.
# <cflags> ne doit jamais porter PICO8_FLOAT_* : -MM ferait entrer dans le
# coeur l'en-tete d'audit, que ce build seul inclut.

set -u

cc=$1
cflags=$2
exclusions=$3
dir=$4
shift 4

fail() {
    echo "float_grep: $1" >&2
    exit 2
}

[ $# -gt 0 ] || fail "aucune source"
[ -f "$exclusions" ] || fail "liste d'exclusion introuvable : $exclusions"

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# cflags est volontairement non quote : c'est une liste d'options.
$cc $cflags -MM "$@" > "$work/deps" || fail "echec de $cc -MM"
# Apple clang ajoute aux dependances -MM des fichiers qui ne sont pas du C
# (SDKSettings.json du SDK) : seuls les .c et .h forment le coeur.
tr ' \\' '\n\n' < "$work/deps" | grep -v ':$' | grep -E '\.[ch]$' | LC_ALL=C sort -u > "$work/core"
[ -s "$work/core" ] || fail "coeur vide"

errors=0
: > "$work/excluded"
while IFS= read -r line; do
    case $line in
        '#'* | '') continue ;;
    esac
    case $line in
        *'|'*) ;;
        *) echo "float_grep: ligne sans justification : $line" >&2; errors=1; continue ;;
    esac
    path=$(printf '%s\n' "${line%%|*}" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//')
    why=$(printf '%s\n' "${line#*|}" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//')
    if [ -z "$why" ]; then
        echo "float_grep: justification vide : $path" >&2; errors=1
    fi
    if [ ! -f "$path" ]; then
        echo "float_grep: exclusion d'un fichier absent : $path" >&2; errors=1
    fi
    if grep -qxF "$path" "$work/excluded"; then
        echo "float_grep: exclusion en double : $path" >&2; errors=1
    fi
    echo "$path" >> "$work/excluded"
done < "$exclusions"
LC_ALL=C sort -u "$work/excluded" -o "$work/excluded"

find "$dir" \( -name '*.c' -o -name '*.h' \) -type f | LC_ALL=C sort > "$work/all"
LC_ALL=C comm -12 "$work/core" "$work/excluded" > "$work/both"
LC_ALL=C sort -u "$work/core" "$work/excluded" | LC_ALL=C comm -23 "$work/all" - > "$work/neither"
while IFS= read -r path; do
    echo "float_grep: dans le coeur et exclu : $path" >&2; errors=1
done < "$work/both"
while IFS= read -r path; do
    echo "float_grep: ni dans le coeur ni exclu : $path" >&2; errors=1
done < "$work/neither"
[ "$errors" -eq 0 ] || exit 2

# L'expression du §6.1 telle quelle, commentaires compris. grep rend 1 quand
# rien ne correspond : ce n'est pas une erreur ici.
status=0
grep -HnE '\b(double|float)\b' $(cat "$work/core") > "$work/found" || status=$?
[ "$status" -le 1 ] || fail "echec de grep"
LC_ALL=C sort -t: -k1,1 -k2,2n "$work/found"

count=$(wc -l < "$work/found" | tr -d ' ')
echo "float-grep coeur $(wc -l < "$work/core" | tr -d ' ') exclus $(wc -l < "$work/excluded" | tr -d ' ') lignes $count"
[ "$count" -eq 0 ]
