#!/bin/sh
# usage: float_inventory_check.sh <inventaire versionne> <sortie> <annotation> <lua.lua> <source.c>...
# Regenere l'inventaire dans <sortie> et le compare octet par octet a la copie
# versionnee, qui n'est jamais reecrite. Rend 0 si elles sont identiques, 1
# sinon, avec un diff court et la commande a relancer.

set -u

versioned=$1
fresh=$2
shift 2

stale() {
    echo "float_inventory_check: $versioned est perime -- $1" >&2
    echo "float_inventory_check: relancer make float-inventory" >&2
    exit 1
}

sh "$(dirname "$0")/float_inventory.sh" "$@" > "$fresh" || stale "l'inventaire des sources actuelles s'arrete en erreur"
[ -f "$versioned" ] || stale "fichier absent"

if ! cmp -s "$versioned" "$fresh"; then
    diff "$versioned" "$fresh" | head -n 20 >&2
    stale "il differe de l'inventaire des sources actuelles ($fresh)"
fi
