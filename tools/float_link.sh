#!/bin/sh
# usage: float_link.sh <cc> <cflags> <binaire> <objet.o>...
# Lie les objets sans -lm (docs/plan-2.md §6.2) dans un binaire jetable.
# Rend 1 si le lien echoue, avec la liste des symboles non resolus ; le
# diagnostic complet reste dans <binaire>.err. Rend 2 sans objet : un lien de
# rien passerait pour vert.
# Le binaire n'est jamais execute : la mesure est un fait de lien, pas un rejeu.

set -u

cc=$1
cflags=$2
bin=$3
shift 3

if [ $# -eq 0 ]; then
    echo "float_link: aucun objet" >&2
    exit 2
fi

mkdir -p "$(dirname "$bin")"
rm -f "$bin"
status=0
# cflags est volontairement non quote : c'est une liste d'options.
$cc $cflags -o "$bin" "$@" 2> "$bin.err" || status=1

# GNU ld : undefined reference to `sym' ; lld : undefined symbol: sym
sed -n "s/.*undefined reference to [\`']\([^']*\)'.*/\1/p; s/.*undefined symbol: \([^[:space:]]*\).*/\1/p" "$bin.err" \
    | LC_ALL=C sort -u > "$bin.unresolved"
sed 's/^/non-resolu /' "$bin.unresolved"
count=$(wc -l < "$bin.unresolved" | tr -d ' ')
echo "float-link objets $# non-resolus $count"
if [ "$status" -ne 0 ] && [ "$count" -eq 0 ]; then
    echo "float_link: echec du lien, voir $bin.err" >&2
fi
exit "$status"
